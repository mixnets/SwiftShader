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

#include "VkCommandBuffer.hpp"
#include "VkImage.hpp"

namespace vk
{

CommandBuffer::CommandBuffer(VkCommandBufferLevel pLevel) : level(pLevel)
{
	pipelines[VK_PIPELINE_BIND_POINT_GRAPHICS] = VK_NULL_HANDLE;
	pipelines[VK_PIPELINE_BIND_POINT_COMPUTE] = VK_NULL_HANDLE;
}

CommandBuffer::operator VkCommandBuffer() { return reinterpret_cast<VkCommandBuffer>(this); }

void* CommandBuffer::operator new(size_t count, const VkAllocationCallbacks* pAllocator)
{
	return vk::allocate(count, pAllocator, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
}

void CommandBuffer::operator delete(void* ptr, const VkAllocationCallbacks* pAllocator)
{
	// Does nothing, objects are deleted through the destroy function
	ASSERT(false);
}

void CommandBuffer::destroy(const VkAllocationCallbacks* pAllocator)
{
}

void CommandBuffer::begin(VkCommandBufferUsageFlags flags, const VkCommandBufferInheritanceInfo* pInheritanceInfo)
{
}

void CommandBuffer::end()
{
}

void CommandBuffer::beginRenderPass(VkRenderPass renderPass, VkFramebuffer framebuffer, VkRect2D renderArea,
                                    uint32_t clearValueCount, const VkClearValue* pClearValues, VkSubpassContents contents)
{
}

void CommandBuffer::endRenderPass()
{
}

void CommandBuffer::pipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                    VkDependencyFlags dependencyFlags,
                                    uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                    uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                    uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers)
{
	// FIXME: memory barriers are currently ignored
}

void CommandBuffer::bindPipeline(VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline)
{
	pipelines[pipelineBindPoint] = pipeline;
}

void CommandBuffer::bindVertexBuffers(uint32_t firstBinding, uint32_t bindingCount,
                                      const VkBuffer* pBuffers, const VkDeviceSize* pOffsets)
{
	for(uint32_t i = firstBinding; i < (firstBinding + bindingCount); ++i)
	{
		vertexInputBindings[i].buffer = pBuffers[i];
		vertexInputBindings[i].offset = pOffsets[i];
	}
}

void CommandBuffer::draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
	// FIXME: hookup renderer here
}

void CommandBuffer::drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
{
	// FIXME: hookup renderer here
}

void CommandBuffer::drawIndirect(VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride)
{
	// FIXME: hookup renderer here
}

void CommandBuffer::drawIndexedIndirect(VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride)
{
	// FIXME: hookup renderer here
}

void CommandBuffer::copyImageToBuffer(VkImage srcImage, VkImageLayout srcImageLayout, VkBuffer dstBuffer,
                                      uint32_t regionCount, const VkBufferImageCopy* pRegions)
{
	Cast(srcImage)->copyTo(srcImageLayout, dstBuffer, regionCount, pRegions);
}

void CommandBuffer::submit()
{
	// Perform recorded work
}

} // namespace vk