#ifndef LIB_VULKAN_HPP
#define LIB_VULKAN_HPP

#include <Vulkan/vulkan.h>

#include "../../Common/SharedLibrary.hpp"

namespace sw
{
	class FrameBuffer;
	enum Format : unsigned char;
}

namespace vulkan
{
	class Display;
	class Context;
	class Image;
	class Config;
}

class LibVulkanExports
{
public:
	LibVulkanExports();
	VkResult (*vkCreateInstance)(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,VkInstance* pInstance);
	void (*vkDestroyInstance)(VkInstance instance, const VkAllocationCallbacks *pAllocator);
	VkResult (*vkEnumeratePhysicalDevices)(VkInstance instance, uint32_t *pPhysicalDeviceCount,VkPhysicalDevice *pPhysicalDevices);
	void (*vkGetPhysicalDeviceFeatures)(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures* pFeatures);
	void (*vkGetPhysicalDeviceFormatProperties)(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties* pFormatProperties);
	VkResult (*vkGetPhysicalDeviceImageFormatProperties)(VkPhysicalDevice physicalDevice, VkFormat format,VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags, VkImageFormatProperties* pImageFormatProperties);
	void (*vkGetPhysicalDeviceProperties)(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties* pProperties);
	void (*vkGetPhysicalDeviceQueueFamilyProperties)(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties* pQueueFamilyProperties);
	void (*vkGetPhysicalDeviceMemoryProperties)(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties* pMemoryProperties);
	PFN_vkVoidFunction (*vkGetInstanceProcAddr)(VkInstance instance, const char* pName);
	PFN_vkVoidFunction (*vkGetDeviceProcAddr)(VkDevice device, const char* pName);
	VkResult (*vkCreateDevice)(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice);
	void (*vkDestroyDevice)(VkDevice device, const VkAllocationCallbacks* pAllocator);
	VkResult (*vkEnumerateInstanceExtensionProperties)(const char* pLayerName, uint32_t* pPropertyCount, VkExtensionProperties* pProperties);
	VkResult (*vkEnumerateDeviceExtensionProperties)(VkPhysicalDevice physicalDevice, const char* pLayerName, uint32_t* pPropertyCount, VkExtensionProperties* pProperties);
	VkResult (*vkEnumerateInstanceLayerProperties)(uint32_t* pPropertyCount, VkLayerProperties* pProperties);
	VkResult (*vkEnumerateDeviceLayerProperties)(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkLayerProperties* pProperties);
	void (*vkGetDeviceQueue)(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue);
	VkResult (*vkQueueSubmit)(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence);
	VkResult (*vkQueueWaitIdle)(VkQueue queue);
	VkResult (*vkDeviceWaitIdle)(VkDevice device);
	VkResult (*vkAllocateMemory)(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo, const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory);
	void (*vkFreeMemory)(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator);
	VkResult (*vkMapMemory)(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** ppData);
	void (*vkUnmapMemory)(VkDevice device, VkDeviceMemory memory);
	VkResult (*vkFlushMappedMemoryRanges)(VkDevice device, uint32_t memoryRangeCount, const VkMappedMemoryRange* pMemoryRanges);
	VkResult (*vkInvalidateMappedMemoryRanges)(VkDevice device, uint32_t memoryRangeCount, const VkMappedMemoryRange* pMemoryRanges);
	void (*vkGetDeviceMemoryCommitment)(VkDevice device, VkDeviceMemory memory,	VkDeviceSize* pCommittedMemoryInBytes);
	VkResult (*vkBindBufferMemory)(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset);
	VkResult (*vkBindImageMemory)(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset);
	void (*vkGetBufferMemoryRequirements)(VkDevice device, VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements);
	void (*vkGetImageMemoryRequirements)(VkDevice device, VkImage image, VkMemoryRequirements* pMemoryRequirements);
	void (*vkGetImageSparseMemoryRequirements)(VkDevice device, VkImage image, uint32_t* pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements* pSparseMemoryRequirements);
	void (*vkGetPhysicalDeviceSparseImageFormatProperties)(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkSampleCountFlagBits samples, VkImageUsageFlags usage, VkImageTiling tiling, uint32_t* pPropertyCount, VkSparseImageFormatProperties* pProperties);
	VkResult (*vkQueueBindSparse)(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence);
	VkResult (*vkCreateFence)(VkDevice device, const VkFenceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFence* pFence);
	void (*vkDestroyFence)(VkDevice device, VkFence fence, const VkAllocationCallbacks* pAllocator);
	VkResult (*vkResetFences)(VkDevice device, uint32_t fenceCount, const VkFence* pFences);
	VkResult (*vkGetFenceStatus)(VkDevice device, VkFence fence);
	VkResult (*vkWaitForFences)(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll, uint64_t timeout);
	VkResult (*vkCreateSemaphore)(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore);
	void (*vkDestroySemaphore)(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator);
	VkResult (*vkCreateEvent)(VkDevice device, const VkEventCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkEvent* pEvent);
	void (*vkDestroyEvent)(VkDevice device, VkEvent vkevent, const VkAllocationCallbacks* pAllocator);
	VkResult (*vkGetEventStatus)(VkDevice device, VkEvent vkevent);
	VkResult (*vkSetEvent)(VkDevice device, VkEvent vkevent);
	VkResult (*vkResetEvent)(VkDevice device, VkEvent vkevent);
	VkResult (*vkCreateQueryPool)(VkDevice device, const VkQueryPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool);
	void (*vkDestroyQueryPool)(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator);
	VkResult (*vkGetQueryPoolResults)(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, size_t dataSize, void* pData, VkDeviceSize stride, VkQueryResultFlags flags);
	VkResult (*vkCreateBuffer)(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer);
	void (*vkDestroyBuffer)(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator);
	VkResult (*vkCreateBufferView)(VkDevice device, const VkBufferViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBufferView* pView);
	void (*vkDestroyBufferView)(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks* pAllocator);
	VkResult (*vkCreateImage)(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImage* pImage);
	void (*vkDestroyImage)(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator);
	void (*vkGetImageSubresourceLayout)(VkDevice device, VkImage image, const VkImageSubresource* pSubresource, VkSubresourceLayout* pLayout);
	VkResult (*vkCreateImageView)(VkDevice device, const VkImageViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImageView* pView);
	void (*vkDestroyImageView)(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator);
	VkResult (*vkCreateShaderModule)(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule);
	void (*vkDestroyShaderModule)(VkDevice device, VkShaderModule shaderModule, const VkAllocationCallbacks* pAllocator);
	VkResult (*vkCreatePipelineCache)(VkDevice device, const VkPipelineCacheCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache);
	void (*vkDestroyPipelineCache)(VkDevice device, VkPipelineCache pipelineCache, const VkAllocationCallbacks* pAllocator);
	VkResult (*vkGetPipelineCacheData)(VkDevice device, VkPipelineCache pipelineCache, size_t* pDataSize, void* pData);
	VkResult (*vkMergePipelineCaches)(VkDevice device, VkPipelineCache dstCache, uint32_t srcCacheCount, const VkPipelineCache* pSrcCaches);
	VkResult (*vkCreateGraphicsPipelines)(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkGraphicsPipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines);
	VkResult (*vkCreateComputePipelines)(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkComputePipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines);
	void (*vkDestroyPipeline)(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator);
	VkResult (*vkCreatePipelineLayout)(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout);
	void (*vkDestroyPipelineLayout)(VkDevice device, VkPipelineLayout pipelineLayout, const VkAllocationCallbacks* pAllocator);
	VkResult (*vkCreateSampler)(VkDevice device, const VkSamplerCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSampler* pSampler);
	void (*vkDestroySampler)(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator);
	VkResult (*vkCreateDescriptorSetLayout)(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorSetLayout* pSetLayout);
	void (*vkDestroyDescriptorSetLayout)(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, const VkAllocationCallbacks* pAllocator);
	VkResult (*vkCreateDescriptorPool)(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool);
	void (*vkDestroyDescriptorPool)(VkDevice device, VkDescriptorPool descriptorPool, const VkAllocationCallbacks* pAllocator);
	VkResult (*vkResetDescriptorPool)(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags flags);
	VkResult (*vkAllocateDescriptorSets)(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo, VkDescriptorSet* pDescriptorSets);
	VkResult (*vkFreeDescriptorSets)(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets);
	void (*vkUpdateDescriptorSets)(VkDevice device, uint32_t descriptorWriteCount, const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount, const VkCopyDescriptorSet* pDescriptorCopies);
	VkResult (*vkCreateFramebuffer)(VkDevice device,const VkFramebufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer);
	void (*vkDestroyFramebuffer)(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks* pAllocator);
	VkResult(*vkCreateRenderPass)(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass);
	void (*vkDestroyRenderPass)(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator);
	void (*vkGetRenderAreaGranularity)(VkDevice device, VkRenderPass renderPass, VkExtent2D* pGranularity);
	VkResult (*vkCreateCommandPool)(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool);
	void (*vkDestroyCommandPool)(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator);
	VkResult (*vkResetCommandPool)(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags);
	VkResult (*vkAllocateCommandBuffers)(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo, VkCommandBuffer* pCommandBuffers);
	void (*vkFreeCommandBuffers)(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers);
	VkResult (*vkBeginCommandBuffer)(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo);
	VkResult (*vkEndCommandBuffer)(VkCommandBuffer commandBuffer);
	VkResult (*vkResetCommandBuffer)(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags);
	void (*vkCmdBindPipeline)(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline);
	void (*vkCmdSetViewport)(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkViewport* pViewports);
	void (*vkCmdSetScissor)(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount, const VkRect2D* pScissors);
	void (*vkCmdSetLineWidth)(VkCommandBuffer commandBuffer, float lineWidth);
	void (*vkCmdSetDepthBias)(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor);
	void (*vkCmdSetBlendConstants)(VkCommandBuffer commandBuffer, const float blendConstants[4]);
	void (*vkCmdSetDepthBounds)(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds);
	void (*vkCmdSetStencilCompareMask)(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t compareMask);
	void (*vkCmdSetStencilWriteMask)(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t writeMask);
	void (*vkCmdSetStencilReference)(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t reference);
	void (*vkCmdBindDescriptorSets)(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets);
	void (*vkCmdBindIndexBuffer)(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType);
	void (*vkCmdBindVertexBuffers)(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets);
	void (*vkCmdDraw)(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
	void (*vkCmdDrawIndexed)(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);
	void (*vkCmdDrawIndirect)(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride);
	void (*vkCmdDrawIndexedIndirect)(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride);
	void (*vkCmdDispatch)(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
	void (*vkCmdDispatchIndirect)(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset);
	void (*vkCmdCopyBuffer)(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy* pRegions);
	void (*vkCmdCopyImage)(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy* pRegions);
	void (*vkCmdBlitImage)(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit* pRegions, VkFilter filter);
	void (*vkCmdCopyBufferToImage)(VkCommandBuffer commandBuffer, VkBuffer srcBuffer,	VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy* pRegions);
	void (*vkCmdCopyImageToBuffer)(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions);
	void (*vkCmdUpdateBuffer)(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize dataSize, const void* pData);
	void (*vkCmdFillBuffer)(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size, uint32_t data);
	void (*vkCmdClearColorImage)(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearColorValue* pColor, uint32_t rangeCount, const VkImageSubresourceRange* pRanges);
	void (*vkCmdClearDepthStencilImage)(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount, const VkImageSubresourceRange* pRanges);
	void (*vkCmdClearAttachments)(VkCommandBuffer commandBuffer, uint32_t attachmentCount, const VkClearAttachment* pAttachments, uint32_t rectCount, const VkClearRect* pRects);
	void (*vkCmdResolveImage)(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageResolve* pRegions);
	void (*vkCmdSetEvent)(VkCommandBuffer commandBuffer, VkEvent vkevent, VkPipelineStageFlags stageMask);
	void (*vkCmdResetEvent)(VkCommandBuffer commandBuffer, VkEvent vkevent, VkPipelineStageFlags stageMask);
	void (*vkCmdWaitEvents)(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers);
	void (*vkCmdPipelineBarrier)(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers);
	void (*vkCmdBeginQuery)(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags);
	void (*vkCmdEndQuery)(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query);
	void (*vkCmdResetQueryPool)(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount);
	void (*vkCmdWriteTimestamp)(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkQueryPool queryPool, uint32_t query);
	void (*vkCmdCopyQueryPoolResults)(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride, VkQueryResultFlags flags);
	void (*vkCmdPushConstants)(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* pValues);
	void (*vkCmdBeginRenderPass)(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin, VkSubpassContents contents);
	void (*vkCmdNextSubpass)(VkCommandBuffer commandBuffer, VkSubpassContents contents);
	void (*vkCmdEndRenderPass)(VkCommandBuffer commandBuffer);
	void (*vkCmdExecuteCommands)(VkCommandBuffer commandBuffer, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers);
	void (*vkDestroySurfaceKHR)(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks* pAllocator);
	VkResult (*vkGetPhysicalDeviceSurfaceSupportKHR)(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, VkSurfaceKHR surface, VkBool32* pSupported);
	VkResult (*vkGetPhysicalDeviceSurfaceCapabilitiesKHR)(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR* pSurfaceCapabilities);
	VkResult (*vkGetPhysicalDeviceSurfaceFormatsKHR)(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats);
	VkResult (*vkGetPhysicalDeviceSurfacePresentModesKHR)(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes);
	VkResult (*vkCreateSwapchainKHR)(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain);
	void (*vkDestroySwapchainKHR)(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator);
	VkResult (*vkGetSwapchainImagesKHR)(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount, VkImage* pSwapchainImages);
	VkResult (*vkAcquireNextImageKHR)(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex);
	VkResult (*vkQueuePresentKHR)(VkQueue queue, const VkPresentInfoKHR* pPresentInfo);
	VkResult (*vkGetPhysicalDeviceDisplayPropertiesKHR)(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkDisplayPropertiesKHR* pProperties);
	VkResult (*vkGetPhysicalDeviceDisplayPlanePropertiesKHR)(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkDisplayPlanePropertiesKHR* pProperties);
	VkResult (*vkGetDisplayPlaneSupportedDisplaysKHR)(VkPhysicalDevice physicalDevice, uint32_t planeIndex, uint32_t* pDisplayCount, VkDisplayKHR* pDisplays);
	VkResult (*vkGetDisplayModePropertiesKHR)(VkPhysicalDevice physicalDevice, VkDisplayKHR display, uint32_t* pPropertyCount, VkDisplayModePropertiesKHR* pProperties);
	VkResult (*vkCreateDisplayModeKHR)(VkPhysicalDevice physicalDevice, VkDisplayKHR display, const VkDisplayModeCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDisplayModeKHR* pMode);
	VkResult (*vkGetDisplayPlaneCapabilitiesKHR)(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode, uint32_t planeIndex, VkDisplayPlaneCapabilitiesKHR* pCapabilities);
	VkResult (*vkCreateDisplayPlaneSurfaceKHR)(VkInstance instance, const VkDisplaySurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
	VkResult (*vkCreateSharedSwapchainsKHR)(VkDevice device, uint32_t swapchainCount, const VkSwapchainCreateInfoKHR* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchains);
	VkResult (*vkCreateWin32SurfaceKHR)(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
	void (*vkTrimCommandPoolKHR)(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlagsKHR flags);
	void (*vkCmdPushDescriptorSetKHR)(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount, const VkWriteDescriptorSet* pDescriptorWrites);
	VkResult (*vkCreateDescriptorUpdateTemplateKHR)(VkDevice device, const VkDescriptorUpdateTemplateCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorUpdateTemplateKHR* pDescriptorUpdateTemplate);
	void (*vkDestroyDescriptorUpdateTemplateKHR)(VkDevice device, VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate, const VkAllocationCallbacks* pAllocator);
	void (*vkUpdateDescriptorSetWithTemplateKHR)(VkDevice device, VkDescriptorSet descriptorSet, VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate, const void* pData);
	void (*vkCmdPushDescriptorSetWithTemplateKHR)(VkCommandBuffer commandBuffer, VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate, VkPipelineLayout layout, uint32_t set, const void* pData);
	VkResult(*vkGetSwapchainStatusKHR)(VkDevice device, VkSwapchainKHR swapchain);
	VkResult (*vkCreateDebugReportCallbackEXT)(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback);
	void (*vkDestroyDebugReportCallbackEXT)(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator);
	void (*vkDebugReportMessageEXT)(VkInstance instance, VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage);
	VkResult (*vkDebugMarkerSetObjectTagEXT)(VkDevice device, VkDebugMarkerObjectTagInfoEXT* pTagInfo);
	VkResult (*vkDebugMarkerSetObjectNameEXT)(VkDevice device, VkDebugMarkerObjectNameInfoEXT* pNameInfo);
	void  (*vkCmdDebugMarkerBeginEXT)(VkCommandBuffer commandBuffer, VkDebugMarkerMarkerInfoEXT* pMarkerInfo);
	void (*vkCmdDebugMarkerEndEXT)(VkCommandBuffer commandBuffer);
	void (*vkCmdDebugMarkerInsertEXT)(VkCommandBuffer commandBuffer, VkDebugMarkerMarkerInfoEXT* pMarkerInfo);
	VkResult (*vkReleaseDisplayEXT)(VkPhysicalDevice physicalDevice, VkDisplayKHR display);
	VkResult (*vkDisplayPowerControlEXT)(VkDevice device, VkDisplayKHR displays, const VkDisplayPowerInfoEXT* pDisplayPowerInfo);
	VkResult (*vkRegisterDeviceEventEXT)(VkDevice device, const VkDeviceEventInfoEXT* pDeviceEventInfo, const VkAllocationCallbacks* pAllocator, VkFence* pFence);
	VkResult (*vkRegisterDisplayEventEXT)(VkDevice device, VkDisplayKHR display, const VkDisplayEventInfoEXT* pDisplayEventInfo, const VkAllocationCallbacks* pAllocator, VkFence* pFence);
	VkResult (*vkGetSwapchainCounterEXT)(VkDevice device, VkSwapchainKHR swapchain, VkSurfaceCounterFlagBitsEXT counter, uint64_t* pCounterValue);

};

class LibVulkan
{
public:
	LibVulkan() : libVulkan(nullptr), libVulkanExports(nullptr) {}

	~LibVulkan()
	{
		freeLibrary(libVulkan);
	}

	LibVulkanExports *operator->()
	{
		return loadExports();
	}
private:
	LibVulkanExports *loadExports()
	{
		if(!libVulkan)
		{
			#if defined(_WIN32)
				#if defined(__LP64__)
					const char *libVulkan_lib[] = {"libVulkan.dll", "lib64Vulkan_translator.dll"};
				#else
					const char *libVulkan_lib[] = {"libVulkan.dll", "libVulkan_translator.dll"};
				#endif
			#else
				#error "libVulkan::loadExports unimplemented for this platform"
			#endif

			libVulkan = loadLibrary(libVulkan_lib, "libVulkan_swiftshader");

			if(libVulkan)
			{
				auto libVulkan_swiftshader = (LibVulkanExports *(*)())getProcAddress(libVulkan, "libVulkan_swiftshader");
				libVulkanExports = libVulkan_swiftshader();
			}
		}

		return libVulkanExports;
	}
	void *libVulkan;
	LibVulkanExports *libVulkanExports;
};
#endif
