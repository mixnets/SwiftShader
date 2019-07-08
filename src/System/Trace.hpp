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

#include "Synchronization.hpp"

#include <chrono>
#include <cstdarg>
#include <cstring>
#include <ostream>
#include <thread>

namespace sw
{

class Trace
{
public:
    static constexpr size_t MaxEventNameLength = 64;

    Trace(std::ostream &out);
    ~Trace();

    static Trace* get();
    static void set(Trace *);

    void nameThread(const char* fmt, ...);
    void beginEvent(const char* name);
    void endEvent();

    class ScopedEvent
    {
    public:
        inline ScopedEvent(const char* fmt, ...);
        inline ~ScopedEvent();
    private:
        Trace * const trace;
    };

private:
    Trace(const Trace&) = delete;
    Trace& operator = (const Trace&) = delete;

    struct Event
    {
        enum class Type : uint8_t
        {
            Begin = 'B',
            End = 'E',
            Complete = 'X',
            Instant = 'i',
            Counter = 'C',
            AsyncStart = 'b',
            AsyncInstant = 'n',
            AsyncEnd = 'e',
            FlowStart = 's',
            FlowStep = 't',
            FlowEnd = 'f',
            Sample = 'P',
            ObjectCreated = 'N',
            ObjectSnapshot = 'O',
            ObjectDestroyed = 'D',
            Metadata = 'M',
            GlobalMemoryDump = 'V',
            ProcessMemoryDump = 'v',
            Mark = 'R',
            ClockSync = 'c',
            ContextEnter = '(',
            ContextLeave = ')',

            // Internal types
            Shutdown = 'S',
        };

        Event() : threadID(std::hash<std::thread::id>()(std::this_thread::get_id())) {}
        virtual ~Event() = default;
        virtual Type type() const = 0;
        virtual void write(std::ostream &out) const;

        void setName(const char* name) { strncpy(this->name, name, MaxEventNameLength); }

        char name[MaxEventNameLength] = {};
        const char **categories = nullptr;
        uint64_t timestamp = 0; // in microseconds
        uint32_t processID = 0;
        uint32_t threadID = 0;
    };

    struct BeginEvent    : public Event { Type type() const override { return Type::Begin; } };
    struct EndEvent      : public Event { Type type() const override { return Type::End; } };
    struct MetadataEvent : public Event { Type type() const override { return Type::Metadata; } };
    struct Shutdown      : public Event { Type type() const override { return Type::Shutdown; } };

    struct NameThreadEvent : public MetadataEvent
    {
        void write(std::ostream &out) const override;
    };

    uint64_t timestamp(); // in microseconds

    std::chrono::time_point<std::chrono::high_resolution_clock> createdAt = std::chrono::high_resolution_clock::now();
    Chan<Event*> chan;
    std::thread thread;
};

Trace::ScopedEvent::ScopedEvent(const char* fmt, ...) : trace(Trace::get())
{
    if (trace != nullptr)
    {
        char name[Trace::MaxEventNameLength];
        va_list vararg;
        va_start(vararg, fmt);
        vsnprintf(name, Trace::MaxEventNameLength, fmt, vararg);
        va_end(vararg);

        trace->beginEvent(name);
    }
}

Trace::ScopedEvent::~ScopedEvent()
{
    if (trace != nullptr)
    {
        trace->endEvent();
    }
}

#define SCOPED_EVENT(...) sw::Trace::ScopedEvent scoped_event_##__LINE__(__VA_ARGS__);
#define NAME_THREAD(...) do { if (auto t = Trace::get()) { t->nameThread(__VA_ARGS__); } } while(false);

}  // namespace sw
