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

#ifndef yarn_condition_variable_hpp
#define yarn_condition_variable_hpp

#include "Fiber.hpp"
#include "Debug.hpp"

#include <list>
#include <memory>
#include <mutex>

namespace yarn {

class ConditionVariable
{
public:
    inline void notify_one();
    inline void notify_all();

    template <typename Predicate>
    inline void wait(std::unique_lock<std::mutex>& lock, Predicate pred);
private:
    std::mutex mutex;
    std::list<Fiber*> waiting;
    std::condition_variable condition;
};

void ConditionVariable::notify_one()
{
    std::unique_lock<std::mutex> lock(mutex);
    if (waiting.size() > 0)
    {
        auto fiber = waiting.back();
        waiting.pop_back();
        fiber->schedule();
    }
    condition.notify_one();
}

void ConditionVariable::notify_all()
{
    std::unique_lock<std::mutex> lock(mutex);
    while (waiting.size() > 0)
    {
        auto fiber = waiting.back();
        waiting.pop_back();
        fiber->schedule();
    }
    condition.notify_all();
}

template <typename Predicate>
void ConditionVariable::wait(std::unique_lock<std::mutex>& dataLock, Predicate pred)
{
    if (auto fiber = Fiber::current())
    {
        // Currently executing on a scheduler fiber.
        // Yield to let other tasks run that can unblock this fiber.
        std::unique_lock<std::mutex> waitingLock(mutex);
        while (!pred())
        {
            waiting.push_back(fiber);

            waitingLock.unlock();
            dataLock.unlock();

            fiber->yield();

            // Beware: Lock order here is important!
            // It must match the entry lock order, otherwise we can deadlock.
            dataLock.lock();
            waitingLock.lock();
        }
    }
    else
    {
        // Currently running outside of the scheduler.
        // Delegate to the std::condition_variable.
        condition.wait(dataLock, pred);
    }
}

} // namespace yarn

#endif // yarn_condition_variable_hpp
