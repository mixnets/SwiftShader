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

#include "VkObject.hpp"

namespace vk {

VkClass(CommandBuffer) {
public:
	CommandBuffer(VkCommandBufferLevel pLevel) : level(pLevel)
	{
		pipelines[VK_PIPELINE_BIND_POINT_GRAPHICS] = VK_NULL_HANDLE;
		pipelines[VK_PIPELINE_BIND_POINT_COMPUTE] = VK_NULL_HANDLE;
	}

	static VkSystemAllocationScope getSystemAllocationScope() { return VK_SYSTEM_ALLOCATION_SCOPE_OBJECT; }

	void begin(VkCommandBufferUsageFlags flags, const VkCommandBufferInheritanceInfo* pInheritanceInfo)
	{
	}

	void end()
	{
	}

	void pipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
	                     uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
	                     uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
	                     uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers)
	{
		// FIXME: memory barriers are currently ignored
	}

	void beginRenderPass(VkRenderPass renderPass, VkFramebuffer framebuffer, VkRect2D renderArea,
	                     uint32_t clearValueCount, const VkClearValue* pClearValues, VkSubpassContents contents)
	{
	}

	void bindPipeline(VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline)
	{
		pipelines[pipelineBindPoint] = pipeline;
	}

private:
	VkCommandBufferLevel level;
	VkPipeline pipelines[VK_PIPELINE_BIND_POINT_RANGE_SIZE];
};

} // namespace vk

#endif // VK_COMMAND_BUFFER_HPP_
