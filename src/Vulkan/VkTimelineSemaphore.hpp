// Copyright 2021 The SwiftShader Authors. All Rights Reserved.
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

#ifndef VK_TIMELINE_SEMAPHORE_HPP_
#define VK_TIMELINE_SEMAPHORE_HPP_

#include "VkConfig.hpp"
#include "VkObject.hpp"
#include "VkSemaphore.hpp"

#include "marl/conditionvariable.h"
#include "marl/mutex.h"
#include "marl/tsa.h"

#include "System/Synchronization.hpp"

#include <chrono>

namespace vk {

class TimelineSemaphore : public Semaphore, public Object<TimelineSemaphore, VkSemaphore>
{
public:
	TimelineSemaphore(const VkSemaphoreCreateInfo *pCreateInfo, void *mem, const VkAllocationCallbacks *pAllocator);
	TimelineSemaphore();

	static size_t ComputeRequiredAllocationSize(const VkSemaphoreCreateInfo *pCreateInfo);

	uint64_t getCounterValue();

	// Block until this semaphore is signaled with the specified value;
	void wait(uint64_t value);

	// Wait until a certain amount of time has passed
	template<class CLOCK, class DURATION>
	VkResult wait(uint64_t value, const std::chrono::time_point<CLOCK, DURATION> end_ns);

	// Set the payload to the specified value and signal all waiting threads.
	void signal(uint64_t value);

	int getTimelineId();

	uint64_t getCounterValue() const;

	int getId() const;

	void addSharedDep(TimelineSemaphore &other);
	void addToWaitMap(int id, uint64_t waitValue);

	template<typename PushBackFunc>
	void addSelfToDeps(PushBackFunc &&pushBackFunc);

private:
	struct Shared
	{
		Shared(marl::Allocator *allocator, uint64_t initialState);
		void signal(uint64_t value);
		void signal(int parentId, uint64_t value);
		void wait(uint64_t value);

		template<typename Clock, typename Duration>
		VkResult waitUntil(uint64_t value,
		                   std::chrono::time_point<Clock, Duration> timeout);

		inline int getId();

		void addSharedDep(TimelineSemaphore &other);

		marl::mutex mutex;
		marl::ConditionVariable cv;
		uint64_t counter;
		marl::containers::vector<std::shared_ptr<Shared>, 1> deps;
		std::map<int, uint64_t> waitMap;

	private:
		int id;
		static std::atomic<int> nextId;
	};
	std::shared_ptr<Shared> shared;
};
template<typename Clock, typename Duration>
VkResult TimelineSemaphore::Shared::waitUntil(uint64_t value,
                                              std::chrono::time_point<Clock, Duration> timeout)
{
	marl::lock lock(mutex);
	if(!cv.wait_until(lock, timeout, [&] { return counter == value; }))
	{
		return VK_TIMEOUT;
	}
	return VK_SUCCESS;
}

template<typename Clock, typename Duration>
VkResult TimelineSemaphore::wait(uint64_t value,
                                 const std::chrono::time_point<Clock, Duration> timeout)
{
	return shared->waitUntil(value, timeout);
}

template<typename PushBackFunc>
void TimelineSemaphore::addSelfToDeps(PushBackFunc &&pushbackFunc)
{
	pushbackFunc(shared);
}

}  // namespace vk

#endif  // VK_TIMELINE_SEMAPHORE_HPP_
