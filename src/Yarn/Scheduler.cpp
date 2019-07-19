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
#include "Fiber.hpp"
#include "Thread.hpp"
#include "Trace.hpp"

#include <assert.h>

namespace
{

template <typename T>
inline T take(std::queue<T>& queue)
{
    T out = queue.front();
    queue.pop();
    return out;
}

template <typename T>
inline T take(std::vector<T>& vector)
{
    T out = vector.back();
    vector.pop_back();
    return out;
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

Scheduler::Scheduler() : singleThreadedWorker(this, Worker::Mode::SingleThreaded, 0) {}

Scheduler::~Scheduler()
{
    setWorkerThreadCount(0);
    singleThreadedWorker.flush();
}

void Scheduler::setThreadInitializer(const Task& func)
{
    std::unique_lock<std::mutex> lock(threadInitFuncMutex);
    threadInitFunc = func;
}

Scheduler::Task Scheduler::getThreadInitializer()
{
    std::unique_lock<std::mutex> lock(threadInitFuncMutex);
    return threadInitFunc;
}

void Scheduler::setWorkerThreadCount(size_t count)
{
    std::unique_lock<std::mutex> lock(setWorkerThreadMutex);
    while (workerThreadCount > count)
    {
        auto idx = --workerThreadCount;
        auto worker = workerThreads[idx].exchange(nullptr);
        delete worker;
    }
    while (workerThreadCount < count)
    {
        auto worker = new Worker(this, Worker::Mode::MultiThreaded, workerThreadCount + 1); // +1 for the single-threaded worker
        auto idx = workerThreadCount.load();
        workerThreads[idx].store(worker);
        workerThreadCount++;
    }
}

size_t Scheduler::getWorkerThreadCount()
{
    return workerThreadCount.load();
}

void Scheduler::enqueue(const Task& task)
{
    while(true)
    {
        auto numThreads = workerThreadCount.load();
        if (numThreads > 0)
        {
            auto idx = nextEnqueueIndex++ % numThreads;
            auto thread = workerThreads[idx].load();
            if (thread != nullptr)
            {
                thread->enqueue(task);
                return;
            }
        }
        else
        {
            singleThreadedWorker.enqueue(task);
            return;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// Scheduler::Worker
////////////////////////////////////////////////////////////////////////////////
thread_local Scheduler::Worker* Scheduler::Worker::current = nullptr;

Scheduler::Worker::Worker(Scheduler *scheduler, Mode mode, uint32_t id) : id(id), mode(mode), scheduler(scheduler)
{
    switch (mode)
    {
    case Mode::MultiThreaded:
        thread = std::thread([=]
        {
            Thread::current().setName("Thread<%.2d>", int(id));
            if (auto initFunc = scheduler->getThreadInitializer())
            {
                initFunc();
            }
            scheduler->bind();
            Worker::current = this;
            mainFiber.reset(Fiber::createFromCurrentThread(FiberStackSize));
            currentFiber = mainFiber.get();
            switchToFiber(createWorkerFiber());
            mainFiber.reset();
        });
        break;

    case Mode::SingleThreaded:
        Worker::current = this;
        mainFiber.reset(Fiber::createFromCurrentThread(FiberStackSize));
        currentFiber = mainFiber.get();
        break;

    default:
        YARN_ASSERT(false, "Unknown mode: %d", int(mode));
    }
}

Scheduler::Worker::~Worker()
{
    switch (mode)
    {
    case Mode::MultiThreaded:
        {
            std::unique_lock<std::mutex> lock(work.mutex);
            shutdown = true;
        }
        work.added.notify_all();
        thread.join(); // Must be called outside of the lock of work.mutex.
        break;

    case Mode::SingleThreaded:
        break;

    default:
        YARN_ASSERT(false, "Unknown mode: %d", int(mode));
    }


    Worker::current = nullptr;
}

Scheduler::Worker* Scheduler::Worker::getCurrent()
{
    return Worker::current;
}

void Scheduler::Worker::yield(Fiber *from)
{
    assert(currentFiber == from);
    // Current fiber is yielding as it is blocked.

    // No unblocked fibers, so we need to re-enter run().
    // Reuse an idle worker fiber, or spawn a new fiber.
    switchToFiber((idleFibers.size() > 0) ? take(idleFibers) : createWorkerFiber());
}

void Scheduler::Worker::enqueue(Fiber* fiber)
{
    enqueue([this, fiber] {
        // We only need one fiber to process the work queue, so place this
        // fiber into the cache so it can be reused, and switch to the
        // newly unblocked fiber.
        idleFibers.push(currentFiber);
        switchToFiber(fiber);
    });
}

void Scheduler::Worker::enqueue(const Task& task)
{
    std::unique_lock<std::mutex> lock(work.mutex);
    auto wasIdle = work.tasks.size() == 0;
    work.tasks.push(task);
    lock.unlock();
    if (wasIdle) { work.added.notify_one(); }
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
        std::unique_lock<std::mutex> lock(work.mutex);
        while (!shutdown || numActiveFibers() > 1)
        {
            work.added.wait(lock, [&] {
                return work.tasks.size() > 0 || (shutdown && numActiveFibers() == 1);
            });
            runUntilIdle(lock);
        }
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

void Scheduler::Worker::runUntilIdle(std::unique_lock<std::mutex> &lock)
{
    Thread::setName("Thread<%.2d> ACTIVE", int(id));

    while (work.tasks.size() > 0)
    {
        auto task = std::move(work.tasks.front());
        work.tasks.pop();
        lock.unlock();

        // Run the task.
        task();

        // std::function<> can carry arguments with complex destructors.
        // Ensure these are destructed outside of the lock.
        task = Task();

        lock.lock();
    }

    Thread::setName("Thread<%.2d> IDLE", int(id));
}

Fiber* Scheduler::Worker::createWorkerFiber()
{
    auto fiber = Fiber::create(FiberStackSize, [&] { run(); });
    workerFibers.push_back(std::unique_ptr<Fiber>(fiber));
    return fiber;
}

int Scheduler::Worker::numActiveFibers()
{
    return workerFibers.size() - idleFibers.size();
}

void Scheduler::Worker::switchToFiber(Fiber* to)
{
    auto from = currentFiber;
    currentFiber = to;
    from->switchTo(to);
}

} // namespace yarn
