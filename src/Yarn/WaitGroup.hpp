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

#ifndef yarn_waitgroup_hpp
#define yarn_waitgroup_hpp

#include "ConditionVariable.hpp"
#include "Debug.hpp"

#include <atomic>
#include <mutex>

namespace yarn {

// WaitGroup is a synchronization primitive that holds an internal counter that
// can incremented, decremented and waited on until it reaches 0.
// WaitGroups can be used as a simple mechanism for waiting on a number of
// concurrently execute a number of tasks to complete.
//
// Example:
//
//  void runTasksConcurrently(int numConcurrentTasks)
//  {
//      // Construct the WaitGroup with an initial count of numConcurrentTasks.
//      yarn::WaitGroup wg(numConcurrentTasks);
//      for (int i = 0; i < numConcurrentTasks; i++)
//      {
//          // Schedule a task to be run asynchronously.
//          // These may all be run concurrently.
//          yarn::schedule([=] {
//              // Once the task has finished, decrement the waitgroup counter
//              // to signal that this has completed.
//              defer(wg.done());
//              doSomeWork();
//          });
//      }
//      // Block until all tasks have completed.
//      wg.wait();
//  }
//
// IMPORTANT NOTE: Unlike ConditionVariable, WaitGroup is a copyable object
// that can be shared safely between different fibers and threads.
class WaitGroup
{
public:
    // Constructs the WaitGroup with the specified initial count.
    inline WaitGroup(unsigned int initialCount = 0);

    // add() increments the internal counter by count.
    inline void add(unsigned int count = 1) const;

    // done() decrements the internal counter by one.
    // Returns true if the internal count has reached zero.
    inline bool done() const;

    // wait() blocks until the WaitGroup counter reaches zero.
    inline void wait() const;

private:
    struct Data {
        unsigned int count = 0;
        ConditionVariable condition;
        std::mutex mutex;
    };
    std::shared_ptr<Data> data = std::make_shared<Data>();
};

inline WaitGroup::WaitGroup(unsigned int initialCount /* = 0 */)
{
    data->count = initialCount;
}

void WaitGroup::add(unsigned int count /* = 1 */) const
{
    std::unique_lock<std::mutex> lock(data->mutex);
    data->count += count;
}

bool WaitGroup::done() const
{
    std::unique_lock<std::mutex> lock(data->mutex);
    bool result = false;
    YARN_ASSERT(data->count > 0, "yarn::WaitGroup::done() called too many times");
    auto count = --data->count;
    if (count == 0)
    {
        // Technical note: this notify_all() called is performed with the
        // mutex acquired. This is slightly less efficient than doing it after
        // releasing the lock, but avoids subtle ordering issues that can lead
        // to runtime crashes if one cannot guarantee that the WaitGroup
        // instance still exists when this specific call is performed.
        data->condition.notify_all();
        result = true;
    }
    return result;
}

void WaitGroup::wait() const
{
    std::unique_lock<std::mutex> lock(data->mutex);
    data->condition.wait(lock, [this]{ return data->count == 0; });
}

} // namespace yarn

#endif  // yarn_waitgroup_hpp
