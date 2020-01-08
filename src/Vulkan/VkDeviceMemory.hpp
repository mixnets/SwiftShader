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

#ifndef VK_DEVICE_MEMORY_HPP_
#define VK_DEVICE_MEMORY_HPP_

#include "VkConfig.h"
#include "VkObject.hpp"

namespace vk {

class DeviceMemory : public Object<DeviceMemory, VkDeviceMemory>
{
public:
	DeviceMemory(const VkMemoryAllocateInfo *pCreateInfo, void *mem);

	static size_t ComputeRequiredAllocationSize(const VkMemoryAllocateInfo *pCreateInfo);

#if SWIFTSHADER_EXTERNAL_MEMORY_OPAQUE_FD
	VkResult exportFd(int *pFd) const;
#endif

	void destroy(const VkAllocationCallbacks *pAllocator);
	VkResult allocate();
	VkResult map(VkDeviceSize offset, VkDeviceSize size, void **ppData);
	VkDeviceSize getCommittedMemoryInBytes() const;
	void *getOffsetPointer(VkDeviceSize pOffset) const;
	uint32_t getMemoryTypeIndex() const { return memoryTypeIndex; }
	void lock() const;
	void unlock() const;

	// If this is external memory, return true iff its handle type matches the bitmask
	// provided by |supportedExternalHandleTypes|. Otherwise, always return true.
	bool checkExternalMemoryHandleType(
	    VkExternalMemoryHandleTypeFlags supportedExternalMemoryHandleType) const;

	// Internal implementation class for external memory. Platform-specific.
	class ExternalBase;

private:
	void *buffer = nullptr;
	VkDeviceSize size = 0;
	uint32_t memoryTypeIndex = 0;
	ExternalBase *external = nullptr;
};

static inline DeviceMemory *Cast(VkDeviceMemory object)
{
	return DeviceMemory::Cast(object);
}

// Base abstract interface for a device memory implementation.
class DeviceMemory::ExternalBase
{
public:
	virtual ~ExternalBase() = default;

	// Allocate the memory according to |size|. On success return VK_SUCCESS
	// and sets |*pBuffer|.
	virtual VkResult allocate(size_t size, void **pBuffer) = 0;

	// Deallocate previously allocated memory at |buffer|.
	virtual void deallocate(void *buffer, size_t size) = 0;

	// Return the handle type flag bit supported by this implementation.
	// A value of 0 corresponds to non-external memory.
	virtual VkExternalMemoryHandleTypeFlagBits getFlagBit() const = 0;

	virtual void lock() const = 0;
	virtual void unlock() const = 0;

#if SWIFTSHADER_EXTERNAL_MEMORY_OPAQUE_FD
	virtual VkResult exportFd(int *pFd) const
	{
		return VK_ERROR_INVALID_EXTERNAL_HANDLE;
	}
#endif

protected:
	ExternalBase() = default;
};

}  // namespace vk

#endif  // VK_DEVICE_MEMORY_HPP_
