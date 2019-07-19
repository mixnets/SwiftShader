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
    assert(bound == nullptr);
    bound = this;
}

void Scheduler::unbind()
{
    assert(bound == this);
    bound = nullptr;
}

Scheduler::Scheduler() : singleThreadedWorker(this, Worker::Mode::SingleThreaded, 0) {}

Scheduler::~Scheduler()
{
    setWorkerThreadCount(0);
    singleThreadedWorker.flush();
}

void Scheduler::setWorkerThreadCount(size_t count)
{
    std::unique_lock<std::mutex> lock(workerThreadsMutex);
    while (workerThreads.size() > count)
    {
        auto worker = take(workerThreads);

        // Delete these one at a time as any pending tasks will be rescheduled
        // back on to the scheduler, and we don't want them going back to the
        // worker we're about to delete.
        lock.unlock();
        delete worker;
        lock.lock();
    }
    while (workerThreads.size() < count)
    {
        auto worker = new Worker(this, Worker::Mode::MultiThreaded, workerThreads.size() + 1); // +1 for the single-threaded worker
        workerThreads.push_back(worker);
    }
}

size_t Scheduler::getWorkerThreadCount()
{
    std::unique_lock<std::mutex> lock(workerThreadsMutex);
    return workerThreads.size();
}

void Scheduler::enqueue(const Task& task)
{
    std::unique_lock<std::mutex> lock(workerThreadsMutex);
    if (workerThreads.size() > 0)
    {
        auto idx = (nextEnqueueIndex++ % workerThreads.size());
        workerThreads[idx]->enqueue(task);
    }
    else
    {
        singleThreadedWorker.enqueue(task);
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
            NAME_THREAD("Thread<%.2d>", int(id));
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

            // Give up all the tasks this worker has queued.
            // The scheduler will have removed this worker, so rescheduling
            // will hand the tasks to another worker.
            while (work.tasks.size() > 0)
            {
                scheduler->enqueue(take(work.tasks));
            }

            // Fibers must only be executed on this thread, so we need to wait
            // for any blocked ones to complete before finishing.
            work.idle.wait(lock, [this] { return numActiveFibers() == 1; });

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

    std::unique_lock<std::mutex> lock(work.mutex);
    Fiber* to = nullptr;
    if (work.fibers.size() > 0)
    {
        // There's an unblocked fiber we can jump directly to.
        to = take(work.fibers);
    }
    else
    {
        // No unblocked fibers, so we need to re-enter run().
        // Reuse an idle worker fiber, or spawn a new fiber.
        to = (idleFibers.size() > 0) ? take(idleFibers) : createWorkerFiber();
    }
    lock.unlock();

    switchToFiber(to);
}

void Scheduler::Worker::enqueue(Fiber* fiber)
{
    std::unique_lock<std::mutex> lock(work.mutex);
    auto wasIdle = isIdle();
    work.fibers.push(fiber);
    lock.unlock();
    if (wasIdle) { work.added.notify_one(); }
}

void Scheduler::Worker::enqueue(const Task& task)
{
    std::unique_lock<std::mutex> lock(work.mutex);
    auto wasIdle = isIdle();
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
        while (!shutdown)
        {
            runUntilIdle(lock);
            work.idle.notify_all();
            work.added.wait(lock, [&] { return !isIdle() || shutdown; });
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
    while (!isIdle() && !shutdown)
    {
        while (work.fibers.size() > 0)
        {
            // Fibers have become unblocked.
            // We only need one fiber to process the worker, so place this
            // fiber into the cache so it can be reused, and switch to the
            // newly unblocked fiber.
            idleFibers.push(currentFiber);
            auto fiber = take(work.fibers);

            lock.unlock();
            switchToFiber(fiber);
            lock.lock();
        }

        TaskQueue tasks;
        work.tasks.swap(tasks);

        while (tasks.size() > 0)
        {
            auto task = take(tasks);
            lock.unlock();
            task();
            lock.lock();
        }
    }
}

bool Scheduler::Worker::isIdle() const
{
    return work.fibers.size() == 0 && work.tasks.size() == 0;
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
