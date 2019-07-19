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

#if 1
#define TRACE(...) SCOPED_EVENT(__VA_ARGS__)
#else
#define TRACE(...)
#endif


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

void Scheduler::enqueue(Task&& task)
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
                thread->enqueue(std::move(task));
                return;
            }
        }
        else
        {
            singleThreadedWorker.enqueue(std::move(task));
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
            mainFiber.reset(Fiber::createFromCurrentThread(0, FiberStackSize));
            currentFiber = mainFiber.get();
            run();
            mainFiber.reset();
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

Scheduler::Worker::~Worker()
{
    switch (mode)
    {
    case Mode::MultiThreaded:
        enqueue([this]{ shutdown = true; });
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

    std::unique_lock<std::mutex> lock(work.mutex);
    waitForWork(lock);

    if (work.fibers.size() > 0)
    {
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
    auto wasIdle = isIdle();
    work.fibers.push(std::move(fiber));
    lock.unlock();
    if (wasIdle) { work.added.notify_one(); }
}

void Scheduler::Worker::enqueue(Task&& task)
{
    std::unique_lock<std::mutex> lock(work.mutex);
    auto wasIdle = isIdle();
    work.tasks.push(std::move(task));
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
        NAME_THREAD("Thread<%.2d> Fiber<%.2d>", int(id), Fiber::current()->id);

        std::unique_lock<std::mutex> lock(work.mutex);
        while (!shutdown)
        {
            waitForWork(lock);
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

bool Scheduler::Worker::isIdle()
{
    return work.tasks.size() == 0 && work.fibers.size() == 0;
}

void Scheduler::Worker::waitForWork(std::unique_lock<std::mutex> &lock)
{
    TRACE("WAIT");
    work.added.wait(lock, [this] { return !isIdle(); });
}

void Scheduler::Worker::runUntilIdle(std::unique_lock<std::mutex> &lock)
{
    while (!isIdle())
    {
        while (work.fibers.size() > 0)
        {
            auto fiber = take(work.fibers);
            lock.unlock();
            idleFibers.push(currentFiber);
            switchToFiber(fiber);
            lock.lock();
        }

        if (work.tasks.size() > 0)
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
