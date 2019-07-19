// Copyright 2019 The SwiftShader Authors. All Rights Reserved.
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

#ifndef yarn_event_hpp
#define yarn_event_hpp

#include "ConditionVariable.hpp"

namespace yarn {

enum class EventType
{
    // The event will remain in the signaled state when calling signal().
    // While the event is in the signaled state, any calls to wait() will
    // unblock without automatically reseting the signaled state.
    // The signaled state can be reset with a call to clear().
    ManualReset,

    // The event signal will be automatically reset when a call to wait()
    // returns.
    // A single call to signal() will only unblock a single (possibly
    // pending) call to wait().
    AutoReset,
};

// Event is a synchronization mechanism used to indicate to waiting threads
// when a boolean condition has become true.
template <EventType Type = EventType::ManualReset>
class Event
{
public:
	Event(bool initialState = false) : signaled(initialState) {}

	// signal() signals the event, unblocking any calls to wait().
	void signal()
	{
		std::unique_lock<std::mutex> lock(mutex);
		if (signaled) { return; }
		signaled = true;
		if (Type == EventType::AutoReset)
		{
			condition.notify_one();
		}
		else
		{
			condition.notify_all();
		}
	}

	// clear() sets the event signal to false.
	void clear()
	{
		std::unique_lock<std::mutex> lock(mutex);
		signaled = false;
	}

	// wait() blocks until all the event signal is set to true.
	// If the event was constructed with the Auto ClearMode, then the signal
	// is cleared before returning.
	void wait()
	{
		std::unique_lock<std::mutex> lock(mutex);
		condition.wait(lock, [this] { return signaled; });
		if (Type == EventType::AutoReset)
		{
			signaled = false;
		}
	}

	// bool() returns true if the event is signaled, otherwise false.
	// Note: No lock is held after bool() returns, so the event state may
	// immediately change after returning.
	operator bool()
	{
		std::unique_lock<std::mutex> lock(mutex);
		return signaled;
	}

public:
	bool signaled; // guarded by mutex
	std::mutex mutex;
	ConditionVariable condition;
};

} // namespace yarn

#endif  // yarn_event_hpp
