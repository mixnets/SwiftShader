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

#ifndef sw_Synchronization_hpp
#define sw_Synchronization_hpp

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>

namespace sw
{

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
// At the same time, wait() can be used to block until all tasks have finished.
class WaitGroup : public TaskEvents
{
public:
	// add() begins a new task.
	void add()
	{
		std::unique_lock<std::mutex> lock(mutex);
		++count_;
	}

	// done() is called when a task of the WaitGroup has been completed.
	// Returns true if there are no more tasks currently running in the
	// WaitGroup.
	bool done()
	{
		std::unique_lock<std::mutex> lock(mutex);
		assert(count_ > 0);
		--count_;
		if(count_ == 0)
		{
			condition.notify_all();
		}
		return count_ == 0;
	}

	// wait() blocks until all the tasks have been finished.
	void wait()
	{
		std::unique_lock<std::mutex> lock(mutex);
		if (count_ > 0)
		{
			condition.wait(lock, [this] { return count_ == 0; });
		}
	}

	// wait() blocks until all the tasks have been finished or the timeout
	// has been reached, returning true if all tasks have been completed, or
	// false if the timeout has been reached.
    template <class CLOCK, class DURATION>
	bool wait(const std::chrono::time_point<CLOCK, DURATION>& timeout)
	{
		std::unique_lock<std::mutex> lock(mutex);
		return condition.wait_until(lock, timeout, [this] { return count_ == 0; });
	}

	// count() returns the number of times add() has been called without a call
	// to done().
	// Note: No lock is held after count() returns, the count may immediate
	// change after returning.
	int32_t count()
	{
		std::unique_lock<std::mutex> lock(mutex);
		return count_;
	}

	// TaskEvents compliance
	void start() override { add(); }
	void finish() override { done(); }

private:
	int32_t count_ = 0; // guarded by mutex
	std::mutex mutex;
	std::condition_variable condition;
};

class Event
{
public:
	Event() : value(false) {}
	Event(bool initialState) : value(initialState) {}

	// signal() signals the event, unblocking any calls to wait().
	void signal()
	{
		std::unique_lock<std::mutex> lock(mutex);
		value = true;
		condition.notify_all();
	}

	// clear() sets the event signal to false.
	void clear()
	{
		std::unique_lock<std::mutex> lock(mutex);
		value = false;
		condition.notify_all();
	}

	// wait() blocks until all the event signal is set to true.
	void wait()
	{
		std::unique_lock<std::mutex> lock(mutex);
		condition.wait(lock, [this] { return value; });
	}

	// wait() blocks until the event signal is set to true or the timeout
	// has been reached, returning true if all tasks have been completed, or
	// false if the timeout has been reached.
    template <class CLOCK, class DURATION>
	bool wait(const std::chrono::time_point<CLOCK, DURATION>& timeout)
	{
		std::unique_lock<std::mutex> lock(mutex);
		return condition.wait_until(lock, timeout, [this] { return value; });
	}

	operator bool()
	{
		std::unique_lock<std::mutex> lock(mutex);
		return value;
	}

public:
	bool value; // guarded by mutex
	std::mutex mutex;
	std::condition_variable condition;
};

} // namespace sw

#endif // sw_Synchronization_hpp
