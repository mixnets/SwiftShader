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
#ifndef _Requires_lock_not_held_
#define _Requires_lock_not_held_(x)
#endif

namespace yarn {

using Task = std::function<void()>;

class Fiber;

class Scheduler
{
public:
    Scheduler();
    ~Scheduler();

    static Scheduler *get();

    void bind();
    void unbind();

    void enqueue(Task&& task);

    void setThreadInitializer(Task&& init);
    const Task& getThreadInitializer();

    void setWorkerThreadCount(int count);
    int getWorkerThreadCount();

private:
    friend class Fiber;

    static constexpr size_t FiberStackSize = 1024 * 1024;
    static constexpr size_t MaxWorkerThreads = 64;

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

        void start();
        void stop();

        void yield(Fiber* fiber);
        void enqueue(Fiber* fiber);
        void enqueue(Task&& task);
        bool tryLock();
        void enqueueAndUnlock(Task&& task);
        void flush();

        bool dequeue(Task& out);

        static Worker* getCurrent();
        inline Fiber* getCurrentFiber() const;

        const uint32_t id;

    private:
        Fiber* createWorkerFiber();
        void switchToFiber(Fiber*);
        void run();

        _Requires_lock_held_(lock)
        void runUntilIdle(std::unique_lock<std::mutex> &lock);

        _Requires_lock_held_(lock)
        void waitForWork(std::unique_lock<std::mutex> &lock);

        void spin();

        struct Work
        {
            std::atomic<uint64_t> num = { 0 };
            TaskQueue tasks;
            FiberQueue fibers;
            std::condition_variable added;
            std::mutex mutex;
        };

        // https://en.wikipedia.org/wiki/Xorshift
        class FastRnd
        {
        public:
            inline uint64_t operator ()()
            {
                x ^= x << 13;
                x ^= x >> 7;
                x ^= x << 17;
                return x;
            }
        private:
            uint64_t x = std::chrono::system_clock::now().time_since_epoch().count();
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
        FastRnd rng;

        std::atomic<bool> isIdle = { false };
        std::atomic<bool> shutdown = { false };
    };

    bool stealWork(Worker* thief, uint64_t from, Task& out);
    void onBeginSpinning(int workerId);

    static thread_local Scheduler* bound;

    Task threadInitFunc;
    std::mutex threadInitFuncMutex;

    std::array<std::atomic<int>, 8> spinningWorkers;
    std::atomic<unsigned int> nextSpinningWorkerIdx = { 0x8000000 };

    unsigned int nextEnqueueIndex = 0;
    unsigned int numWorkerThreads = 0;
    std::array<Worker*, MaxWorkerThreads> workerThreads;
    Worker singleThreadedWorker;
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

template<typename Function>
inline void schedule(Function&& f)
{
    YARN_ASSERT_HAS_BOUND_SCHEDULER("yarn::schedule");
    auto scheduler = Scheduler::get();
    scheduler->enqueue(std::forward<Function>(f));
}

} // namespace yarn

#endif // yarn_scheduler_hpp
