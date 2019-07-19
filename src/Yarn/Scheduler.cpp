// Copyright 2019 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "Scheduler.hpp"

#include "Debug.hpp"
#include "Defer.hpp"
#include "Fiber.hpp"
#include "Thread.hpp"
#include "Trace.hpp"

#include <assert.h>

#if 0
#define TRACE(...) SCOPED_EVENT(__VA_ARGS__)
#else
#define TRACE(...)
#endif


namespace
{

template <typename T>
inline T take(std::queue<T>& queue)
{
    auto out = std::move(queue.front());
    queue.pop();
    return out;
}

inline void nop()
{
    #if defined(_WIN32)
        __nop();
    #else
        __asm__ __volatile__ ("nop");
    #endif
}

} // anonymous namespace

namespace yarn {

////////////////////////////////////////////////////////////////////////////////
// Scheduler
////////////////////////////////////////////////////////////////////////////////
thread_local Scheduler* Scheduler::bound = nullptr;

Scheduler* Scheduler::get()
{
    return bound;
}

void Scheduler::bind()
{
    YARN_ASSERT(bound == nullptr, "Scheduler already bound");
    bound = this;
}

void Scheduler::unbind()
{
    YARN_ASSERT(bound == this, "Different scheduler bound");
    bound = nullptr;
}

Scheduler::Scheduler() : singleThreadedWorker(this, Worker::Mode::SingleThreaded, 0)
{
    for (size_t i = 0; i < spinningWorkers.size(); i++)
    {
        spinningWorkers[i] = -1;
    }
    singleThreadedWorker.start();
}

Scheduler::~Scheduler()
{
    setWorkerThreadCount(0);
    singleThreadedWorker.flush();
    singleThreadedWorker.stop();
}

void Scheduler::setThreadInitializer(Task&& func)
{
    std::unique_lock<std::mutex> lock(threadInitFuncMutex);
    threadInitFunc = std::move(func);
}

const Task& Scheduler::getThreadInitializer()
{
    std::unique_lock<std::mutex> lock(threadInitFuncMutex);
    return threadInitFunc;
}

void Scheduler::setWorkerThreadCount(int newCount)
{
    YARN_ASSERT(newCount >= 0, "count must be positive");
    auto oldCount = numWorkerThreads;
    for (int idx = oldCount - 1; idx >= newCount; idx--)
    {
        workerThreads[idx]->stop();
    }
    for (int idx = oldCount - 1; idx >= newCount; idx--)
    {
        delete workerThreads[idx];
    }
    for (int idx = oldCount; idx < newCount; idx++)
    {
        workerThreads[idx] = new Worker(this, Worker::Mode::MultiThreaded, idx);
    }
    numWorkerThreads = newCount;
    for (int idx = oldCount; idx < newCount; idx++)
    {
        workerThreads[idx]->start();
    }
}

int Scheduler::getWorkerThreadCount()
{
    return numWorkerThreads;
}

void Scheduler::enqueue(Task&& task)
{
    if (numWorkerThreads > 0)
    {
        for (size_t i = 0; i < spinningWorkers.size(); i++)
        {
            auto idx = spinningWorkers[i].exchange(-1);
            if (idx >= 0)
            {
                workerThreads[idx]->enqueue(std::move(task));
                return;
            }
        }

        auto idx = nextEnqueueIndex++ % numWorkerThreads;
        workerThreads[idx]->enqueue(std::move(task));
    }
    else
    {
        singleThreadedWorker.enqueue(std::move(task));
    }
}

bool Scheduler::stealWork(Worker* thief, uint64_t from, Task& out)
{
    if (numWorkerThreads > 0)
    {
        auto thread = workerThreads[from % numWorkerThreads];
        if (thread != thief)
        {
            if (thread->dequeue(out))
            {
                return true;
            }
        }
    }
    return false;
}

void Scheduler::onBeginSpinning(int workerId)
{
    auto idx = nextSpinningWorkerIdx++ % spinningWorkers.size();
    spinningWorkers[idx] = workerId;
}

////////////////////////////////////////////////////////////////////////////////
// Scheduler::Worker
////////////////////////////////////////////////////////////////////////////////
thread_local Scheduler::Worker* Scheduler::Worker::current = nullptr;

Scheduler::Worker::Worker(Scheduler *scheduler, Mode mode, uint32_t id) : id(id), mode(mode), scheduler(scheduler) {}

void Scheduler::Worker::start()
{
    switch (mode)
    {
    case Mode::MultiThreaded:
        thread = std::thread([=]
        {
            Thread::setName("Thread<%.2d>", int(id));

            if (auto const &initFunc = scheduler->getThreadInitializer())
            {
                initFunc();
            }

            scheduler->bind();
            Worker::current = this;
            mainFiber.reset(Fiber::createFromCurrentThread(0, FiberStackSize));
            currentFiber = mainFiber.get();
            run();
            mainFiber.reset();
            Worker::current = nullptr;
        });
        break;

    case Mode::SingleThreaded:
        Worker::current = this;
        mainFiber.reset(Fiber::createFromCurrentThread(0, FiberStackSize));
        currentFiber = mainFiber.get();
        break;

    default:
        YARN_ASSERT(false, "Unknown mode: %d", int(mode));
    }
}

void Scheduler::Worker::stop()
{
    switch (mode)
    {
    case Mode::MultiThreaded:
        shutdown = true;
        enqueue([]{});
        thread.join();
        break;

    case Mode::SingleThreaded:
        Worker::current = nullptr;
        break;

    default:
        YARN_ASSERT(false, "Unknown mode: %d", int(mode));
    }
}

Scheduler::Worker* Scheduler::Worker::getCurrent()
{
    return Worker::current;
}

void Scheduler::Worker::yield(Fiber *from)
{
    assert(currentFiber == from);
    // Current fiber is yielding as it is blocked.

    std::unique_lock<std::mutex> lock(work.mutex);
    waitForWork(lock);

    if (work.fibers.size() > 0)
    {
        work.num--;
        auto to = take(work.fibers);
        lock.unlock();
        switchToFiber(to);
    }
    else if (idleFibers.size() > 0)
    {
        auto to = take(idleFibers);
        lock.unlock();
        switchToFiber(to);
    }
    else
    {
        lock.unlock();
        switchToFiber(createWorkerFiber());
    }
}

void Scheduler::Worker::enqueue(Fiber* fiber)
{
    std::unique_lock<std::mutex> lock(work.mutex);
    auto wasIdle = work.num == 0;
    work.fibers.push(std::move(fiber));
    work.num++;
    lock.unlock();
    if (wasIdle) { work.added.notify_one(); }
}

void Scheduler::Worker::enqueue(Task&& task)
{
    std::unique_lock<std::mutex> lock(work.mutex);
    auto wasIdle = work.num == 0;
    work.tasks.push(std::move(task));
    work.num++;
    lock.unlock();
    if (wasIdle) { work.added.notify_one(); }
}

bool Scheduler::Worker::dequeue(Task& out)
{
    if (work.num.load() == 0) { return false; }

    std::unique_lock<std::mutex> lock(work.mutex);
    if (work.tasks.size() == 0) { return false; }
    work.num--;
    out = take(work.tasks);
    return true;
}

void Scheduler::Worker::flush()
{
    YARN_ASSERT(mode == Mode::SingleThreaded, "flush() can only be used on a single-threaded worker");
    std::unique_lock<std::mutex> lock(work.mutex);
    runUntilIdle(lock);
}

void Scheduler::Worker::run()
{
    switch (mode)
    {
    case Mode::MultiThreaded:
    {
        NAME_THREAD("Thread<%.2d> Fiber<%.2d>", int(id), Fiber::current()->id);

#if 0
        Thread::AffinityMask mask;
        mask.set(id, true);
        Thread::setAffinity(mask);
#endif

        std::unique_lock<std::mutex> lock(work.mutex);
        work.added.wait(lock, [this] { return work.num > 0 || shutdown; });
        while (!shutdown)
        {
            waitForWork(lock);
            runUntilIdle(lock);
        }
        Worker::current = nullptr;
        break;
    }
    case Mode::SingleThreaded:
        flush();
        break;

    default:
        YARN_ASSERT(false, "Unknown mode: %d", int(mode));
    }

    switchToFiber(mainFiber.get());
}

_Requires_lock_held_(lock)
void Scheduler::Worker::waitForWork(std::unique_lock<std::mutex> &lock)
{
    YARN_ASSERT(work.num == work.fibers.size() + work.tasks.size(), "work.num out of sync");
    if (work.num == 0)
    {
        scheduler->onBeginSpinning(id);
        lock.unlock();
        spin();
        lock.lock();
    }
    work.added.wait(lock, [this] { return work.num > 0 || shutdown; });
}

void Scheduler::Worker::spin()
{
    TRACE("SPIN");
    Task stolen;

    constexpr auto duration = std::chrono::milliseconds(1);
    auto start = std::chrono::high_resolution_clock::now();
    while (std::chrono::high_resolution_clock::now() - start < duration)
    {
        for (int i = 0; i < 256; i++)
        {
            nop(); nop(); nop(); nop(); nop(); nop(); nop(); nop();
            nop(); nop(); nop(); nop(); nop(); nop(); nop(); nop();
            nop(); nop(); nop(); nop(); nop(); nop(); nop(); nop();
            nop(); nop(); nop(); nop(); nop(); nop(); nop(); nop();
            if (work.num > 0)
            {
                return;
            }
        }

        if (scheduler->stealWork(this, rng(), stolen))
        {
            std::unique_lock<std::mutex> lock(work.mutex);
            work.tasks.emplace(std::move(stolen));
            work.num++;
            return;
        }
        std::this_thread::yield();
    }
}

_Requires_lock_held_(lock)
void Scheduler::Worker::runUntilIdle(std::unique_lock<std::mutex> &lock)
{
    YARN_ASSERT(work.num == work.fibers.size() + work.tasks.size(), "work.num out of sync");
    while (work.fibers.size() > 0 || work.tasks.size() > 0)
    {
        while (work.fibers.size() > 0)
        {
            work.num--;
            auto fiber = take(work.fibers);
            lock.unlock();
            idleFibers.push(currentFiber);
            switchToFiber(fiber);
            lock.lock();
        }

        if (work.tasks.size() > 0)
        {
            work.num--;
            auto task = take(work.tasks);
            lock.unlock();

            // Run the task.
            task();

            // std::function<> can carry arguments with complex destructors.
            // Ensure these are destructed outside of the lock.
            task = Task();

            lock.lock();
        }
    }
}

Fiber* Scheduler::Worker::createWorkerFiber()
{
    auto fiber = Fiber::create(workerFibers.size() + 1, FiberStackSize, [&] { run(); });
    workerFibers.push_back(std::unique_ptr<Fiber>(fiber));
    return fiber;
}

void Scheduler::Worker::switchToFiber(Fiber* to)
{
    auto from = currentFiber;
    currentFiber = to;
    from->switchTo(to);
}

} // namespace yarn
