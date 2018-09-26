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

#include "VkBuffer.hpp"
#include "VkCommandBuffer.hpp"
#include "VkImage.hpp"
#include "VkQueue.hpp"
#include "VkFence.hpp"
#include "VkSemaphore.hpp"
#include "Device/Renderer.hpp"

namespace vk
{

Queue::Queue(uint32_t pFamilyIndex, float pPriority) : context(), renderer(&context, sw::OpenGL, true), familyIndex(pFamilyIndex), priority(pPriority)
{
}

void Queue::submit(uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence)
{
	for(uint32_t i = 0; i < submitCount; i++)
	{
		auto& submitInfo = pSubmits[i];
		for(uint32_t j = 0; j < submitInfo.waitSemaphoreCount; j++)
		{
			vk::Cast(submitInfo.pWaitSemaphores[j])->wait(submitInfo.pWaitDstStageMask[j]);
		}

		{
			CommandBuffer::ExecutionState executionState;
			executionState.renderer = &renderer;
			for(uint32_t j = 0; j < submitInfo.commandBufferCount; j++)
			{
				vk::Cast(submitInfo.pCommandBuffers[j])->submit(executionState);
			}
		}

		for(uint32_t j = 0; j < submitInfo.signalSemaphoreCount; j++)
		{
			vk::Cast(submitInfo.pSignalSemaphores[j])->signal();
		}
	}

	signal(fence);
}

void Queue::bindSparse(uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence)
{
	for(uint32_t i = 0; i < bindInfoCount; i++)
	{
		auto& bindInfo = pBindInfo[i];
		for(uint32_t j = 0; j < bindInfo.waitSemaphoreCount; j++)
		{
			vk::Cast(bindInfo.pWaitSemaphores[j])->wait();
		}

		for(uint32_t j = 0; j < bindInfo.bufferBindCount; j++)
		{
			auto& bufferBinds = bindInfo.pBufferBinds[j];
			auto buffer = vk::Cast(bufferBinds.buffer);
			for(uint32_t k = 0; k < bufferBinds.bindCount; k++)
			{
				auto& sparseMemoryBind = bufferBinds.pBinds[k];
				UNIMPLEMENTED();
			}
		}

		for(uint32_t j = 0; j < bindInfo.imageBindCount; j++)
		{
			auto& imageBinds = bindInfo.pImageBinds[j];
			auto image = vk::Cast(imageBinds.image);
			for(uint32_t k = 0; k < imageBinds.bindCount; k++)
			{
				auto& sparseImageMemoryBind = imageBinds.pBinds[k];
				UNIMPLEMENTED();
			}
		}

		for(uint32_t j = 0; j < bindInfo.imageOpaqueBindCount; j++)
		{
			auto& imageOpaqueBinds = bindInfo.pImageOpaqueBinds[j];
			auto image = vk::Cast(imageOpaqueBinds.image);
			for(uint32_t k = 0; k < imageOpaqueBinds.bindCount; k++)
			{
				auto& sparseMemoryBind = imageOpaqueBinds.pBinds[k];
				UNIMPLEMENTED();
			}
		}

		for(uint32_t i = 0; i < bindInfo.signalSemaphoreCount; i++)
		{
			vk::Cast(bindInfo.pSignalSemaphores[i])->signal();
		}
	}

	signal(fence);
}

void Queue::signal(VkFence fence)
{
	// FIXME (b\117835459): signal the fence only once the work is completed
	if(fence != VK_NULL_HANDLE)
	{
		vk::Cast(fence)->signal();
	}
}

void Queue::waitIdle()
{
	// equivalent to submitting a fence to a queue and waiting
	// with an infinite timeout for that fence to signal
}

} // namespace vk