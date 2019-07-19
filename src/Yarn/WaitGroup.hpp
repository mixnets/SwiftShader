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

#include <mutex>

#include <assert.h>

namespace yarn {

class WaitGroup
{
public:
    inline WaitGroup() = default;
    inline WaitGroup(unsigned int initialCount);
    inline void add(unsigned int count = 1);
    inline void done();
    inline void wait();
private:
    WaitGroup(const WaitGroup&) = delete;
    WaitGroup& operator = (const WaitGroup&) = delete;

    unsigned int count = 0;
    ConditionVariable condition;
    std::mutex mutex;
};

inline WaitGroup::WaitGroup(unsigned int initialCount) : count(initialCount) {}

void WaitGroup::add(unsigned int count /* = 1  */)
{
    std::unique_lock<std::mutex> lock(mutex);
    this->count += count;
}

void WaitGroup::done()
{
    std::unique_lock<std::mutex> lock(mutex);
    YARN_ASSERT(count > 0, "yarn::WaitGroup::done() called too many times");
    count--;
    if (count == 0)
    {
        condition.notify_all();
    }
}

void WaitGroup::wait()
{
    std::unique_lock<std::mutex> lock(mutex);
    condition.wait(lock, [this]{ return count == 0; });
}

} // namespace yarn

#endif  // yarn_waitgroup_hpp
