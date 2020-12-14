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

#include "marl/blockingcall.h"
#include "marl/conditionvariable.h"

#include <vector>

namespace vk {

std::atomic<int> TimelineSemaphore::Shared::nextId;

TimelineSemaphore::Shared::Shared(marl::Allocator *allocator, uint64_t initialState)
    : cv(allocator)
    , payload(initialState)
{
	id = TimelineSemaphore::Shared::nextId++;
}

int TimelineSemaphore::Shared::getId()
{
	return id;
}

void TimelineSemaphore::Shared::signal(uint64_t value)
{
	marl::lock lock(mutex);
	if(payload >= value)
	{
		return;
	}
	payload = value;
	cv.notify_all();
	for(auto dep : deps)
	{
		dep->signal(id, payload);
	}
}

void TimelineSemaphore::Shared::signal(int parentId, uint64_t value)
{
	marl::lock lock(mutex);
	// Either we aren't waiting for a signal, or parentId is not something we're waiting for
	if(payload == 1 || waitMap.find(parentId) == waitMap.end())
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
	payload = 1;
	cv.notify_all();
	for(auto dep : deps)
	{
		dep->signal(id, payload);
	}
}

void TimelineSemaphore::Shared::wait(uint64_t value)
{
	marl::lock lock(mutex);
	cv.wait(lock, [&] { return payload == value; });
}

TimelineSemaphore::TimelineSemaphore(uint64_t initialValue, marl::Allocator *allocator)
    : shared(allocator->make_shared<Shared>(allocator, initialValue))
{
}
TimelineSemaphore::TimelineSemaphore(marl::Allocator *allocator)
    : shared(allocator->make_shared<Shared>(allocator, 0))
{
}

void TimelineSemaphore::signal(uint64_t value) const
{
	shared->signal(value);
}

void TimelineSemaphore::wait(uint64_t value) const
{
	shared->wait(value);
}

int TimelineSemaphore::getId() const
{
	return shared->getId();
}

void TimelineSemaphore::addSharedDep(TimelineSemaphore other)
{
	shared->addSharedDep(other);
}

void TimelineSemaphore::Shared::addSharedDep(TimelineSemaphore other)
{
	other.addSelfToDeps([&](const std::shared_ptr<TimelineSemaphore::Shared> otherShared) { deps.push_back(otherShared); });
}

uint64_t TimelineSemaphore::getPayloadValue() const
{
	marl::lock lock(shared->mutex);
	return shared->payload;
}

void TimelineSemaphore::addToWaitMap(int id, uint64_t waitValue)
{
	shared->waitMap[id] = waitValue;
}

}  // namespace vk
