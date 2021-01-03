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

#ifndef VK_OBJECT_HPP_
#define VK_OBJECT_HPP_

#include "VkConfig.hpp"
#include "VkMemory.hpp"

#include "System/Debug.hpp"

#include <vulkan/vk_icd.h>
#undef None
#undef Bool

#include <new>

namespace vk {

template<typename T, typename VkT>
static inline T *VkTtoT(VkT vkObject)
{
	return static_cast<T *>(static_cast<void *>(vkObject));
}

template<typename T, typename VkT>
static inline VkT TtoVkT(T *object)
{
	return { static_cast<uint64_t>(reinterpret_cast<uintptr_t>(object)) };
}

// For use in the placement new to make it verbose that we're allocating an object using device memory
static constexpr VkAllocationCallbacks *DEVICE_MEMORY = nullptr;

template<typename T, typename VkT, typename CreateInfo, typename... ExtendedInfo>
static VkResult Create(const VkAllocationCallbacks *pAllocator, const CreateInfo *pCreateInfo, VkT *outObject, ExtendedInfo... extendedInfo)
{
	*outObject = VK_NULL_HANDLE;

	size_t size = T::ComputeRequiredAllocationSize(pCreateInfo);
	void *memory = nullptr;
	if(size)
	{
		memory = vk::allocate(size, REQUIRED_MEMORY_ALIGNMENT, pAllocator, T::GetAllocationScope());
		if(!memory)
		{
			return VK_ERROR_OUT_OF_HOST_MEMORY;
		}
	}

	void *objectMemory = vk::allocate(sizeof(T), alignof(T), pAllocator, T::GetAllocationScope());
	if(!objectMemory)
	{
		vk::deallocate(memory, pAllocator);
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	auto object = new(objectMemory) T(pCreateInfo, memory, extendedInfo...);

	if(!object)
	{
		vk::deallocate(memory, pAllocator);
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	*outObject = *object;

	// Assert that potential v-table offsets from multiple inheritance aren't causing an offset on the handle
	ASSERT(*outObject == objectMemory);

	return VK_SUCCESS;
}

template<typename T, typename VkT>
class ObjectBase
{
public:
	using VkType = VkT;

	void destroy(const VkAllocationCallbacks *pAllocator) {}  // Method defined by objects to delete their content, if necessary

	template<typename CreateInfo, typename... ExtendedInfo>
	static VkResult Create(const VkAllocationCallbacks *pAllocator, const CreateInfo *pCreateInfo, VkT *outObject, ExtendedInfo... extendedInfo)
	{
		return vk::Create<T, VkT, CreateInfo>(pAllocator, pCreateInfo, outObject, extendedInfo...);
	}

	static constexpr VkSystemAllocationScope GetAllocationScope() { return VK_SYSTEM_ALLOCATION_SCOPE_OBJECT; }
};

template<typename T, typename VkT>
class Object : public ObjectBase<T, VkT>
{
public:
	operator VkT()
	{
		// The static_cast<T*> is used to make sure the returned pointer points to the
		// beginning of the object, even if the derived class uses multiple inheritance
		return vk::TtoVkT<T, VkT>(static_cast<T *>(this));
	}

	static inline T *Cast(VkT vkObject)
	{
		return vk::VkTtoT<T, VkT>(vkObject);
	}
};

template<typename T, typename VkT>
class DispatchableObject
{
	VK_LOADER_DATA loaderData = { ICD_LOADER_MAGIC };

	T object;

public:
	static constexpr VkSystemAllocationScope GetAllocationScope() { return T::GetAllocationScope(); }

	template<typename... Args>
	DispatchableObject(Args... args)
	    : object(args...)
	{
	}

	~DispatchableObject() = delete;

	void destroy(const VkAllocationCallbacks *pAllocator)
	{
		object.destroy(pAllocator);
	}

	void operator delete(void *ptr, const VkAllocationCallbacks *pAllocator)
	{
		// Should never happen
		ASSERT(false);
	}

	template<typename CreateInfo, typename... ExtendedInfo>
	static VkResult Create(const VkAllocationCallbacks *pAllocator, const CreateInfo *pCreateInfo, VkT *outObject, ExtendedInfo... extendedInfo)
	{
		return vk::Create<DispatchableObject<T, VkT>, VkT, CreateInfo>(pAllocator, pCreateInfo, outObject, extendedInfo...);
	}

	template<typename CreateInfo>
	static size_t ComputeRequiredAllocationSize(const CreateInfo *pCreateInfo)
	{
		return T::ComputeRequiredAllocationSize(pCreateInfo);
	}

	static inline T *Cast(VkT vkObject)
	{
		return (vkObject == VK_NULL_HANDLE) ? nullptr : &(reinterpret_cast<DispatchableObject<T, VkT> *>(vkObject)->object);
	}

	operator VkT()
	{
		return reinterpret_cast<VkT>(this);
	}
};

}  // namespace vk

namespace vk {

inline VkResult result(VkResult value, const char *string, const char *file, int line)
{
	if(value > 0)  // Success code
	{
		sw::trace("%s:%s result: %s", file, line, string);
	}
	else
	{
		sw::log_trap("%s:%s result: %s", file, line, string);
	}

	return value;
}

}  // namespace vk

#define VULKAN_RESULT_CODE(x) vk::result(x, #x, __FILE__, __LINE__)

//#define VK_SUCCESS VULKAN_RESULT_CODE(VK_SUCCESS)
#define VK_NOT_READY VULKAN_RESULT_CODE(VK_NOT_READY)
#define VK_TIMEOUT VULKAN_RESULT_CODE(VK_TIMEOUT)
#define VK_EVENT_SET VULKAN_RESULT_CODE(VK_EVENT_SET)
#define VK_EVENT_RESET VULKAN_RESULT_CODE(VK_EVENT_RESET)
#define VK_INCOMPLETE VULKAN_RESULT_CODE(VK_INCOMPLETE)
#define VK_ERROR_OUT_OF_HOST_MEMORY VULKAN_RESULT_CODE(VK_ERROR_OUT_OF_HOST_MEMORY)
#define VK_ERROR_OUT_OF_DEVICE_MEMORY VULKAN_RESULT_CODE(VK_ERROR_OUT_OF_DEVICE_MEMORY)
#define VK_ERROR_INITIALIZATION_FAILED VULKAN_RESULT_CODE(VK_ERROR_INITIALIZATION_FAILED)
#define VK_ERROR_DEVICE_LOST VULKAN_RESULT_CODE(VK_ERROR_DEVICE_LOST)
#define VK_ERROR_MEMORY_MAP_FAILED VULKAN_RESULT_CODE(VK_ERROR_MEMORY_MAP_FAILED)
#define VK_ERROR_LAYER_NOT_PRESENT VULKAN_RESULT_CODE(VK_ERROR_LAYER_NOT_PRESENT)
#define VK_ERROR_EXTENSION_NOT_PRESENT VULKAN_RESULT_CODE(VK_ERROR_EXTENSION_NOT_PRESENT)
#define VK_ERROR_FEATURE_NOT_PRESENT VULKAN_RESULT_CODE(VK_ERROR_FEATURE_NOT_PRESENT)
#define VK_ERROR_INCOMPATIBLE_DRIVER VULKAN_RESULT_CODE(VK_ERROR_INCOMPATIBLE_DRIVER)
#define VK_ERROR_TOO_MANY_OBJECTS VULKAN_RESULT_CODE(VK_ERROR_TOO_MANY_OBJECTS)
#define VK_ERROR_FORMAT_NOT_SUPPORTED VULKAN_RESULT_CODE(VK_ERROR_FORMAT_NOT_SUPPORTED)
#define VK_ERROR_FRAGMENTED_POOL VULKAN_RESULT_CODE(VK_ERROR_FRAGMENTED_POOL)
#define VK_ERROR_UNKNOWN VULKAN_RESULT_CODE(VK_ERROR_UNKNOWN)
#define VK_ERROR_OUT_OF_POOL_MEMORY VULKAN_RESULT_CODE(VK_ERROR_OUT_OF_POOL_MEMORY)
#define VK_ERROR_INVALID_EXTERNAL_HANDLE VULKAN_RESULT_CODE(VK_ERROR_INVALID_EXTERNAL_HANDLE)
#define VK_ERROR_FRAGMENTATION VULKAN_RESULT_CODE(VK_ERROR_FRAGMENTATION)
#define VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS VULKAN_RESULT_CODE(VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS)
#define VK_ERROR_SURFACE_LOST_KHR VULKAN_RESULT_CODE(VK_ERROR_SURFACE_LOST_KHR)
#define VK_ERROR_NATIVE_WINDOW_IN_USE_KHR VULKAN_RESULT_CODE(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR)
#define VK_SUBOPTIMAL_KHR VULKAN_RESULT_CODE(VK_SUBOPTIMAL_KHR)
#define VK_ERROR_OUT_OF_DATE_KHR VULKAN_RESULT_CODE(VK_ERROR_OUT_OF_DATE_KHR)
#define VK_ERROR_INCOMPATIBLE_DISPLAY_KHR VULKAN_RESULT_CODE(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR)
#define VK_ERROR_VALIDATION_FAILED_EXT VULKAN_RESULT_CODE(VK_ERROR_VALIDATION_FAILED_EXT)
#define VK_ERROR_INVALID_SHADER_NV VULKAN_RESULT_CODE(VK_ERROR_INVALID_SHADER_NV)
#define VK_ERROR_INCOMPATIBLE_VERSION_KHR VULKAN_RESULT_CODE(VK_ERROR_INCOMPATIBLE_VERSION_KHR)
#define VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT VULKAN_RESULT_CODE(VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT)
#define VK_ERROR_NOT_PERMITTED_EXT VULKAN_RESULT_CODE(VK_ERROR_NOT_PERMITTED_EXT)
#define VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT VULKAN_RESULT_CODE(VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT)
#define VK_THREAD_IDLE_KHR VULKAN_RESULT_CODE(VK_THREAD_IDLE_KHR)
#define VK_THREAD_DONE_KHR VULKAN_RESULT_CODE(VK_THREAD_DONE_KHR)
#define VK_OPERATION_DEFERRED_KHR VULKAN_RESULT_CODE(VK_OPERATION_DEFERRED_KHR)
#define VK_OPERATION_NOT_DEFERRED_KHR VULKAN_RESULT_CODE(VK_OPERATION_NOT_DEFERRED_KHR)
#define VK_PIPELINE_COMPILE_REQUIRED_EXT VULKAN_RESULT_CODE(VK_PIPELINE_COMPILE_REQUIRED_EXT)
#define VK_ERROR_OUT_OF_POOL_MEMORY_KHR VULKAN_RESULT_CODE(VK_ERROR_OUT_OF_POOL_MEMORY_KHR)
#define VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR VULKAN_RESULT_CODE(VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR)
#define VK_ERROR_FRAGMENTATION_EXT VULKAN_RESULT_CODE(VK_ERROR_FRAGMENTATION_EXT)
#define VK_ERROR_INVALID_DEVICE_ADDRESS_EXT VULKAN_RESULT_CODE(VK_ERROR_INVALID_DEVICE_ADDRESS_EXT)
#define VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR VULKAN_RESULT_CODE(VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR)
#define VK_ERROR_PIPELINE_COMPILE_REQUIRED_EXT VULKAN_RESULT_CODE(VK_ERROR_PIPELINE_COMPILE_REQUIRED_EXT)

#endif  // VK_OBJECT_HPP_
