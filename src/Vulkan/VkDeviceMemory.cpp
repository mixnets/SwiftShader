// Copyright 2018 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "VkDeviceMemory.hpp"

#include "VkConfig.h"

namespace vk
{

// Base abstract interface for a device memory implementation.
class DeviceMemory::Impl {
public:
	virtual ~Impl() = default;

    // Allocate the memory according to |size|. On success return VK_SUCCESS
    // and sets |*pBuffer|.
	virtual VkResult allocate(size_t size, void** pBuffer) = 0;

    // Deallocate previously allocated memory at |buffer|.
	virtual void deallocate(void* buffer, size_t size) = 0;

    // Return the handle type flag bit supported by this implementation.
    // A value of 0 corresponds to non-external memory.
	virtual VkExternalMemoryHandleTypeFlagBits getFlagBit() const = 0;

#if SWIFTSHADER_EXTERNAL_MEMORY_LINUX_MEMFD
	virtual VkResult exportFd(int* pFd) const
	{
		return VK_ERROR_INVALID_EXTERNAL_HANDLE;
	}
#endif

protected:
	Impl() = default;
};

// Small class describing a given DeviceMemory::Impl derived class.
// |typeFlagBit| corresponds to the external memory handle type.
// |instanceSize| is the size of each class instance in bytes.
// |instanceInit| is a function pointer used to initialize an instance inplace
// according to a |pAllocateInfo| parameter.
class ExternalMemoryTraits {
public:
	VkExternalMemoryHandleTypeFlagBits typeFlagBit;
	size_t instanceSize;
	void (*instanceInit)(void* impl, const VkMemoryAllocateInfo* pAllocateInfo);
};

// Template function that parses a |pAllocateInfo.pNext| chain to verify that
// it asks for the creation or import of a memory type managed by implementation
// class T. On success, return true and sets |pTraits| accordingly. Otherwise
// return false.
template <typename  T>
static bool parseCreateInfo(const VkMemoryAllocateInfo* pAllocateInfo,
							ExternalMemoryTraits* pTraits)
{
	if (T::supportsAllocateInfo(pAllocateInfo)) {
		pTraits->typeFlagBit = T::typeFlagBit;
		pTraits->instanceSize = sizeof(T);
		pTraits->instanceInit = [](void* impl,
					   const VkMemoryAllocateInfo* pAllocateInfo) {
			new (impl) T(pAllocateInfo);
		};
		return true;
	}
	return false;
}

// Implementation of DeviceMemory::Impl that uses host memory (non-external).
class DeviceMemoryHostImpl : public DeviceMemory::Impl {
public:

    // Does not support any external memory type at all.
	static const VkExternalMemoryHandleTypeFlagBits typeFlagBit = (VkExternalMemoryHandleTypeFlagBits)0;

    // Always return true as is used as a fallback in findTraits() below.
	static bool supportsAllocateInfo(const VkMemoryAllocateInfo* pAllocateInfo) {
		return true;
	}

	DeviceMemoryHostImpl(const VkMemoryAllocateInfo* pAllocateInfo) {}

	VkResult allocate(size_t size, void** pBuffer) override {
		void* buffer = vk::allocate(size, REQUIRED_MEMORY_ALIGNMENT, DEVICE_MEMORY);
		if (!buffer)
			return VK_ERROR_OUT_OF_DEVICE_MEMORY;

		*pBuffer = buffer;
		return VK_SUCCESS;
	}

	void deallocate(void* buffer, size_t size) override {
		vk::deallocate(buffer, DEVICE_MEMORY);
	}

	VkExternalMemoryHandleTypeFlagBits getFlagBit() const override {
		return typeFlagBit;
	}
};

}  // namespace vk

#if SWIFTSHADER_EXTERNAL_MEMORY_LINUX_MEMFD
#include "VkDeviceMemoryExternalLinux.hpp"
#endif

namespace vk
{

static void findTraits(const VkMemoryAllocateInfo* pAllocateInfo,
		       ExternalMemoryTraits*       pTraits)
{
#if SWIFTSHADER_EXTERNAL_MEMORY_LINUX_MEMFD
	if (parseCreateInfo<LinuxMemfdExternalMemory>(pAllocateInfo, pTraits))
		return;
#endif
	parseCreateInfo<DeviceMemoryHostImpl>(pAllocateInfo, pTraits);
}

DeviceMemory::DeviceMemory(const VkMemoryAllocateInfo* pAllocateInfo, void* mem) :
	size(pAllocateInfo->allocationSize), memoryTypeIndex(pAllocateInfo->memoryTypeIndex),
	impl(reinterpret_cast<Impl *>(mem))
{
	ASSERT(size);

	ExternalMemoryTraits traits;
	findTraits(pAllocateInfo, &traits);
	traits.instanceInit(impl, pAllocateInfo);
}

void DeviceMemory::destroy(const VkAllocationCallbacks* pAllocator)
{
	if (buffer)
	{
		impl->deallocate(buffer, size);
		buffer = nullptr;
	}
	impl->~Impl();  // Call virtual destructor in place.
	vk::deallocate(impl, pAllocator);
}

size_t DeviceMemory::ComputeRequiredAllocationSize(const VkMemoryAllocateInfo* pAllocateInfo)
{
	ExternalMemoryTraits traits;
	findTraits(pAllocateInfo, &traits);
	return traits.instanceSize;
}

VkResult DeviceMemory::allocate()
{
	VkResult result = VK_SUCCESS;
	if (!buffer)
	{
		result = impl->allocate(size, &buffer);
	}
	return result;
}

VkResult DeviceMemory::map(VkDeviceSize pOffset, VkDeviceSize pSize, void** ppData)
{
	*ppData = getOffsetPointer(pOffset);

	return VK_SUCCESS;
}

VkDeviceSize DeviceMemory::getCommittedMemoryInBytes() const
{
	return size;
}

void* DeviceMemory::getOffsetPointer(VkDeviceSize pOffset) const
{
	ASSERT(buffer);

	return reinterpret_cast<char*>(buffer) + pOffset;
}

bool DeviceMemory::checkExternalMemoryHandleType(
		VkExternalMemoryHandleTypeFlags supportedHandleTypes) const {
	if (!supportedHandleTypes) {
		// This image or buffer does not need to be stored on external
		// memory, so this check should always pass.
		return true;
	}
	VkExternalMemoryHandleTypeFlagBits handle_type_bit = impl->getFlagBit();
	if (!handle_type_bit)
	{
		// This device memory is not external and can accomodate
		// any image or buffer as well.
		return true;
	}
	// Return true only if the external memory type is compatible with the
	// one specified during VkCreate{Image,Buffer}(), through a
	// VkExternalMemory{Image,Buffer}AllocateInfo struct.
	return (supportedHandleTypes & handle_type_bit) != 0;
}

#if SWIFTSHADER_EXTERNAL_MEMORY_LINUX_MEMFD
VkResult DeviceMemory::exportFd(int* pFd) const
{
	return impl->exportFd(pFd);
}
#endif

} // namespace vk
