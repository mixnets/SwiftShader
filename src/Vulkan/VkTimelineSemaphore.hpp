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

#include "marl/conditionvariable.h"
#include "marl/mutex.h"
#include "marl/tsa.h"

#include "System/Synchronization.hpp"

#include <chrono>
#include <vector>

#if VK_USE_PLATFORM_FUCHSIA
#	include <zircon/types.h>
#endif

namespace vk {

// TimelineSemaphore is a synchronization primitive that tracks a monotonically increasing
// uint64_t payload
class TimelineSemaphore
{
public:
	TimelineSemaphore(uint64_t initialValue, marl::Allocator *allocator = marl::Allocator::Default);
	TimelineSemaphore(marl::Allocator *allocator = marl::Allocator::Default);

	// Update the value of the payload and unblock all calls to wait that were waiting on that value.
	void signal(uint64_t value) const;

	// Block until this semaphore is signaled with the specified value.
	void wait(uint64_t value) const;

	// Block until the timeline is signaled with the proper payload, or the timeout has been
	// reached.
	// If the timeout was reached, then wait_until() return false.
	template<typename Clock, typename Duration>
	bool waitUntil(uint64_t value,
	               const std::chrono::time_point<Clock, Duration> timeout);

	uint64_t getPayloadValue() const;

	int getId() const;

	void addToWaitMap(int id, uint64_t waitValue);

private:
	struct Shared
	{
		Shared(marl::Allocator *allocator, uint64_t initialState);
		void signal(uint64_t value);
		void signal(int parentId, uint64_t value);
		void wait(uint64_t value);

		template<typename Clock, typename Duration>
		bool waitUntil(uint64_t value,
		               std::chrono::time_point<Clock, Duration> timeout);

		inline int getId();

		void addSharedDep(TimelineSemaphore other);

		marl::mutex mutex;
		marl::ConditionVariable cv;
		uint64_t payload;
		marl::containers::vector<std::shared_ptr<Shared>, 1> deps;
		std::map<int, uint64_t> waitMap;

	private:
		int id;
		static std::atomic<int> nextId;
	};

	std::shared_ptr<Shared> shared;

public:
	void addSharedDep(TimelineSemaphore other);

	template<typename PushBackFunc>
	void addSelfToDeps(PushBackFunc &&pushBackFunc);
};

template<typename Clock, typename Duration>
bool TimelineSemaphore::Shared::waitUntil(uint64_t value,
                                          std::chrono::time_point<Clock, Duration> timeout)
{
	marl::lock lock(mutex);
	if(!cv.wait_until(lock, timeout, [&] { return payload == value; }))
	{
		return false;
	}
	return true;
}

template<typename Clock, typename Duration>
bool TimelineSemaphore::waitUntil(uint64_t value,
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
