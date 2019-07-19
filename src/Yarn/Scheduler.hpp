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

#ifndef yarn_scheduler_hpp
#define yarn_scheduler_hpp

#include "Debug.hpp"

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

// SAL annotations. See https://docs.microsoft.com/en-us/visualstudio/code-quality/annotating-locking-behavior?view=vs-2019
#ifndef _Requires_lock_held_
#define _Requires_lock_held_(x)
#endif

namespace yarn {

class Fiber;

class Scheduler
{
public:
    using Task = std::function<void()>;

    Scheduler();
    ~Scheduler();

    static Scheduler *get();

    void bind();
    void unbind();

    void enqueue(const Task& task);

    void setThreadInitializer(const Task& init);
    Task getThreadInitializer();

    void setWorkerThreadCount(size_t count);
    size_t getWorkerThreadCount();

private:
    friend class Fiber;

    static constexpr size_t FiberStackSize = 256 * 1024;

    using TaskQueue = std::queue<Task>;
    using FiberQueue = std::queue<Fiber*>;

    class Worker
    {
    public:
        enum class Mode
        {
            // Worker will spawn a background thread to process tasks.
            MultiThreaded,

            // Worker will execute tasks whenever it yields.
            SingleThreaded,
        };

        Worker(Scheduler *scheduler, Mode mode, uint32_t id);
        ~Worker();

        void yield(Fiber *fiber);
        void enqueue(Fiber *fiber);
        void enqueue(const Task &task);
        void flush();

        static Worker* getCurrent();
        inline Fiber* getCurrentFiber() const;

        const uint32_t id;

    private:
        Fiber* createWorkerFiber();
        void switchToFiber(Fiber*);
        int numActiveFibers();
        void run();
        void runUntilIdle(std::unique_lock<std::mutex> &lock);

        struct Work
        {
            TaskQueue tasks;
            std::condition_variable added;
            std::mutex mutex;
        };

        static thread_local Worker* current;

        Mode const mode;
        Scheduler* const scheduler;
        std::unique_ptr<Fiber> mainFiber;
        Fiber* currentFiber = nullptr;
        std::thread thread;
        Work work;
        FiberQueue pendingFibers;
        FiberQueue idleFibers;
        std::vector<std::unique_ptr<Fiber>> workerFibers;
        bool shutdown = false;
    };

    static thread_local Scheduler* bound;

    Task threadInitFunc;
    std::atomic<uint64_t> nextEnqueueIndex = {0};
    std::vector<Worker*> workerThreads;
    Worker singleThreadedWorker;
    std::mutex mutex;
};

Fiber* Scheduler::Worker::getCurrentFiber() const
{
    return currentFiber;
}

template<typename Function, typename ... Args>
inline void schedule(Function&& f, Args&& ... args)
{
    YARN_ASSERT_HAS_BOUND_SCHEDULER("yarn::schedule");
    auto scheduler = Scheduler::get();
    scheduler->enqueue(std::bind(std::forward<Function>(f), std::forward<Args>(args)...));
}

inline void schedule(const Scheduler::Task& f)
{
    YARN_ASSERT_HAS_BOUND_SCHEDULER("yarn::schedule");
    auto scheduler = Scheduler::get();
    scheduler->enqueue(f);
}

} // namespace yarn

#endif // yarn_scheduler_hpp
