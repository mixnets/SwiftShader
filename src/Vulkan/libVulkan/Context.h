// Copyright 2017 The SwiftShader Authors. All Rights Reserved.
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

// Context.h: Defines the Context class, managing all vulkan state and performing
// rendering operations.

#ifndef VULKAN_CONTEXT_H_
#define VULKAN_CONTEXT_H_

#include <unordered_map>
#include "Vulkan/vulkan.h"

namespace vulkan
{
#define GLOBAL_EXT_SIZE 4
#define DEVICE_EXT_SIZE 36


	static const VkExtensionProperties global_ext[] = {
		{
			VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
			1,
		},
		{
			VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME,
			1,
		},
		{
			VK_KHR_SURFACE_EXTENSION_NAME,
			25,
		},
		{
			VK_KHX_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME,
			1,
		},
		{
			VK_KHX_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME,
			1,
		}
	};


	static const VkExtensionProperties device_extensions[] = {
		{
			VK_KHR_DESCRIPTOR_UPDATE_TEMPLATE_EXTENSION_NAME,
			1,
		},
		{
			VK_KHR_INCREMENTAL_PRESENT_EXTENSION_NAME,
			1,
		},
		{
			VK_KHR_MAINTENANCE1_EXTENSION_NAME,
			1,
		},
		{
			VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME,
			1,
		},
		{
			VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME,
			1,
		},
		{
			VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME,
			1,
		},
		{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			68,
		},
		{
			VK_KHX_EXTERNAL_MEMORY_EXTENSION_NAME,
			1,
		},
		{
			VK_KHX_EXTERNAL_MEMORY_FD_EXTENSION_NAME,
			1,
		},
		{
			VK_KHX_EXTERNAL_SEMAPHORE_EXTENSION_NAME,
			1,
		},
		{
			VK_KHX_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME,
			1,
		},
		{
			VK_KHX_MULTIVIEW_EXTENSION_NAME,
			1,
		},
	};

	static void *
		default_alloc_func(void *pUserData, size_t size, size_t align,
			VkSystemAllocationScope allocationScope)
	{
		return malloc(size);
	}

	static void *
		default_realloc_func(void *pUserData, void *pOriginal, size_t size,
			size_t align, VkSystemAllocationScope allocationScope)
	{
		return realloc(pOriginal, size);
	}

	static void
		default_free_func(void *pUserData, void *pMemory)
	{
		free(pMemory);
	}

	static const VkAllocationCallbacks default_alloc = {
		NULL,
		default_alloc_func,
		default_realloc_func,
		default_free_func,
	};

	static const VkQueueFamilyProperties
		queue_family_properties = {
		VK_QUEUE_GRAPHICS_BIT |
		VK_QUEUE_COMPUTE_BIT |
		VK_QUEUE_TRANSFER_BIT,
		 1,
		 36, 
		{ 1, 1, 1 },
	};

	static const std::unordered_map < std::string, void *> func_ptrs = {
		{ "vkCreateInstance", vkCreateInstance },
		{ "vkDestroyInstance", vkDestroyInstance },
		{ "vkEnumeratePhysicalDevices", vkEnumeratePhysicalDevices },
		{ "vkGetPhysicalDeviceFeatures", vkGetPhysicalDeviceFeatures },
		{ "vkGetPhysicalDeviceFormatProperties", vkGetPhysicalDeviceFormatProperties },
		{ "vkGetPhysicalDeviceImageFormatProperties", vkGetPhysicalDeviceImageFormatProperties },
		{ "vkGetPhysicalDeviceProperties", vkGetPhysicalDeviceProperties },
		{ "vkGetPhysicalDeviceQueueFamilyProperties", vkGetPhysicalDeviceQueueFamilyProperties },
		{ "vkGetPhysicalDeviceMemoryProperties", vkGetPhysicalDeviceMemoryProperties },
		{ "vkGetInstanceProcAddr", vkGetInstanceProcAddr },
		{ "vkGetDeviceProcAddr", vkGetDeviceProcAddr },
		{ "vkCreateDevice", vkCreateDevice },
		{ "vkDestroyDevice", vkDestroyDevice },
		{ "vkEnumerateInstanceExtensionProperties", vkEnumerateInstanceExtensionProperties },
		{ "vkEnumerateDeviceExtensionProperties", vkEnumerateDeviceExtensionProperties },
		{ "vkEnumerateInstanceLayerProperties", vkEnumerateInstanceLayerProperties },
		{ "vkEnumerateDeviceLayerProperties", vkEnumerateDeviceLayerProperties },
		{ "vkGetDeviceQueue", vkGetDeviceQueue },
		{ "vkQueueSubmit", vkQueueSubmit },
		{ "vkQueueWaitIdle", vkQueueWaitIdle },
		{ "vkDeviceWaitIdle", vkDeviceWaitIdle },
		{ "vkAllocateMemory", vkAllocateMemory },
		{ "vkFreeMemory", vkFreeMemory },
		{ "vkMapMemory", vkMapMemory },
		{ "vkUnmapMemory", vkUnmapMemory },
		{ "vkFlushMappedMemoryRanges", vkFlushMappedMemoryRanges },
		{ "vkInvalidateMappedMemoryRanges", vkInvalidateMappedMemoryRanges },
		{ "vkGetDeviceMemoryCommitment", vkGetDeviceMemoryCommitment },
		{ "vkBindBufferMemory", vkBindBufferMemory },
		{ "vkBindImageMemory", vkBindImageMemory },
		{ "vkGetBufferMemoryRequirements", vkGetBufferMemoryRequirements },
		{ "vkGetImageMemoryRequirements", vkGetImageMemoryRequirements },
		{ "vkGetImageSparseMemoryRequirements", vkGetImageSparseMemoryRequirements },
		{ "vkGetPhysicalDeviceSparseImageFormatProperties", vkGetPhysicalDeviceSparseImageFormatProperties },
		{ "vkQueueBindSparse", vkQueueBindSparse },
		{ "vkCreateFence", vkCreateFence },
		{ "vkDestroyFence", vkDestroyFence },
		{ "vkResetFences", vkResetFences },
		{ "vkGetFenceStatus", vkGetFenceStatus },
		{ "vkWaitForFences", vkWaitForFences },
		{ "vkCreateSemaphore", vkCreateSemaphore },
		{ "vkDestroySemaphore", vkDestroySemaphore },
		{ "vkCreateEvent", vkCreateEvent },
		{ "vkDestroyEvent", vkDestroyEvent },
		{ "vkGetEventStatus", vkGetEventStatus },
		{ "vkSetEvent", vkSetEvent },
		{ "vkResetEvent", vkResetEvent },
		{ "vkCreateQueryPool",vkCreateQueryPool },
		{ "vkDestroyQueryPool",vkDestroyQueryPool },
		{ "vkGetQueryPoolResults", vkGetQueryPoolResults },
		{ "vkCreateBuffer", vkCreateBuffer },
		{ "vkDestroyBuffer", vkDestroyBuffer },
		{ "vkCreateBufferView", vkCreateBufferView },
		{ "vkDestroyBufferView", vkDestroyBufferView },
		{ "vkDestroyBufferView", vkDestroyBufferView },
		{ "vkDestroyImage",vkDestroyImage },
		{ "vkGetImageSubresourceLayout", vkGetImageSubresourceLayout },
		{ "vkCreateImageView", vkCreateImageView },
		{ "vkDestroyImageView", vkDestroyImageView },
		{ "vkCreateShaderModule", vkCreateShaderModule },
		{ "vkDestroyShaderModule", vkDestroyShaderModule },
		{ "vkCreatePipelineCache",vkCreatePipelineCache },
		{ "vkDestroyPipelineCache", vkDestroyPipelineCache },
		{ "vkGetPipelineCacheData", vkGetPipelineCacheData },
		{ "vkMergePipelineCaches", vkMergePipelineCaches },
		{ "vkCreateGraphicsPipelines", vkCreateGraphicsPipelines },
		{ "vkCreateComputePipelines", vkCreateComputePipelines },
		{ "vkDestroyPipeline",vkDestroyPipeline },
		{ "vkCreatePipelineLayout", vkCreatePipelineLayout },
		{ "vkDestroyPipelineLayout", vkDestroyPipelineLayout },
		{ "vkCreateSampler", vkCreateSampler },
		{ "vkDestroySampler",vkDestroySampler },
		{ "vkCreateDescriptorSetLayout", vkCreateDescriptorSetLayout },
		{ "vkDestroyDescriptorSetLayout", vkDestroyDescriptorSetLayout },
		{ "vkCreateDescriptorPool", vkCreateDescriptorPool },
		{ "vkDestroyDescriptorPool",vkDestroyDescriptorPool },
		{ "vkResetDescriptorPool", vkResetDescriptorPool },
		{ "vkAllocateDescriptorSets", vkAllocateDescriptorSets },
		{ "vkFreeDescriptorSets", vkFreeDescriptorSets },
		{ "vkUpdateDescriptorSets", vkUpdateDescriptorSets },
		{ "vkCreateFramebuffer", vkCreateFramebuffer },
		{ "vkDestroyFramebuffer", vkDestroyFramebuffer },
		{ "vkCreateRenderPass", vkCreateRenderPass },
		{ "vkDestroyRenderPass", vkDestroyRenderPass },
		{ "vkGetRenderAreaGranularity", vkGetRenderAreaGranularity },
		{ "vkCreateCommandPool", vkCreateCommandPool },
		{ "vkDestroyCommandPool", vkDestroyCommandPool },
		{ "vkResetCommandPool", vkResetCommandPool },
		{ "vkAllocateCommandBuffers", vkAllocateCommandBuffers },
		{ "vkFreeCommandBuffers", vkFreeCommandBuffers },
		{ "vkBeginCommandBuffer", vkBeginCommandBuffer },
		{ "vkEndCommandBuffer",vkEndCommandBuffer },
		{ "vkResetCommandBuffer", vkResetCommandBuffer },
		{ "vkCmdBindPipeline", vkCmdBindPipeline },
		{ "vkCmdSetViewport", vkCmdSetViewport },
		{ "vkCmdSetScissor", vkCmdSetScissor },
		{ "vkCmdSetLineWidth", vkCmdSetLineWidth },
		{ "vkCmdSetDepthBias", vkCmdSetDepthBias },
		{ "vkCmdSetBlendConstants", vkCmdSetBlendConstants },
		{ "vkCmdSetDepthBounds",vkCmdSetDepthBounds },
		{ "vkCmdSetStencilCompareMask", vkCmdSetStencilCompareMask },
		{ "vkCmdSetStencilWriteMask", vkCmdSetStencilWriteMask },
		{ "vkCmdSetStencilReference", vkCmdSetStencilReference },
		{ "vkCmdBindDescriptorSets", vkCmdBindDescriptorSets },
		{ "vkCmdBindIndexBuffer", vkCmdBindIndexBuffer },
		{ "vkCmdBindVertexBuffers", vkCmdBindVertexBuffers },
		{ "vkCmdDraw", vkCmdDraw },
		{ "vkCmdDrawIndexed", vkCmdDrawIndexed },
		{ "vkCmdDrawIndirect", vkCmdDrawIndirect },
		{ "vkCmdDrawIndexedIndirect", vkCmdDrawIndexedIndirect },
		{ "vkCmdDispatch", vkCmdDispatch },
		{ "vkCmdDispatchIndirect", vkCmdDispatchIndirect },
		{ "vkCmdCopyBuffer", vkCmdCopyBuffer },
		{ "vkCmdCopyImage", vkCmdCopyImage },
		{ "vkCmdBlitImage", vkCmdBlitImage },
		{ "vkCmdCopyBufferToImage", vkCmdCopyBufferToImage },
		{ "vkCmdCopyImageToBuffer",vkCmdCopyImageToBuffer },
		{ "vkCmdUpdateBuffer", vkCmdUpdateBuffer },
		{ "vkCmdFillBuffer", vkCmdFillBuffer },
		{ "vkCmdClearColorImage", vkCmdClearColorImage },
		{ "vkCmdClearDepthStencilImage",vkCmdClearDepthStencilImage },
		{ "vkCmdClearAttachments", vkCmdClearAttachments },
		{ "vkCmdResolveImage", vkCmdResolveImage },
		{ "vkCmdSetEvent", vkCmdSetEvent },
		{ "vkCmdResetEvent", vkCmdResetEvent },
		{ "vkCmdWaitEvents", vkCmdWaitEvents },
		{ "vkCmdPipelineBarrier",vkCmdPipelineBarrier },
		{ "vkCmdBeginQuery", vkCmdBeginQuery },
		{ "vkCmdEndQuery", vkCmdEndQuery },
		{ "vkCmdResetQueryPool",vkCmdResetQueryPool },
		{ "vkCmdWriteTimestamp", vkCmdWriteTimestamp },
		{ "vkCmdCopyQueryPoolResults", vkCmdCopyQueryPoolResults },
		{ "vkCmdPushConstants",vkCmdPushConstants },
		{ "vkCmdBeginRenderPass", vkCmdBeginRenderPass },
		{ "vkCmdNextSubpass", vkCmdNextSubpass },
		{ "vkCmdEndRenderPass", vkCmdEndRenderPass }, 
		{ "vkCmdExecuteCommands", vkCmdExecuteCommands }, 
		{ "vkDestroySurfaceKHR", vkDestroySurfaceKHR }, 
		{ "vkGetPhysicalDeviceSurfaceSupportKHR", vkGetPhysicalDeviceSurfaceSupportKHR }, 
		{ "vkGetPhysicalDeviceSurfaceCapabilitiesKHR",vkGetPhysicalDeviceSurfaceCapabilitiesKHR },
		{ "vkGetPhysicalDeviceSurfaceFormatsKHR", vkGetPhysicalDeviceSurfaceFormatsKHR }, 
		{ "vkGetPhysicalDeviceSurfacePresentModesKHR",vkGetPhysicalDeviceSurfacePresentModesKHR }, 
		{ "vkCreateSwapchainKHR", vkCreateSwapchainKHR }, 
		{ "vkDestroySwapchainKHR", vkDestroySwapchainKHR }, 
		{ "vkGetSwapchainImagesKHR", vkGetSwapchainImagesKHR }, 
		{ "vkAcquireNextImageKHR", vkAcquireNextImageKHR }, 
		{ "vkQueuePresentKHR", vkQueuePresentKHR }, 
		{ "vkGetPhysicalDeviceDisplayPropertiesKHR", vkGetPhysicalDeviceDisplayPropertiesKHR }, 
		{ "vkGetPhysicalDeviceDisplayPlanePropertiesKHR", vkGetPhysicalDeviceDisplayPlanePropertiesKHR }, 
		{ "vkGetDisplayPlaneSupportedDisplaysKHR", vkGetDisplayPlaneSupportedDisplaysKHR }, 
		{ "vkGetDisplayModePropertiesKHR", vkGetDisplayModePropertiesKHR }, 
		{ "vkCreateDisplayModeKHR", vkCreateDisplayModeKHR }, 
		{ "vkGetDisplayPlaneCapabilitiesKHR", vkGetDisplayPlaneCapabilitiesKHR }, 
		{ "vkCreateDisplayPlaneSurfaceKHR", vkCreateDisplayPlaneSurfaceKHR }, 
		{ "vkCreateSharedSwapchainsKHR", vkCreateSharedSwapchainsKHR }, 
		{ "vkCreateWin32SurfaceKHR", vkCreateWin32SurfaceKHR }, 
		{ "vkTrimCommandPoolKHR", vkTrimCommandPoolKHR }, 
		{ "vkCmdPushDescriptorSetKHR", vkCmdPushDescriptorSetKHR }, 
		{ "vkCreateDescriptorUpdateTemplateKHR", vkCreateDescriptorUpdateTemplateKHR }, 
		{ "vkDestroyDescriptorUpdateTemplateKHR", vkDestroyDescriptorUpdateTemplateKHR }, 
		{ "vkUpdateDescriptorSetWithTemplateKHR", vkUpdateDescriptorSetWithTemplateKHR }, 
		{ "vkCmdPushDescriptorSetWithTemplateKHR", vkCmdPushDescriptorSetWithTemplateKHR }, 
		{ "vkGetSwapchainStatusKHR", vkGetSwapchainStatusKHR }, 
		{ "vkCreateDebugReportCallbackEXT", vkCreateDebugReportCallbackEXT }, 
		{ "vkDestroyDebugReportCallbackEXT", vkDestroyDebugReportCallbackEXT }, 
		{ "vkDebugReportMessageEXT", vkDebugReportMessageEXT }, 
		{ "vkDebugMarkerSetObjectTagEXT", vkDebugMarkerSetObjectTagEXT }, 
		{ "vkDebugMarkerSetObjectNameEXT",vkDebugMarkerSetObjectNameEXT }, 
		{ "vkCmdDebugMarkerBeginEXT", vkCmdDebugMarkerBeginEXT }, 
		{ "vkCmdDebugMarkerEndEXT", vkCmdDebugMarkerEndEXT }, 
		{ "vkCmdDebugMarkerEndEXT",vkCmdDebugMarkerEndEXT }, 
		{ "vkReleaseDisplayEXT", vkReleaseDisplayEXT }, 
		{ "vkDisplayPowerControlEXT", vkDisplayPowerControlEXT }, 
		{ "vkRegisterDeviceEventEXT", vkRegisterDeviceEventEXT }, 
		{ "vkRegisterDisplayEventEXT", vkRegisterDisplayEventEXT }, 
		{ "vkGetSwapchainCounterEXT", vkGetSwapchainCounterEXT }
	};


	struct MemoryType
	{
		VkMemoryPropertyFlags propertyFlags;
		uint32_t heapIndex; 

		VkBufferUsageFlagBits validUsage;
	};

	struct MemoryHeap
	{
		VkDeviceSize size; 
		VkMemoryHeapFlags flags;
	};

	struct PhysicalDevice 
	{
		struct Instance*    instance;
		const char*         name;

		struct  {
			uint32_t                           type_count;
			struct MemoryType                  types[VK_MAX_MEMORY_TYPES];
			uint32_t                           heap_count;
			struct MemoryHeap                  heaps[VK_MAX_MEMORY_HEAPS];
		} memory;
	};

	struct Instance 
	{
		VkAllocationCallbacks                       alloc;

		uint32_t                                    apiVersion;
		int                                         physicalDeviceNum;
		struct PhysicalDevice                       physicalDevice;
	};

	struct Device
	{
		struct Instance *instance;
		VkAllocationCallbacks alloc;
		bool robustBufferAccess;
	};
}
#endif

