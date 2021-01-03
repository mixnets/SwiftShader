// Copyright 2019 The SwiftShader Authors. All Rights Reserved.
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

#ifndef VULKAN_PLATFORM
#define VULKAN_PLATFORM

#include "System/Debug.hpp"

#include <cstddef>
#include <cstdint>
#include <type_traits>

template<typename T>
class VkNonDispatchableHandle
{
public:
	operator void *() const
	{
		static_assert(sizeof(VkNonDispatchableHandle) == sizeof(uint64_t), "Size is not 64 bits!");

		// VkNonDispatchableHandle must be POD to ensure it gets passed by value the same way as a uint64_t,
		// which is the upstream header's handle type when compiled for 32-bit architectures. On 64-bit architectures,
		// the upstream header's handle type is a pointer type.
		static_assert(std::is_trivial<VkNonDispatchableHandle<T>>::value, "VkNonDispatchableHandle<T> is not trivial!");
		static_assert(std::is_standard_layout<VkNonDispatchableHandle<T>>::value, "VkNonDispatchableHandle<T> is not standard layout!");

		return reinterpret_cast<void *>(static_cast<uintptr_t>(handle));
	}

	void operator=(uint64_t h)
	{
		handle = h;
	}

	uint64_t handle;
};

#define VK_DEFINE_NON_DISPATCHABLE_HANDLE(object)        \
	typedef struct object##_T *object##Ptr;              \
	typedef VkNonDispatchableHandle<object##Ptr> object; \
	template class VkNonDispatchableHandle<object##Ptr>;

#include <vulkan/vk_ext_provoking_vertex.h>
#include <vulkan/vk_google_filtering_precision.h>
#include <vulkan/vulkan_core.h>

namespace vk {
// Here we define constants that used to be in the Vulkan headers, but are not part of the spec.
// See: https://github.com/KhronosGroup/Vulkan-Docs/issues/1230

// When updating the Vulkan Headers we use, go through each constant below and make sure they are valid.
// Once that's done, update SwiftShaderVulkanHeaderVersion.
constexpr int SwiftShaderVulkanHeaderVersion = 160;
static_assert(SwiftShaderVulkanHeaderVersion == VK_HEADER_VERSION, "Please validate/update constants below upon upgrading Vulkan headers");

constexpr auto VK_PIPELINE_BIND_POINT_RANGE_SIZE = (VK_PIPELINE_BIND_POINT_COMPUTE - VK_PIPELINE_BIND_POINT_GRAPHICS + 1);
constexpr auto VK_IMAGE_VIEW_TYPE_END_RANGE = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;

// Utility function to log the usage of a Vulkan result code
inline VkResult result(VkResult value, const char *string, const char *file, int line)
{
	if(value == VK_SUCCESS)  // VK_SUCCESS = 0
	{
		sw::trace("%s:%d result: %s\n", file, line, string);
	}
	else
	{
		if(value > 0)  // Successful completion code with status information
		{
			// TODO(b/139528621): Use 'Info' severity level
			sw::trace("%s:%d result: %s\n", file, line, string);
		}
		else  // Run time error code
		{
			sw::log_trap("%s:%d result: %s\n", file, line, string);
		}
	}

	return value;
}

#define VULKAN_RESULT_CODE(x) vk::result(x, #x, __FILE__, __LINE__)

// These macros override the enum values defined in vulkan_core.h, to log their usage.
#define VK_SUCCESS VULKAN_RESULT_CODE(VK_SUCCESS)
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

}  // namespace vk

#endif  // VULKAN_PLATFORM
