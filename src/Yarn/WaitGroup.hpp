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
    inline WaitGroup(unsigned int initialCount = 0);
    inline void add(unsigned int count = 1) const;
    inline bool done() const;
    inline void wait() const;

private:
    struct Data
    {
        unsigned int count;
        ConditionVariable condition;
        std::mutex mutex;
    };
    const std::shared_ptr<Data> data = std::make_shared<Data>();
};

inline WaitGroup::WaitGroup(unsigned int initialCount /* = 0 */)
{
    std::unique_lock<std::mutex> lock(data->mutex);
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
    YARN_ASSERT(data->count > 0, "yarn::WaitGroup::done() called too many times");
    data->count--;
    if (data->count == 0)
    {
        lock.unlock();
        data->condition.notify_all();
        return true;
    }
    return false;
}

void WaitGroup::wait() const
{
    std::unique_lock<std::mutex> lock(data->mutex);
    data->condition.wait(lock, [this]{ return data->count == 0; });
}

} // namespace yarn

#endif  // yarn_waitgroup_hpp
