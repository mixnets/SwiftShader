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
#include "VkFence.hpp"
#include "VkQueue.hpp"
#include "VkSemaphore.hpp"
#include "WSI/VkSwapchainKHR.hpp"

#include <cstring>

namespace
{

VkSubmitInfo* DeepCopySubmitInfo(uint32_t submitCount, const VkSubmitInfo* pSubmits)
{
	size_t submitSize = sizeof(VkSubmitInfo) * submitCount;
	size_t totalSize = submitSize;
	for(uint32_t i = 0; i < submitCount; i++)
	{
		totalSize += pSubmits[i].waitSemaphoreCount * (sizeof(VkSemaphore) + sizeof(VkPipelineStageFlags));
		totalSize += pSubmits[i].commandBufferCount * sizeof(VkCommandBuffer);
		totalSize += pSubmits[i].signalSemaphoreCount * sizeof(VkSemaphore);
	}

	uint8_t* mem = static_cast<uint8_t*>(
		vk::allocate(totalSize, vk::REQUIRED_MEMORY_ALIGNMENT, vk::DEVICE_MEMORY, vk::Fence::GetAllocationScope()));

	VkSubmitInfo* submits = new (mem) VkSubmitInfo[submitCount];
	memcpy(submits, pSubmits, submitSize);
	mem += submitSize;

	for(uint32_t i = 0; i < submitCount; i++)
	{
		size_t size = pSubmits[i].waitSemaphoreCount * sizeof(VkSemaphore);
		submits[i].pWaitSemaphores = new (mem) VkSemaphore[pSubmits[i].waitSemaphoreCount];
		memcpy(const_cast<VkSemaphore*>(submits[i].pWaitSemaphores), pSubmits[i].pWaitSemaphores, size);
		mem += size;

		size = pSubmits[i].waitSemaphoreCount * sizeof(VkPipelineStageFlags);
		submits[i].pWaitDstStageMask = new (mem) VkPipelineStageFlags[pSubmits[i].waitSemaphoreCount];
		memcpy(const_cast<VkPipelineStageFlags*>(submits[i].pWaitDstStageMask), pSubmits[i].pWaitDstStageMask, size);
		mem += size;

		size = pSubmits[i].signalSemaphoreCount * sizeof(VkSemaphore);
		submits[i].pSignalSemaphores = new (mem) VkSemaphore[pSubmits[i].signalSemaphoreCount];
		memcpy(const_cast<VkSemaphore*>(submits[i].pSignalSemaphores), pSubmits[i].pSignalSemaphores, size);
		mem += size;

		size = pSubmits[i].commandBufferCount * sizeof(VkCommandBuffer);
		submits[i].pCommandBuffers = new (mem) VkCommandBuffer[pSubmits[i].commandBufferCount];
		memcpy(const_cast<VkCommandBuffer*>(submits[i].pCommandBuffers), pSubmits[i].pCommandBuffers, size);
		mem += size;
	}

	return submits;
}

}

namespace vk
{

Queue::Queue(uint32_t pFamilyIndex, float pPriority) : context(), renderer(&context, sw::OpenGL, true),
	mutex(), newTaskAvailable(), emptySubmitQueue(), queueThread(TaskLoop, this)
{
}

void Queue::addTask(const Task& task)
{
	std::unique_lock<std::mutex> mutexLock(mutex);
	tasks.push(task);
	mutexLock.unlock();
	newTaskAvailable.notify_one();
}

void Queue::destroy()
{
	addTask({ 0, nullptr, nullptr, Task::KILL_THREAD });

	waitIdle();

	queueThread.join();
}

VkResult Queue::submit(uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence)
{
	addTask(
	{
		submitCount,
		DeepCopySubmitInfo(submitCount, pSubmits),
		(fence != VK_NULL_HANDLE) ? vk::Cast(fence) : nullptr,
		Task::SUBMIT_QUEUE
	});

	return VK_SUCCESS;
}

void Queue::TaskLoop(vk::Queue* queue)
{
	queue->taskLoop();
}

Queue::Task Queue::getTask()
{
	std::unique_lock<std::mutex> mutexLock(mutex);
	if(tasks.empty())
	{
		newTaskAvailable.wait(mutexLock, [this] { return (!tasks.empty()); });
	}
	Task task = tasks.front();
	tasks.pop();
	return task;
}

void Queue::purgeQueue()
{
	// The queue should already be empty at this point, make sure it is
	{
		std::unique_lock<std::mutex> mutexLock(mutex);
		if(!tasks.empty())
		{
			UNIMPLEMENTED("tasks queue not empty"); // This should never happen

			// Clear tasks queue
			std::queue<Task> empty;
			std::swap(tasks, empty);
		}
	}

	emptySubmitQueue.notify_one();
}

void Queue::submit(const Task& task)
{
	if(task.fence)
	{
		task.fence->ref();
	}

	for(uint32_t i = 0; i < task.submitCount; i++)
	{
		auto& submitInfo = task.pSubmits[i];
		for(uint32_t j = 0; j < submitInfo.waitSemaphoreCount; j++)
		{
			vk::Cast(submitInfo.pWaitSemaphores[j])->wait(submitInfo.pWaitDstStageMask[j]);
		}

		{
			CommandBuffer::ExecutionState executionState;
			executionState.renderer = &renderer;
			executionState.fence = task.fence;
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

	if(task.fence)
	{
		task.fence->unref();
	}
}

void Queue::taskLoop()
{
	while(true)
	{
		Task task = getTask();

		switch(task.type)
		{
		case Task::KILL_THREAD:
			purgeQueue();
			return;
		case Task::SUBMIT_QUEUE:
			submit(task);
			break;
		default:
			UNIMPLEMENTED("task.type %d", static_cast<int>(task.type));
			break;
		}

		notifyIfQueueIsEmpty();
	}
}

void Queue::notifyIfQueueIsEmpty()
{
	std::unique_lock<std::mutex> mutexLock(mutex);
	if(tasks.empty())
	{
		mutexLock.unlock();
		emptySubmitQueue.notify_one();
	}
}

void Queue::waitForEmptyQueue()
{
	std::unique_lock<std::mutex> mutexLock(mutex);
	if(!tasks.empty())
	{
		emptySubmitQueue.wait(mutexLock, [this] { return (tasks.empty()); });
	}
}

VkResult Queue::waitIdle()
{
	waitForEmptyQueue();

	renderer.synchronize();

	return VK_SUCCESS;
}

#ifndef __ANDROID__
void Queue::present(const VkPresentInfoKHR* presentInfo)
{
	for(uint32_t i = 0; i < presentInfo->waitSemaphoreCount; i++)
	{
		vk::Cast(presentInfo->pWaitSemaphores[i])->wait();
	}

	for(uint32_t i = 0; i < presentInfo->swapchainCount; i++)
	{
		vk::Cast(presentInfo->pSwapchains[i])->present(presentInfo->pImageIndices[i]);
	}
}
#endif

} // namespace vk
