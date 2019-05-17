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

#ifndef VK_FENCE_HPP_
#define VK_FENCE_HPP_

#include "VkObject.hpp"
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>

namespace vk
{

using time_point = std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>;

// TaskEvents is an interface for notifying when tasks begin and end.
// Tasks can be nested and/or overlapping.
// TaskEvents is used for task queue synchronization.
class TaskEvents
{
public:
	// start() is called before a task begins.
	virtual void start() = 0;
	// finish() is called after a task ends. finish() must only be called after
	// a corresponding call to start().
	virtual void finish() = 0;
	// complete() is a helper for calling start() followed by finish().
	inline void complete() { start(); finish(); }
};

// WaitGroup is a synchronization primitive that allows you to wait for
// collection of asynchronous tasks to finish executing.
// Call add() before each task begins, and then call done() when after each task
// is finished.
// At the same time, wait() and waitUntil() can be used to block until all tasks
// have finished.
class WaitGroup : public TaskEvents
{
public:
	// add() begins a new task.
	void add()
	{
		std::unique_lock<std::mutex> lock(mutex);
		++count;
	}

	// done() is called when a task of the WaitGroup has been completed.
	// Returns true if there are no more tasks currently running in the
	// WaitGroup.
	bool done()
	{
		std::unique_lock<std::mutex> lock(mutex);
		ASSERT(count > 0);
		--count;
		if(count == 0)
		{
			condition.notify_all();
		}
		return count == 0;
	}

	// wait() blocks until all the tasks have been finished.
	void wait()
	{
		std::unique_lock<std::mutex> lock(mutex);
		if (count > 0)
		{
			condition.wait(lock, [this] { return count == 0; });
		}
		return;
	}

	// waitUntil() blocks until all the tasks have been finished or the timeout
	// has been reached, returning true if all tasks have been completed, or
	// false if the timeout has been reached.
	bool waitUntil(const time_point& timeout_ns)
	{
		std::unique_lock<std::mutex> lock(mutex);
		return condition.wait_until(lock, timeout_ns, [this] { return count == 0; });
	}

	// TaskEvents compliance
	void start() override { add(); }
	void finish() override { done(); }

private:
	int32_t count = 0; // guarded by mutex
	std::mutex mutex;
	std::condition_variable condition;
};

class Fence : public Object<Fence, VkFence>, public TaskEvents
{
public:
	Fence() : signaled(false) {}

	Fence(const VkFenceCreateInfo* pCreateInfo, void* mem) :
		signaled((pCreateInfo->flags & VK_FENCE_CREATE_SIGNALED_BIT) != 0) {}

	static size_t ComputeRequiredAllocationSize(const VkFenceCreateInfo* pCreateInfo)
	{
		return 0;
	}

	void reset()
	{
		wg.wait();
		signaled = false;
	}

	VkResult getStatus()
	{
		return signaled ? VK_SUCCESS : VK_NOT_READY;
	}

	VkResult wait()
	{
		if (!signaled)
		{
			wg.wait();
			signaled = true;
		}
		return VK_SUCCESS;
	}

	VkResult waitUntil(const time_point& timeout_ns)
	{
		if (!signaled)
		{
			if (!wg.waitUntil(timeout_ns))
			{
				return VK_TIMEOUT;
			}
			signaled = true;
		}
		return VK_SUCCESS;
	}

	// TaskEvents compliance
	void start() override
	{
		ASSERT(!signaled);
		wg.add();
	}

	void finish() override
	{
		ASSERT(!signaled);
		if (wg.done())
		{
			signaled = true;
		}
	}

private:
	Fence(const Fence&) = delete;
	~Fence() = delete;
	Fence& operator = (const Fence&) = delete;

	WaitGroup wg;
	std::atomic<bool> signaled;
};

static inline Fence* Cast(VkFence object)
{
	return reinterpret_cast<Fence*>(object.get());
}

} // namespace vk

#endif // VK_FENCE_HPP_
