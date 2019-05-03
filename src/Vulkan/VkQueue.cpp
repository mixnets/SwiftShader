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

namespace vk
{

Queue::Queue(uint32_t pFamilyIndex, float pPriority) : context(), renderer(&context, sw::OpenGL, true),
	mutex(), newTaskAvailable(), taskExecuted(), queueThread(TaskLoop, this)
{
}

void Queue::destroy()
{
	{
		std::unique_lock<std::mutex> mutexLock(mutex);
		task.type = Task::KILL_THREAD;
		task.executed = false;
		mutexLock.unlock();
		newTaskAvailable.notify_all();
	}

	waitIdle();

	queueThread.join();
}

void Queue::submit(uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence)
{
	{
		std::unique_lock<std::mutex> mutexLock(mutex);
		task.submitCount = submitCount;
		task.pSubmits = pSubmits;
		task.fence = (fence != VK_NULL_HANDLE) ? vk::Cast(fence) : nullptr;
		task.executed = false;
		mutexLock.unlock();
		newTaskAvailable.notify_all();
	}

	waitIdle();
}

void Queue::TaskLoop(vk::Queue* queue)
{
	queue->taskLoop();
}

void Queue::submit()
{
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

	// FIXME (b/117835459): signal the fence only once the work is completed
	if(task.fence)
	{
		task.fence->signal();
	}

	task.executed = true;
}

void Queue::waitForTask()
{
	std::unique_lock<std::mutex> mutexLock(mutex);
	if(task.executed)
	{
		newTaskAvailable.wait(mutexLock, [this] { return (!task.executed); });
	}
}

void Queue::taskLoop()
{
	bool doLoop = true;
	while(doLoop)
	{
		waitForTask();

		std::unique_lock<std::mutex> mutexLock(mutex);
		switch(task.type)
		{
		case Task::KILL_THREAD:
			doLoop = false;
			break;
		case Task::SUBMIT_QUEUE:
			submit();
			break;
		default:
			UNIMPLEMENTED("task.type %d", static_cast<int>(task.type));
			break;
		}

		task.executed = true;
		mutexLock.unlock();
		taskExecuted.notify_all();
	}
}

void Queue::waitIdle()
{
	// Wait for task queue to be empty
	{
		std::unique_lock<std::mutex> mutexLock(mutex);
		if(!task.executed)
		{
			taskExecuted.wait(mutexLock, [this] { return task.executed; });
		}
	}

	// Wait for all draw operations to complete, if any
	renderer.synchronize();
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
