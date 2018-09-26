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
#include <algorithm>
#include <vector>

namespace vk
{

class CommandPool : public Object<CommandPool, VkCommandPool>
{
public:
	CommandPool(const VkCommandPoolCreateInfo* pCreateInfo, void* mem) :
		flags(pCreateInfo->flags), queueFamilyIndex(pCreateInfo->queueFamilyIndex)
	{
	}

	~CommandPool() = delete;

	void destroy(const VkAllocationCallbacks* pAllocator)
	{
		// Free command Buffers allocated in allocateCommandBuffers
		for(auto commandBuffer : commandBuffers)
		{
			vk::destroy(commandBuffer, DEVICE_MEMORY);
		}

		// Force commandBuffers to release all of its memory
		std::vector<VkCommandBuffer> tmp;
		commandBuffers.swap(tmp);
	}

	static size_t ComputeRequiredAllocationSize(const VkCommandPoolCreateInfo* pCreateInfo)
	{
		return 0;
	}

	VkResult allocateCommandBuffers(VkCommandBufferLevel level, uint32_t commandBufferCount, VkCommandBuffer* pCommandBuffers)
	{
		for(uint32_t i = 0; i < commandBufferCount; i++)
		{
			DispatchableCommandBuffer* commandBuffer = new (DEVICE_MEMORY) DispatchableCommandBuffer(level);
			if(commandBuffer)
			{
				pCommandBuffers[i] = *commandBuffer;
			}
			else
			{
				for(uint32_t j = 0; j < i; j++)
				{
					vk::destroy(pCommandBuffers[j], DEVICE_MEMORY);
				}
				for(uint32_t j = 0; j < i; j++)
				{
					pCommandBuffers[j] = VK_NULL_HANDLE;
				}
				return VK_ERROR_OUT_OF_DEVICE_MEMORY;
			}
		}

		// Insert successfully created command buffers
		commandBuffers.insert(commandBuffers.end(), pCommandBuffers, pCommandBuffers + commandBufferCount);

		return VK_SUCCESS;
	}

	void freeCommandBuffers(uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers)
	{
		for(uint32_t i = 0; i < commandBufferCount; ++i)
		{
			// Use erase-remove idiom to remove the element and resize the vector
			commandBuffers.erase(
				std::remove(commandBuffers.begin(), commandBuffers.end(), pCommandBuffers[i]),
				commandBuffers.end());

			vk::destroy(pCommandBuffers[i], DEVICE_MEMORY);
		}
	}

	void reset(VkCommandPoolResetFlags flags)
	{
		// Resetting a command pool recycles all of the resources from all
		// of the command buffers allocated from the command pool back to
		// the command pool. All command buffers that have been allocated
		// from the command pool are put in the initial state.
	}

	void trim()
	{
		// Release unused memory here
	}

private:
	VkCommandPoolCreateFlags     flags = 0;
	uint32_t                     queueFamilyIndex = 0;
	std::vector<VkCommandBuffer> commandBuffers;
};

static inline CommandPool* Cast(VkCommandPool object)
{
	return reinterpret_cast<CommandPool*>(object);
}

} // namespace vk

#endif // VK_COMMAND_POOL_HPP_
