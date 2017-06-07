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

	struct PhysicalDevice 
	{
		struct Instance*                       instance;

	};

	struct Instance 
	{
		VkAllocationCallbacks                       alloc;

		uint32_t                                    apiVersion;
		int                                         physicalDeviceNum;
		struct PhysicalDevice                       physicalDevice;
	};
}
#endif

