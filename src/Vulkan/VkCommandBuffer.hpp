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

#ifndef VK_COMMAND_BUFFER_HPP_
#define VK_COMMAND_BUFFER_HPP_

#include "VkConfig.h"
#include "VkDebug.hpp"
#include "VkMemory.h"
#include "vulkan/vk_icd.h"

namespace vk
{

class CommandBuffer final
{
	VK_LOADER_DATA loaderData = { ICD_LOADER_MAGIC };
public:
	CommandBuffer(VkCommandBufferLevel pLevel);

	~CommandBuffer() = delete;

	operator VkCommandBuffer();

	void* operator new(size_t count, const VkAllocationCallbacks* pAllocator);
	void operator delete(void* ptr, const VkAllocationCallbacks* pAllocator);

	void destroy(const VkAllocationCallbacks* pAllocator);
	void begin(VkCommandBufferUsageFlags flags, const VkCommandBufferInheritanceInfo* pInheritanceInfo);
	void end();
	void beginRenderPass(VkRenderPass renderPass, VkFramebuffer framebuffer, VkRect2D renderArea,
	                     uint32_t clearValueCount, const VkClearValue* pClearValues, VkSubpassContents contents);
	void endRenderPass();
	void pipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
	                     uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
	                     uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
	                     uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers);
	void bindPipeline(VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline);
	void bindVertexBuffers(uint32_t firstBinding, uint32_t bindingCount,
	                       const VkBuffer* pBuffers, const VkDeviceSize* pOffsets);

	void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
	void drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);
	void drawIndirect(VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride);
	void drawIndexedIndirect(VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride);
	void copyImageToBuffer(VkImage srcImage, VkImageLayout srcImageLayout, VkBuffer dstBuffer,
	                       uint32_t regionCount, const VkBufferImageCopy* pRegions);
	void submit();

private:
	VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	VkPipeline pipelines[VK_PIPELINE_BIND_POINT_RANGE_SIZE];

	struct VertexInputBindings
	{
		VkBuffer buffer;
		VkDeviceSize offset;
	};
	VertexInputBindings vertexInputBindings[MaxVertexInputBindings];
};

static CommandBuffer* Cast(VkCommandBuffer commandBuffer) { return reinterpret_cast<CommandBuffer*>(commandBuffer); }

} // namespace vk

#endif // VK_COMMAND_BUFFER_HPP_
