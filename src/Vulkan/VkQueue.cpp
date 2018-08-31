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

#include "VkQueue.hpp"
#include "VkCommandBuffer.hpp"
#include "VkFence.hpp"

namespace vk
{

Queue::Queue(uint32_t pFamilyIndex, float pPriority) : familyIndex(pFamilyIndex), priority(pPriority)
{
}

Queue::operator VkQueue()
{
	return reinterpret_cast<VkQueue>(this);
}

void* Queue::operator new(size_t count, const VkAllocationCallbacks* pAllocator)
{
	return vk::allocate(count, pAllocator, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
}

void Queue::operator delete(void* ptr, const VkAllocationCallbacks* pAllocator)
{
	// Does nothing, objects are deleted through the destroy function
	ASSERT(false);
}

void Queue::submit(uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence)
{
	for(uint32_t i = 0; i < submitCount; i++)
	{
		const VkSubmitInfo& submitInfo = pSubmits[i];
		if(submitInfo.waitSemaphoreCount || submitInfo.signalSemaphoreCount)
		{
			UNIMPLEMENTED();
		}

		for(uint32_t j = 0; j < submitInfo.commandBufferCount; j++)
		{
			Cast(submitInfo.pCommandBuffers[j])->submit();
		}
	}

	signal(fence);
}

void Queue::bindSparse(uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence)
{
	for(uint32_t i = 0; i < bindInfoCount; i++)
	{
		const VkBindSparseInfo& bindInfo = pBindInfo[i];
		if(bindInfo.waitSemaphoreCount || bindInfo.signalSemaphoreCount || bindInfo.signalSemaphoreCount ||
		   bindInfo.signalSemaphoreCount || bindInfo.signalSemaphoreCount)
		{
			UNIMPLEMENTED();
		}
	}

	signal(fence);
}

void Queue::signal(VkFence fence)
{
	// FIXME: signal the fence only once the work is completed
	if(fence != VK_NULL_HANDLE)
	{
		Cast(fence)->signal();
	}
}

void Queue::waitIdle()
{
	// noop
}

} // namespace vk