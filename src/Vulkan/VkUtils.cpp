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

#include "VkDebug.hpp"
#include "VkUtils.h"

#include <unordered_map>

namespace vkutils
{
	#define DRIVER_VERSION 1
	#define VENDOR_ID 0x1AE0 // Google
	#define DEVICE_ID 0xC0DE // SwiftShader

	const VkSampleCountFlags GetSampleCounts()
	{
		return VK_SAMPLE_COUNT_1_BIT | VK_SAMPLE_COUNT_4_BIT;
	}

	const VkPhysicalDeviceLimits& GetPhysicalDeviceLimits()
	{
		VkSampleCountFlags sampleCounts = GetSampleCounts();

		static const VkPhysicalDeviceLimits limits = {
			(1 << MaxImageLevels1D), // maxImageDimension1D
			(1 << MaxImageLevels2D), // maxImageDimension2D
			(1 << MaxImageLevels3D), // maxImageDimension3D
			(1 << MaxImageLevelsCube), // maxImageDimensionCube
			(1 << MaxImageArrayLayers), // maxImageArrayLayers
			128 * 1024 * 1024, // maxTexelBufferElements
			(1ul << 27), // maxUniformBufferRange
			16, // maxStorageBufferRange
			128, // maxPushConstantsSize
			UINT32_MAX, // maxMemoryAllocationCount
			64 * 1024, // maxSamplerAllocationCount
			64, // bufferImageGranularity
			0, // sparseAddressSpaceSize
			8, // maxBoundDescriptorSets
			64, // maxPerStageDescriptorSamplers
			64, // maxPerStageDescriptorUniformBuffers
			64, // maxPerStageDescriptorStorageBuffers
			64, // maxPerStageDescriptorSampledImages
			64, // maxPerStageDescriptorStorageImages
			64, // maxPerStageDescriptorInputAttachments
			128, // maxPerStageResources
			256, // maxDescriptorSetSamplers
			256, // maxDescriptorSetUniformBuffers
			8, // maxDescriptorSetUniformBuffersDynamic
			256, // maxDescriptorSetStorageBuffers
			8, // maxDescriptorSetStorageBuffersDynamic
			256, // maxDescriptorSetSampledImages
			256, // maxDescriptorSetStorageImages
			256, // maxDescriptorSetInputAttachments
			31, // maxVertexInputAttributes
			31, // maxVertexInputBindings
			2047, // maxVertexInputAttributeOffset
			2048, // maxVertexInputBindingStride
			128, // maxVertexOutputComponents
			64, // maxTessellationGenerationLevel
			32, // maxTessellationPatchSize
			128, // maxTessellationControlPerVertexInputComponents
			128, // maxTessellationControlPerVertexOutputComponents
			128, // maxTessellationControlPerPatchOutputComponents
			2048, // maxTessellationControlTotalOutputComponents
			128, // maxTessellationEvaluationInputComponents
			128, // maxTessellationEvaluationOutputComponents
			32, // maxGeometryShaderInvocations
			64, // maxGeometryInputComponents
			128, // maxGeometryOutputComponents
			256, // maxGeometryOutputVertices
			1024, // maxGeometryTotalOutputComponents
			128, // maxFragmentInputComponents
			8, // maxFragmentOutputAttachments
			1, // maxFragmentDualSrcAttachments
			8, // maxFragmentCombinedOutputResources
			32768, // maxComputeSharedMemorySize
			{ 65535, 65535, 65535 }, // maxComputeWorkGroupCount[3]
			16, // maxComputeWorkGroupInvocations
			{ 16, 16, 16, }, // maxComputeWorkGroupSize[3]
			4, // subPixelPrecisionBits
			4, // subTexelPrecisionBits
			4, // mipmapPrecisionBits
			UINT32_MAX, // maxDrawIndexedIndexValue
			UINT32_MAX, // maxDrawIndirectCount
			16, // maxSamplerLodBias
			16, // maxSamplerAnisotropy
			16, // maxViewports
			{ (1 << 14), (1 << 14) }, // maxViewportDimensions[2]
			{ INT16_MIN, INT16_MAX }, // viewportBoundsRange[2]
			13, // viewportSubPixelBits
			4096, // minMemoryMapAlignment
			1, // minTexelBufferOffsetAlignment
			16, // minUniformBufferOffsetAlignment
			4, // minStorageBufferOffsetAlignment
			-8, // minTexelOffset
			7, // maxTexelOffset
			-32, // minTexelGatherOffset
			31, // maxTexelGatherOffset
			-0.5, // minInterpolationOffset
			0.4375, // maxInterpolationOffset
			4, // subPixelInterpolationOffsetBits
			(1 << 14), // maxFramebufferWidth
			(1 << 14), // maxFramebufferHeight
			(1 << 11), // maxFramebufferLayers
			sampleCounts, // framebufferColorSampleCounts
			sampleCounts, // framebufferDepthSampleCounts
			sampleCounts, // framebufferStencilSampleCounts
			sampleCounts, // framebufferNoAttachmentsSampleCounts
			8,  // maxColorAttachments
			sampleCounts, // sampledImageColorSampleCounts
			VK_SAMPLE_COUNT_1_BIT, // sampledImageIntegerSampleCounts
			sampleCounts, // sampledImageDepthSampleCounts
			sampleCounts, // sampledImageStencilSampleCounts
			VK_SAMPLE_COUNT_1_BIT, // storageImageSampleCounts
			1, // maxSampleMaskWords
			false, // timestampComputeAndGraphics
			60, // timestampPeriod
			8, // maxClipDistances
			8, // maxCullDistances
			8, // maxCombinedClipAndCullDistances
			1, // discreteQueuePriorities
			{ 0.125, 255.875 }, // pointSizeRange[2]
			{ 0.0, 7.9921875 }, // lineWidthRange[2]
			(1.0 / 8.0), // pointSizeGranularity
			(1.0 / 128.0), // lineWidthGranularity
			false, // strictLines
			true, // standardSampleLocations
			128, // optimalBufferCopyOffsetAlignment
			128, // optimalBufferCopyRowPitchAlignment
			64 // nonCoherentAtomSize
		};

		return limits;
	}

	const VkPhysicalDeviceProperties GetPhysicalDeviceProperties()
	{
		uint32_t apiVersion;
		VkResult result = vkEnumerateInstanceVersion(&apiVersion);
		ASSERT(result == VK_SUCCESS);

		return VkPhysicalDeviceProperties{
			apiVersion,
			DRIVER_VERSION,
			VENDOR_ID,
			DEVICE_ID,
			VK_PHYSICAL_DEVICE_TYPE_CPU, // deviceType
			"SwiftShader Device", // deviceName
			{ 0 }, // pipelineCacheUUID
			vkutils::GetPhysicalDeviceLimits(), // limits
			{ 0 } // sparseProperties
		};
	}

	const VkPhysicalDeviceFeatures& GetPhysicalDeviceFeatures()
	{
		static const VkPhysicalDeviceFeatures features{
			false, // robustBufferAccess
			false, // fullDrawIndexUint32
			false, // imageCubeArray
			false, // independentBlend
			false, // geometryShader
			false, // tessellationShader
			false, // sampleRateShading
			false, // dualSrcBlend
			false, // logicOp
			false, // multiDrawIndirect
			false, // drawIndirectFirstInstance
			false, // depthClamp
			false, // depthBiasClamp
			false, // fillModeNonSolid
			false, // depthBounds
			false, // wideLines
			false, // largePoints
			false, // alphaToOne
			false, // multiViewport
			false, // samplerAnisotropy
			false, // textureCompressionETC2
			false, // textureCompressionASTC_LDR
			false, // textureCompressionBC
			false, // occlusionQueryPrecise
			false, // pipelineStatisticsQuery
			false, // vertexPipelineStoresAndAtomics
			false, // fragmentStoresAndAtomics
			false, // shaderTessellationAndGeometryPointSize
			false, // shaderImageGatherExtended
			false, // shaderStorageImageExtendedFormats
			false, // shaderStorageImageMultisample
			false, // shaderStorageImageReadWithoutFormat
			false, // shaderStorageImageWriteWithoutFormat
			false, // shaderUniformBufferArrayDynamicIndexing
			false, // shaderSampledImageArrayDynamicIndexing
			false, // shaderStorageBufferArrayDynamicIndexing
			false, // shaderStorageImageArrayDynamicIndexing
			false, // shaderClipDistance
			false, // shaderCullDistance
			false, // shaderFloat64
			false, // shaderInt64
			false, // shaderInt16
			false, // shaderResourceResidency
			false, // shaderResourceMinLod
			false, // sparseBinding
			false, // sparseResidencyBuffer
			false, // sparseResidencyImage2D
			false, // sparseResidencyImage3D
			false, // sparseResidency2Samples
			false, // sparseResidency4Samples
			false, // sparseResidency8Samples
			false, // sparseResidency16Samples
			false, // sparseResidencyAliased
			false, // variableMultisampleRate
			false, // inheritedQueries
		};

		return features;
	}

	bool AreRequestedFeaturesPresent(const VkPhysicalDeviceFeatures& requestedFeatures)
	{
		const VkPhysicalDeviceFeatures& availableFeatures = GetPhysicalDeviceFeatures();

		return (!requestedFeatures.robustBufferAccess || availableFeatures.robustBufferAccess) &&
		       (!requestedFeatures.fullDrawIndexUint32 || availableFeatures.fullDrawIndexUint32) &&
		       (!requestedFeatures.imageCubeArray || availableFeatures.imageCubeArray) &&
		       (!requestedFeatures.independentBlend || availableFeatures.independentBlend) &&
		       (!requestedFeatures.geometryShader || availableFeatures.geometryShader) &&
		       (!requestedFeatures.tessellationShader || availableFeatures.tessellationShader) &&
		       (!requestedFeatures.sampleRateShading || availableFeatures.sampleRateShading) &&
		       (!requestedFeatures.dualSrcBlend || availableFeatures.dualSrcBlend) &&
		       (!requestedFeatures.logicOp || availableFeatures.logicOp) &&
		       (!requestedFeatures.multiDrawIndirect || availableFeatures.multiDrawIndirect) &&
		       (!requestedFeatures.drawIndirectFirstInstance || availableFeatures.drawIndirectFirstInstance) &&
		       (!requestedFeatures.depthClamp || availableFeatures.depthClamp) &&
		       (!requestedFeatures.depthBiasClamp || availableFeatures.depthBiasClamp) &&
		       (!requestedFeatures.fillModeNonSolid || availableFeatures.fillModeNonSolid) &&
		       (!requestedFeatures.depthBounds || availableFeatures.depthBounds) &&
		       (!requestedFeatures.wideLines || availableFeatures.wideLines) &&
		       (!requestedFeatures.largePoints || availableFeatures.largePoints) &&
		       (!requestedFeatures.alphaToOne || availableFeatures.alphaToOne) &&
		       (!requestedFeatures.multiViewport || availableFeatures.multiViewport) &&
		       (!requestedFeatures.samplerAnisotropy || availableFeatures.samplerAnisotropy) &&
		       (!requestedFeatures.textureCompressionETC2 || availableFeatures.textureCompressionETC2) &&
		       (!requestedFeatures.textureCompressionASTC_LDR || availableFeatures.textureCompressionASTC_LDR) &&
		       (!requestedFeatures.textureCompressionBC || availableFeatures.textureCompressionBC) &&
		       (!requestedFeatures.occlusionQueryPrecise || availableFeatures.occlusionQueryPrecise) &&
		       (!requestedFeatures.pipelineStatisticsQuery || availableFeatures.pipelineStatisticsQuery) &&
		       (!requestedFeatures.vertexPipelineStoresAndAtomics || availableFeatures.vertexPipelineStoresAndAtomics) &&
		       (!requestedFeatures.fragmentStoresAndAtomics || availableFeatures.fragmentStoresAndAtomics) &&
		       (!requestedFeatures.shaderTessellationAndGeometryPointSize || availableFeatures.shaderTessellationAndGeometryPointSize) &&
		       (!requestedFeatures.shaderImageGatherExtended || availableFeatures.shaderImageGatherExtended) &&
		       (!requestedFeatures.shaderStorageImageExtendedFormats || availableFeatures.shaderStorageImageExtendedFormats) &&
		       (!requestedFeatures.shaderStorageImageMultisample || availableFeatures.shaderStorageImageMultisample) &&
		       (!requestedFeatures.shaderStorageImageReadWithoutFormat || availableFeatures.shaderStorageImageReadWithoutFormat) &&
		       (!requestedFeatures.shaderStorageImageWriteWithoutFormat || availableFeatures.shaderStorageImageWriteWithoutFormat) &&
		       (!requestedFeatures.shaderUniformBufferArrayDynamicIndexing || availableFeatures.shaderUniformBufferArrayDynamicIndexing) &&
		       (!requestedFeatures.shaderSampledImageArrayDynamicIndexing || availableFeatures.shaderSampledImageArrayDynamicIndexing) &&
		       (!requestedFeatures.shaderStorageBufferArrayDynamicIndexing || availableFeatures.shaderStorageBufferArrayDynamicIndexing) &&
		       (!requestedFeatures.shaderStorageImageArrayDynamicIndexing || availableFeatures.shaderStorageImageArrayDynamicIndexing) &&
		       (!requestedFeatures.shaderClipDistance || availableFeatures.shaderClipDistance) &&
		       (!requestedFeatures.shaderCullDistance || availableFeatures.shaderCullDistance) &&
		       (!requestedFeatures.shaderFloat64 || availableFeatures.shaderFloat64) &&
		       (!requestedFeatures.shaderInt64 || availableFeatures.shaderInt64) &&
		       (!requestedFeatures.shaderInt16 || availableFeatures.shaderInt16) &&
		       (!requestedFeatures.shaderResourceResidency || availableFeatures.shaderResourceResidency) &&
		       (!requestedFeatures.shaderResourceMinLod || availableFeatures.shaderResourceMinLod) &&
		       (!requestedFeatures.sparseBinding || availableFeatures.sparseBinding) &&
		       (!requestedFeatures.sparseResidencyBuffer || availableFeatures.sparseResidencyBuffer) &&
		       (!requestedFeatures.sparseResidencyImage2D || availableFeatures.sparseResidencyImage2D) &&
		       (!requestedFeatures.sparseResidencyImage3D || availableFeatures.sparseResidencyImage3D) &&
		       (!requestedFeatures.sparseResidency2Samples || availableFeatures.sparseResidency2Samples) &&
		       (!requestedFeatures.sparseResidency4Samples || availableFeatures.sparseResidency4Samples) &&
		       (!requestedFeatures.sparseResidency8Samples || availableFeatures.sparseResidency8Samples) &&
		       (!requestedFeatures.sparseResidency16Samples || availableFeatures.sparseResidency16Samples) &&
		       (!requestedFeatures.sparseResidencyAliased || availableFeatures.sparseResidencyAliased) &&
		       (!requestedFeatures.variableMultisampleRate || availableFeatures.variableMultisampleRate) &&
		       (!requestedFeatures.inheritedQueries || availableFeatures.inheritedQueries);
	}

	PFN_vkVoidFunction GetProcAddr(const char* pName)
	{
		#define MAKE_VULKAN_ENTRY(aFunction) { #aFunction, reinterpret_cast<PFN_vkVoidFunction>(aFunction) }
		static const std::unordered_map <std::string, PFN_vkVoidFunction> func_ptrs = {
			MAKE_VULKAN_ENTRY(vkCreateInstance),
			MAKE_VULKAN_ENTRY(vkDestroyInstance),
			MAKE_VULKAN_ENTRY(vkEnumeratePhysicalDevices),
			MAKE_VULKAN_ENTRY(vkGetPhysicalDeviceFeatures),
			MAKE_VULKAN_ENTRY(vkGetPhysicalDeviceFormatProperties),
			MAKE_VULKAN_ENTRY(vkGetPhysicalDeviceImageFormatProperties),
			MAKE_VULKAN_ENTRY(vkGetPhysicalDeviceProperties),
			MAKE_VULKAN_ENTRY(vkGetPhysicalDeviceQueueFamilyProperties),
			MAKE_VULKAN_ENTRY(vkGetPhysicalDeviceMemoryProperties),
			MAKE_VULKAN_ENTRY(vkGetInstanceProcAddr),
			MAKE_VULKAN_ENTRY(vkGetDeviceProcAddr),
			MAKE_VULKAN_ENTRY(vkCreateDevice),
			MAKE_VULKAN_ENTRY(vkDestroyDevice),
			MAKE_VULKAN_ENTRY(vkEnumerateInstanceExtensionProperties),
			MAKE_VULKAN_ENTRY(vkEnumerateDeviceExtensionProperties),
			MAKE_VULKAN_ENTRY(vkEnumerateInstanceLayerProperties),
			MAKE_VULKAN_ENTRY(vkEnumerateDeviceLayerProperties),
			MAKE_VULKAN_ENTRY(vkGetDeviceQueue),
			MAKE_VULKAN_ENTRY(vkQueueSubmit),
			MAKE_VULKAN_ENTRY(vkQueueWaitIdle),
			MAKE_VULKAN_ENTRY(vkDeviceWaitIdle),
			MAKE_VULKAN_ENTRY(vkAllocateMemory),
			MAKE_VULKAN_ENTRY(vkFreeMemory),
			MAKE_VULKAN_ENTRY(vkMapMemory),
			MAKE_VULKAN_ENTRY(vkUnmapMemory),
			MAKE_VULKAN_ENTRY(vkFlushMappedMemoryRanges),
			MAKE_VULKAN_ENTRY(vkInvalidateMappedMemoryRanges),
			MAKE_VULKAN_ENTRY(vkGetDeviceMemoryCommitment),
			MAKE_VULKAN_ENTRY(vkBindBufferMemory),
			MAKE_VULKAN_ENTRY(vkBindImageMemory),
			MAKE_VULKAN_ENTRY(vkGetBufferMemoryRequirements),
			MAKE_VULKAN_ENTRY(vkGetImageMemoryRequirements),
			MAKE_VULKAN_ENTRY(vkGetImageSparseMemoryRequirements),
			MAKE_VULKAN_ENTRY(vkGetPhysicalDeviceSparseImageFormatProperties),
			MAKE_VULKAN_ENTRY(vkQueueBindSparse),
			MAKE_VULKAN_ENTRY(vkCreateFence),
			MAKE_VULKAN_ENTRY(vkDestroyFence),
			MAKE_VULKAN_ENTRY(vkResetFences),
			MAKE_VULKAN_ENTRY(vkGetFenceStatus),
			MAKE_VULKAN_ENTRY(vkWaitForFences),
			MAKE_VULKAN_ENTRY(vkCreateSemaphore),
			MAKE_VULKAN_ENTRY(vkDestroySemaphore),
			MAKE_VULKAN_ENTRY(vkCreateEvent),
			MAKE_VULKAN_ENTRY(vkDestroyEvent),
			MAKE_VULKAN_ENTRY(vkGetEventStatus),
			MAKE_VULKAN_ENTRY(vkSetEvent),
			MAKE_VULKAN_ENTRY(vkResetEvent),
			MAKE_VULKAN_ENTRY(vkCreateQueryPool),
			MAKE_VULKAN_ENTRY(vkDestroyQueryPool),
			MAKE_VULKAN_ENTRY(vkGetQueryPoolResults),
			MAKE_VULKAN_ENTRY(vkCreateBuffer),
			MAKE_VULKAN_ENTRY(vkDestroyBuffer),
			MAKE_VULKAN_ENTRY(vkCreateBufferView),
			MAKE_VULKAN_ENTRY(vkDestroyBufferView),
			MAKE_VULKAN_ENTRY(vkCreateImage),
			MAKE_VULKAN_ENTRY(vkDestroyImage),
			MAKE_VULKAN_ENTRY(vkGetImageSubresourceLayout),
			MAKE_VULKAN_ENTRY(vkCreateImageView),
			MAKE_VULKAN_ENTRY(vkDestroyImageView),
			MAKE_VULKAN_ENTRY(vkCreateShaderModule),
			MAKE_VULKAN_ENTRY(vkDestroyShaderModule),
			MAKE_VULKAN_ENTRY(vkCreatePipelineCache),
			MAKE_VULKAN_ENTRY(vkDestroyPipelineCache),
			MAKE_VULKAN_ENTRY(vkGetPipelineCacheData),
			MAKE_VULKAN_ENTRY(vkMergePipelineCaches),
			MAKE_VULKAN_ENTRY(vkCreateGraphicsPipelines),
			MAKE_VULKAN_ENTRY(vkCreateComputePipelines),
			MAKE_VULKAN_ENTRY(vkDestroyPipeline),
			MAKE_VULKAN_ENTRY(vkCreatePipelineLayout),
			MAKE_VULKAN_ENTRY(vkDestroyPipelineLayout),
			MAKE_VULKAN_ENTRY(vkCreateSampler),
			MAKE_VULKAN_ENTRY(vkDestroySampler),
			MAKE_VULKAN_ENTRY(vkCreateDescriptorSetLayout),
			MAKE_VULKAN_ENTRY(vkDestroyDescriptorSetLayout),
			MAKE_VULKAN_ENTRY(vkCreateDescriptorPool),
			MAKE_VULKAN_ENTRY(vkDestroyDescriptorPool),
			MAKE_VULKAN_ENTRY(vkResetDescriptorPool),
			MAKE_VULKAN_ENTRY(vkAllocateDescriptorSets),
			MAKE_VULKAN_ENTRY(vkFreeDescriptorSets),
			MAKE_VULKAN_ENTRY(vkUpdateDescriptorSets),
			MAKE_VULKAN_ENTRY(vkCreateFramebuffer),
			MAKE_VULKAN_ENTRY(vkDestroyFramebuffer),
			MAKE_VULKAN_ENTRY(vkCreateRenderPass),
			MAKE_VULKAN_ENTRY(vkDestroyRenderPass),
			MAKE_VULKAN_ENTRY(vkGetRenderAreaGranularity),
			MAKE_VULKAN_ENTRY(vkCreateCommandPool),
			MAKE_VULKAN_ENTRY(vkDestroyCommandPool),
			MAKE_VULKAN_ENTRY(vkResetCommandPool),
			MAKE_VULKAN_ENTRY(vkAllocateCommandBuffers),
			MAKE_VULKAN_ENTRY(vkFreeCommandBuffers),
			MAKE_VULKAN_ENTRY(vkBeginCommandBuffer),
			MAKE_VULKAN_ENTRY(vkEndCommandBuffer),
			MAKE_VULKAN_ENTRY(vkResetCommandBuffer),
			MAKE_VULKAN_ENTRY(vkCmdBindPipeline),
			MAKE_VULKAN_ENTRY(vkCmdSetViewport),
			MAKE_VULKAN_ENTRY(vkCmdSetScissor),
			MAKE_VULKAN_ENTRY(vkCmdSetLineWidth),
			MAKE_VULKAN_ENTRY(vkCmdSetDepthBias),
			MAKE_VULKAN_ENTRY(vkCmdSetBlendConstants),
			MAKE_VULKAN_ENTRY(vkCmdSetDepthBounds),
			MAKE_VULKAN_ENTRY(vkCmdSetStencilCompareMask),
			MAKE_VULKAN_ENTRY(vkCmdSetStencilWriteMask),
			MAKE_VULKAN_ENTRY(vkCmdSetStencilReference),
			MAKE_VULKAN_ENTRY(vkCmdBindDescriptorSets),
			MAKE_VULKAN_ENTRY(vkCmdBindIndexBuffer),
			MAKE_VULKAN_ENTRY(vkCmdBindVertexBuffers),
			MAKE_VULKAN_ENTRY(vkCmdDraw),
			MAKE_VULKAN_ENTRY(vkCmdDrawIndexed),
			MAKE_VULKAN_ENTRY(vkCmdDrawIndirect),
			MAKE_VULKAN_ENTRY(vkCmdDrawIndexedIndirect),
			MAKE_VULKAN_ENTRY(vkCmdDispatch),
			MAKE_VULKAN_ENTRY(vkCmdDispatchIndirect),
			MAKE_VULKAN_ENTRY(vkCmdCopyBuffer),
			MAKE_VULKAN_ENTRY(vkCmdCopyImage),
			MAKE_VULKAN_ENTRY(vkCmdBlitImage),
			MAKE_VULKAN_ENTRY(vkCmdCopyBufferToImage),
			MAKE_VULKAN_ENTRY(vkCmdCopyImageToBuffer),
			MAKE_VULKAN_ENTRY(vkCmdUpdateBuffer),
			MAKE_VULKAN_ENTRY(vkCmdFillBuffer),
			MAKE_VULKAN_ENTRY(vkCmdClearColorImage),
			MAKE_VULKAN_ENTRY(vkCmdClearDepthStencilImage),
			MAKE_VULKAN_ENTRY(vkCmdClearAttachments),
			MAKE_VULKAN_ENTRY(vkCmdResolveImage),
			MAKE_VULKAN_ENTRY(vkCmdSetEvent),
			MAKE_VULKAN_ENTRY(vkCmdResetEvent),
			MAKE_VULKAN_ENTRY(vkCmdWaitEvents),
			MAKE_VULKAN_ENTRY(vkCmdPipelineBarrier),
			MAKE_VULKAN_ENTRY(vkCmdBeginQuery),
			MAKE_VULKAN_ENTRY(vkCmdEndQuery),
			MAKE_VULKAN_ENTRY(vkCmdResetQueryPool),
			MAKE_VULKAN_ENTRY(vkCmdWriteTimestamp),
			MAKE_VULKAN_ENTRY(vkCmdCopyQueryPoolResults),
			MAKE_VULKAN_ENTRY(vkCmdPushConstants),
			MAKE_VULKAN_ENTRY(vkCmdBeginRenderPass),
			MAKE_VULKAN_ENTRY(vkCmdNextSubpass),
			MAKE_VULKAN_ENTRY(vkCmdEndRenderPass),
			MAKE_VULKAN_ENTRY(vkCmdExecuteCommands),
			MAKE_VULKAN_ENTRY(vkEnumerateInstanceVersion),
			MAKE_VULKAN_ENTRY(vkBindBufferMemory2),
			MAKE_VULKAN_ENTRY(vkBindImageMemory2),
			MAKE_VULKAN_ENTRY(vkGetDeviceGroupPeerMemoryFeatures),
			MAKE_VULKAN_ENTRY(vkCmdSetDeviceMask),
			MAKE_VULKAN_ENTRY(vkCmdDispatchBase),
			MAKE_VULKAN_ENTRY(vkEnumeratePhysicalDeviceGroups),
			MAKE_VULKAN_ENTRY(vkGetImageMemoryRequirements2),
			MAKE_VULKAN_ENTRY(vkGetBufferMemoryRequirements2),
			MAKE_VULKAN_ENTRY(vkGetImageSparseMemoryRequirements2),
			MAKE_VULKAN_ENTRY(vkGetPhysicalDeviceFeatures2),
			MAKE_VULKAN_ENTRY(vkGetPhysicalDeviceProperties2),
			MAKE_VULKAN_ENTRY(vkGetPhysicalDeviceFormatProperties2),
			MAKE_VULKAN_ENTRY(vkGetPhysicalDeviceImageFormatProperties2),
			MAKE_VULKAN_ENTRY(vkGetPhysicalDeviceQueueFamilyProperties2),
			MAKE_VULKAN_ENTRY(vkGetPhysicalDeviceMemoryProperties2),
			MAKE_VULKAN_ENTRY(vkGetPhysicalDeviceSparseImageFormatProperties2),
			MAKE_VULKAN_ENTRY(vkTrimCommandPool),
			MAKE_VULKAN_ENTRY(vkGetDeviceQueue2),
			MAKE_VULKAN_ENTRY(vkCreateSamplerYcbcrConversion),
			MAKE_VULKAN_ENTRY(vkDestroySamplerYcbcrConversion),
			MAKE_VULKAN_ENTRY(vkCreateDescriptorUpdateTemplate),
			MAKE_VULKAN_ENTRY(vkDestroyDescriptorUpdateTemplate),
			MAKE_VULKAN_ENTRY(vkUpdateDescriptorSetWithTemplate),
			MAKE_VULKAN_ENTRY(vkGetPhysicalDeviceExternalBufferProperties),
			MAKE_VULKAN_ENTRY(vkGetPhysicalDeviceExternalFenceProperties),
			MAKE_VULKAN_ENTRY(vkGetPhysicalDeviceExternalSemaphoreProperties),
			MAKE_VULKAN_ENTRY(vkGetDescriptorSetLayoutSupport),
			MAKE_VULKAN_ENTRY(vkDestroySurfaceKHR),
			MAKE_VULKAN_ENTRY(vkGetPhysicalDeviceSurfaceSupportKHR),
			MAKE_VULKAN_ENTRY(vkGetPhysicalDeviceSurfaceCapabilitiesKHR),
			MAKE_VULKAN_ENTRY(vkGetPhysicalDeviceSurfaceFormatsKHR),
			MAKE_VULKAN_ENTRY(vkGetPhysicalDeviceSurfacePresentModesKHR),
			MAKE_VULKAN_ENTRY(vkCreateSwapchainKHR),
			MAKE_VULKAN_ENTRY(vkDestroySwapchainKHR),
			MAKE_VULKAN_ENTRY(vkGetSwapchainImagesKHR),
			MAKE_VULKAN_ENTRY(vkAcquireNextImageKHR),
			MAKE_VULKAN_ENTRY(vkQueuePresentKHR),
			MAKE_VULKAN_ENTRY(vkGetDeviceGroupPresentCapabilitiesKHR),
			MAKE_VULKAN_ENTRY(vkGetDeviceGroupSurfacePresentModesKHR),
			MAKE_VULKAN_ENTRY(vkGetPhysicalDevicePresentRectanglesKHR),
			MAKE_VULKAN_ENTRY(vkAcquireNextImage2KHR),
			MAKE_VULKAN_ENTRY(vkGetPhysicalDeviceDisplayPropertiesKHR),
			MAKE_VULKAN_ENTRY(vkGetPhysicalDeviceDisplayPlanePropertiesKHR),
			MAKE_VULKAN_ENTRY(vkGetDisplayPlaneSupportedDisplaysKHR),
			MAKE_VULKAN_ENTRY(vkGetDisplayModePropertiesKHR),
			MAKE_VULKAN_ENTRY(vkCreateDisplayModeKHR),
			MAKE_VULKAN_ENTRY(vkGetDisplayPlaneCapabilitiesKHR),
			MAKE_VULKAN_ENTRY(vkCreateDisplayPlaneSurfaceKHR),
			MAKE_VULKAN_ENTRY(vkCreateSharedSwapchainsKHR),
			MAKE_VULKAN_ENTRY(vkGetPhysicalDeviceFeatures2KHR),
			MAKE_VULKAN_ENTRY(vkGetPhysicalDeviceProperties2KHR),
			MAKE_VULKAN_ENTRY(vkGetPhysicalDeviceFormatProperties2KHR),
			MAKE_VULKAN_ENTRY(vkGetPhysicalDeviceImageFormatProperties2KHR),
			MAKE_VULKAN_ENTRY(vkGetPhysicalDeviceQueueFamilyProperties2KHR),
			MAKE_VULKAN_ENTRY(vkGetPhysicalDeviceMemoryProperties2KHR),
			MAKE_VULKAN_ENTRY(vkGetPhysicalDeviceSparseImageFormatProperties2KHR),
			MAKE_VULKAN_ENTRY(vkGetDeviceGroupPeerMemoryFeaturesKHR),
			MAKE_VULKAN_ENTRY(vkCmdSetDeviceMaskKHR),
			MAKE_VULKAN_ENTRY(vkCmdDispatchBaseKHR),
			MAKE_VULKAN_ENTRY(vkTrimCommandPoolKHR),
			MAKE_VULKAN_ENTRY(vkEnumeratePhysicalDeviceGroupsKHR),
			MAKE_VULKAN_ENTRY(vkGetPhysicalDeviceExternalBufferPropertiesKHR),
			MAKE_VULKAN_ENTRY(vkGetMemoryFdKHR),
			MAKE_VULKAN_ENTRY(vkGetMemoryFdPropertiesKHR),
			MAKE_VULKAN_ENTRY(vkGetPhysicalDeviceExternalSemaphorePropertiesKHR),
			MAKE_VULKAN_ENTRY(vkImportSemaphoreFdKHR),
			MAKE_VULKAN_ENTRY(vkGetSemaphoreFdKHR),
			MAKE_VULKAN_ENTRY(vkCmdPushDescriptorSetKHR),
			MAKE_VULKAN_ENTRY(vkCmdPushDescriptorSetWithTemplateKHR),
			MAKE_VULKAN_ENTRY(vkCreateDescriptorUpdateTemplateKHR),
			MAKE_VULKAN_ENTRY(vkDestroyDescriptorUpdateTemplateKHR),
			MAKE_VULKAN_ENTRY(vkUpdateDescriptorSetWithTemplateKHR),
			MAKE_VULKAN_ENTRY(vkCreateRenderPass2KHR),
			MAKE_VULKAN_ENTRY(vkCmdBeginRenderPass2KHR),
			MAKE_VULKAN_ENTRY(vkCmdNextSubpass2KHR),
			MAKE_VULKAN_ENTRY(vkCmdEndRenderPass2KHR),
			MAKE_VULKAN_ENTRY(vkGetSwapchainStatusKHR),
			MAKE_VULKAN_ENTRY(vkGetPhysicalDeviceExternalFencePropertiesKHR),
			MAKE_VULKAN_ENTRY(vkImportFenceFdKHR),
			MAKE_VULKAN_ENTRY(vkGetFenceFdKHR),
			MAKE_VULKAN_ENTRY(vkGetPhysicalDeviceSurfaceCapabilities2KHR),
			MAKE_VULKAN_ENTRY(vkGetPhysicalDeviceSurfaceFormats2KHR),
			MAKE_VULKAN_ENTRY(vkGetPhysicalDeviceDisplayProperties2KHR),
			MAKE_VULKAN_ENTRY(vkGetPhysicalDeviceDisplayPlaneProperties2KHR),
			MAKE_VULKAN_ENTRY(vkGetDisplayModeProperties2KHR),
			MAKE_VULKAN_ENTRY(vkGetDisplayPlaneCapabilities2KHR),
			MAKE_VULKAN_ENTRY(vkGetImageMemoryRequirements2KHR),
			MAKE_VULKAN_ENTRY(vkGetBufferMemoryRequirements2KHR),
			MAKE_VULKAN_ENTRY(vkGetImageSparseMemoryRequirements2KHR),
			MAKE_VULKAN_ENTRY(vkCreateSamplerYcbcrConversionKHR),
			MAKE_VULKAN_ENTRY(vkDestroySamplerYcbcrConversionKHR),
			MAKE_VULKAN_ENTRY(vkBindBufferMemory2KHR),
			MAKE_VULKAN_ENTRY(vkBindImageMemory2KHR),
			MAKE_VULKAN_ENTRY(vkGetDescriptorSetLayoutSupportKHR),
			MAKE_VULKAN_ENTRY(vkCmdDrawIndirectCountKHR),
			MAKE_VULKAN_ENTRY(vkCmdDrawIndexedIndirectCountKHR),
			MAKE_VULKAN_ENTRY(vkCreateDebugReportCallbackEXT),
			MAKE_VULKAN_ENTRY(vkDestroyDebugReportCallbackEXT),
			MAKE_VULKAN_ENTRY(vkDebugReportMessageEXT),
			MAKE_VULKAN_ENTRY(vkDebugMarkerSetObjectTagEXT),
			MAKE_VULKAN_ENTRY(vkDebugMarkerSetObjectNameEXT),
			MAKE_VULKAN_ENTRY(vkCmdDebugMarkerBeginEXT),
			MAKE_VULKAN_ENTRY(vkCmdDebugMarkerEndEXT),
			MAKE_VULKAN_ENTRY(vkCmdDebugMarkerInsertEXT),
			MAKE_VULKAN_ENTRY(vkCmdDrawIndirectCountAMD),
			MAKE_VULKAN_ENTRY(vkCmdDrawIndexedIndirectCountAMD),
			MAKE_VULKAN_ENTRY(vkGetShaderInfoAMD),
			MAKE_VULKAN_ENTRY(vkGetPhysicalDeviceExternalImageFormatPropertiesNV),
			MAKE_VULKAN_ENTRY(vkCmdBeginConditionalRenderingEXT),
			MAKE_VULKAN_ENTRY(vkCmdEndConditionalRenderingEXT),
			MAKE_VULKAN_ENTRY(vkCmdProcessCommandsNVX),
			MAKE_VULKAN_ENTRY(vkCmdReserveSpaceForCommandsNVX),
			MAKE_VULKAN_ENTRY(vkCreateIndirectCommandsLayoutNVX),
			MAKE_VULKAN_ENTRY(vkDestroyIndirectCommandsLayoutNVX),
			MAKE_VULKAN_ENTRY(vkCreateObjectTableNVX),
			MAKE_VULKAN_ENTRY(vkDestroyObjectTableNVX),
			MAKE_VULKAN_ENTRY(vkRegisterObjectsNVX),
			MAKE_VULKAN_ENTRY(vkUnregisterObjectsNVX),
			MAKE_VULKAN_ENTRY(vkGetPhysicalDeviceGeneratedCommandsPropertiesNVX),
			MAKE_VULKAN_ENTRY(vkCmdSetViewportWScalingNV),
			MAKE_VULKAN_ENTRY(vkReleaseDisplayEXT),
			MAKE_VULKAN_ENTRY(vkGetPhysicalDeviceSurfaceCapabilities2EXT),
			MAKE_VULKAN_ENTRY(vkDisplayPowerControlEXT),
			MAKE_VULKAN_ENTRY(vkRegisterDeviceEventEXT),
			MAKE_VULKAN_ENTRY(vkRegisterDisplayEventEXT),
			MAKE_VULKAN_ENTRY(vkGetSwapchainCounterEXT),
			MAKE_VULKAN_ENTRY(vkGetRefreshCycleDurationGOOGLE),
			MAKE_VULKAN_ENTRY(vkGetPastPresentationTimingGOOGLE),
			MAKE_VULKAN_ENTRY(vkCmdSetDiscardRectangleEXT),
			MAKE_VULKAN_ENTRY(vkSetHdrMetadataEXT),
			MAKE_VULKAN_ENTRY(vkSetDebugUtilsObjectNameEXT),
			MAKE_VULKAN_ENTRY(vkSetDebugUtilsObjectTagEXT),
			MAKE_VULKAN_ENTRY(vkQueueBeginDebugUtilsLabelEXT),
			MAKE_VULKAN_ENTRY(vkQueueEndDebugUtilsLabelEXT),
			MAKE_VULKAN_ENTRY(vkQueueInsertDebugUtilsLabelEXT),
			MAKE_VULKAN_ENTRY(vkCmdBeginDebugUtilsLabelEXT),
			MAKE_VULKAN_ENTRY(vkCmdEndDebugUtilsLabelEXT),
			MAKE_VULKAN_ENTRY(vkCmdInsertDebugUtilsLabelEXT),
			MAKE_VULKAN_ENTRY(vkCreateDebugUtilsMessengerEXT),
			MAKE_VULKAN_ENTRY(vkDestroyDebugUtilsMessengerEXT),
			MAKE_VULKAN_ENTRY(vkSubmitDebugUtilsMessageEXT),
			MAKE_VULKAN_ENTRY(vkCmdSetSampleLocationsEXT),
			MAKE_VULKAN_ENTRY(vkGetPhysicalDeviceMultisamplePropertiesEXT),
			MAKE_VULKAN_ENTRY(vkCreateValidationCacheEXT),
			MAKE_VULKAN_ENTRY(vkDestroyValidationCacheEXT),
			MAKE_VULKAN_ENTRY(vkMergeValidationCachesEXT),
			MAKE_VULKAN_ENTRY(vkGetValidationCacheDataEXT),
			MAKE_VULKAN_ENTRY(vkGetMemoryHostPointerPropertiesEXT),
			MAKE_VULKAN_ENTRY(vkCmdWriteBufferMarkerAMD),
			MAKE_VULKAN_ENTRY(vkCmdSetCheckpointNV),
			MAKE_VULKAN_ENTRY(vkGetQueueCheckpointDataNV)
		};
		#undef MAKE_VULKAN_ENTRY

		auto pFunc = func_ptrs.find(std::string(pName));
		return (pFunc == func_ptrs.end()) ? nullptr : pFunc->second;
	}
}