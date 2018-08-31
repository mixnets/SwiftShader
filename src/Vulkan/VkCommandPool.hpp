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

#ifndef VK_COMMAND_POOL_HPP_
#define VK_COMMAND_POOL_HPP_

#include "VkCommandBuffer.hpp"

namespace vk
{

VkClass(CommandPool)
{
public:
	CommandPool(const VkCommandPoolCreateInfo* pCreateInfo) :
		flags(pCreateInfo->flags), queueFamilyIndex(pCreateInfo->queueFamilyIndex)
	{
	}	~CommandPool() = delete;
	void destroy(const VkAllocationCallbacks* pAllocator) override
	{
		// Note: Command Buffers allocated in allocateCommandBuffers may have to be freed here
	}

	void allocateCommandBuffers(VkCommandBufferLevel level, uint32_t commandBufferCount, VkCommandBuffer* pCommandBuffers)
	{
		for(uint32_t i = 0; i < commandBufferCount; ++i)
		{
			pCommandBuffers[i] = *(new (INTERNAL_MEMORY) CommandBuffer(level));
		}
	}

	void freeCommandBuffers(uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers)
	{
		for(uint32_t i = 0; i < commandBufferCount; ++i)
		{
			vk::destroy(pCommandBuffers[i], nullptr);
		}
	}

private:
	VkCommandPoolCreateFlags flags = 0;
	uint32_t                 queueFamilyIndex = 0;
};

} // namespace vk

#endif // VK_COMMAND_POOL_HPP_
