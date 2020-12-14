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

#include "VkTimelineSemaphore.hpp"
#include "VkSemaphore.hpp"

#include "marl/blockingcall.h"
#include "marl/conditionvariable.h"

#include <vector>

namespace vk {

TimelineSemaphore::TimelineSemaphore(const VkSemaphoreCreateInfo *pCreateInfo, void *mem, const VkAllocationCallbacks *pAllocator)
    : Semaphore(VK_SEMAPHORE_TYPE_TIMELINE)
{
	SemaphoreCreateInfo info(pCreateInfo);
	ASSERT(info.semaphoreType == VK_SEMAPHORE_TYPE_TIMELINE);
	type = info.semaphoreType;
	shared = marl::Allocator::Default->make_shared<TimelineSemaphore::Shared>(marl::Allocator::Default, info.initialPayload);
}

TimelineSemaphore::TimelineSemaphore()
    : Semaphore(VK_SEMAPHORE_TYPE_TIMELINE)
{
	type = VK_SEMAPHORE_TYPE_TIMELINE;
	shared = marl::Allocator::Default->make_shared<TimelineSemaphore::Shared>(marl::Allocator::Default, 0);
}

size_t TimelineSemaphore::ComputeRequiredAllocationSize(const VkSemaphoreCreateInfo *pCreateInfo)
{
	return 0;
}

void TimelineSemaphore::destroy(const VkAllocationCallbacks *pAllocator)
{
}

void TimelineSemaphore::signal(uint64_t value)
{
	marl::lock lock(shared->mutex);
	if(shared->counter >= value)
	{
		return;
	}
	shared->counter = value;
	shared->cv.notify_all();
	for(auto dep : shared->deps)
	{
		dep->signal(shared->id, shared->counter);
	}
}

void TimelineSemaphore::wait(uint64_t value)
{
	marl::lock lock(shared->mutex);
	shared->cv.wait(lock, [&] { return shared->counter == value; });
}

uint64_t TimelineSemaphore::getCounterValue() const
{
	marl::lock lock(shared->mutex);
	return shared->counter;
}

std::atomic<int> TimelineSemaphore::Shared::nextId;

TimelineSemaphore::Shared::Shared(marl::Allocator *allocator, uint64_t initialState)
    : cv(allocator)
    , counter(initialState)
    , id(nextId++)
{
}

void TimelineSemaphore::Shared::signal(int parentId, uint64_t value)
{
	marl::lock lock(mutex);
	// Either we aren't waiting for a signal, or parentId is not something we're waiting for
	if(counter == 1 || waitMap.find(parentId) == waitMap.end())
	{
		return;
	}
	// Reject any signals that we aren't waiting on
	if(value != waitMap[parentId])
	{
		return;
	}
	// Stop waiting on all parents once we find a signal
	waitMap.clear();
	counter = 1;
	cv.notify_all();
	for(auto dep : deps)
	{
		dep->signal(id, counter);
	}
}

void TimelineSemaphore::addDependent(TimelineSemaphore &other, uint64_t waitValue)
{
	marl::lock lock(shared->mutex);
	marl::lock lock2(other.shared->mutex);
	shared->deps.push_back(other.shared);
	other.shared->waitMap[shared->id] = waitValue;
}

}  // namespace vk
