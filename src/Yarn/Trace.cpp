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

// The Trace API produces a trace event file that can be consumed with Chrome's
// about:tracing viewer.
// Documentation can be found at:
//   https://www.chromium.org/developers/how-tos/trace-event-profiling-tool
//   https://docs.google.com/document/d/1CvAClvFfyA5R-PhYUmn5OOQtYMH4h6I0nSsKchNAySU/edit

#include "Trace.hpp"

#include "Fiber.hpp"
#include "Thread.hpp"

#if YARN_TRACE_ENABLED

#include <atomic>
#include <fstream>

namespace
{

std::atomic<yarn::Trace*> instance;

} // anonymous namespace

namespace yarn
{

Trace* Trace::get()
{
    // HACKS
    static auto file = std::fstream("swiftshader.trace", std::ios_base::out);
    static Trace trace(file);
    return &trace;

    //return instance;
}

void Trace::set(Trace * trace)
{
    instance = trace;
}

Trace::Trace(std::ostream &out)
{
    nameThread("main");
    thread = std::thread([&] {
        Thread::current().setName("Trace worker");
        out << "[" << std::endl;
        auto first = true;
        while (true)
        {
            auto event = take();
            if (event->type() == Event::Type::Shutdown)
            {
                break;
            }
            if (!first) { out << "," << std::endl; };
            first = false;
            out << "{" << std::endl;
            event->write(out);
            out << "}";
            delete event; // TODO: Use pool.
        }
        out << std::endl << "]" << std::endl;
    });
}

Trace::~Trace()
{
    put(new Shutdown());
    thread.join();
}

void Trace::nameThread(const char* fmt, ...)
{
    auto event = new NameThreadEvent();

    va_list vararg;
    va_start(vararg, fmt);
    vsnprintf(event->name, Trace::MaxEventNameLength, fmt, vararg);
    va_end(vararg);

    event->threadID = std::hash<std::thread::id>()(std::this_thread::get_id());
    put(event);
}

void Trace::beginEvent(const char* fmt, ...)
{
    auto event = new BeginEvent();

    va_list vararg;
    va_start(vararg, fmt);
    vsnprintf(event->name, Trace::MaxEventNameLength, fmt, vararg);
    va_end(vararg);

    event->timestamp = timestamp();
    event->threadID = std::hash<std::thread::id>()(std::this_thread::get_id());
    if (auto fiber = yarn::Fiber::current())
    {
        event->fiberID = fiber->id;
    }
    put(event);
}

void Trace::endEvent()
{
    auto event = new EndEvent();
    event->timestamp = timestamp();
    event->threadID = std::hash<std::thread::id>()(std::this_thread::get_id());
    put(event);
}

void Trace::beginAsyncEvent(uint32_t id, const char* fmt, ...)
{
    auto event = new AsyncStartEvent();

    va_list vararg;
    va_start(vararg, fmt);
    vsnprintf(event->name, Trace::MaxEventNameLength, fmt, vararg);
    va_end(vararg);

    event->timestamp = timestamp();
    event->id = id;
    if (auto fiber = yarn::Fiber::current())
    {
        event->fiberID = fiber->id;
    }
    put(event);
}

void Trace::endAsyncEvent(uint32_t id, const char* fmt, ...)
{
    auto event = new AsyncEndEvent();

    va_list vararg;
    va_start(vararg, fmt);
    vsnprintf(event->name, Trace::MaxEventNameLength, fmt, vararg);
    va_end(vararg);

    event->timestamp = timestamp();
    event->id = id;
    put(event);
}

uint64_t Trace::timestamp()
{
    auto now = std::chrono::high_resolution_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::microseconds>(now - createdAt);
    return static_cast<uint64_t>(diff.count());
}

void Trace::put(Event* event)
{
    std::unique_lock<std::mutex> lock(queueMutex);
    auto notify = queue.size() == 0;
    queue.push(event);
    lock.unlock();
    if (notify) { queueCV.notify_one(); }
}

Trace::Event* Trace::take()
{
    std::unique_lock<std::mutex> lock(queueMutex);
    queueCV.wait(lock, [this] { return queue.size() > 0; });
    auto event = queue.front();
    queue.pop();
    return event;
}

#define QUOTE(x) "\"" << x << "\""
#define INDENT "  "

void Trace::Event::write(std::ostream &out) const
{
    out << INDENT << QUOTE("name") << ": " << QUOTE(name) << "," << std::endl;
    if (categories != nullptr)
    {
        out << INDENT << QUOTE("cat") << ": " << "\"";
        auto first = true;
        for (auto category = *categories; category != nullptr; category++)
        {
            if (!first) { out << ","; }
            out << category;
        }
        out << "\"," << std::endl;
    }
    if (fiberID != 0)
    {
        out << INDENT << QUOTE("args") << ": " << "{" << std::endl
            << INDENT << INDENT << QUOTE("fiber") << ": " << fiberID << std::endl
            << INDENT << "}," << std::endl;
    }
    if (threadID != 0)
    {
        out << INDENT << QUOTE("tid") << ": " << threadID << "," << std::endl;
    }
    out << INDENT << QUOTE("ph") << ": " << QUOTE(static_cast<char>(type())) << "," << std::endl
        << INDENT << QUOTE("pid") << ": " << processID << "," << std::endl
        << INDENT << QUOTE("ts") << ": " << timestamp << std::endl;
}

void Trace::NameThreadEvent::write(std::ostream &out) const
{
    out << INDENT << QUOTE("name") << ": " << QUOTE("thread_name") << "," << std::endl
        << INDENT << QUOTE("ph") << ": " << QUOTE("M") << "," << std::endl
        << INDENT << QUOTE("pid") << ": " << processID << "," << std::endl
        << INDENT << QUOTE("tid") << ": " << threadID << "," << std::endl
        << INDENT << QUOTE("args") << ": {"  << QUOTE("name") << ": " << QUOTE(name) << "}" << std::endl;
}

void Trace::AsyncEvent::write(std::ostream &out) const
{
    out << INDENT << QUOTE("id") << ": " << QUOTE(id) << "," << std::endl
        << INDENT << QUOTE("cat") << ": " << QUOTE("async") << "," << std::endl;
    Event::write(out);
}


}  // namespace yarn

#endif // YARN_TRACE_ENABLED