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

#include <chrono>
#include <ostream>
#include <thread>
#include <queue>

namespace sw
{

// Chan is a thread-safe FIFO queue of type T.
// Chan takes its name after Golang's chan.
template <typename T>
class Chan
{
public:
	Chan();

	// take returns the next item in the chan, blocking until an item is
	// available.
	T take();

	// tryTake returns a <T, bool> pair.
	// If the chan is not empty, then the next item and true are returned.
	// If the chan is empty, then a default-initialized T and false are returned.
	std::pair<T, bool> tryTake();

	// put places an item into the chan, blocking if the chan is bounded and
	// full.
	void put(const T &v);

	// Returns the number of items in the chan.
	// Note: that this may change as soon as the function returns, so should
	// only be used for debugging.
	size_t count();

private:
	std::queue<T> queue;
	std::mutex mutex;
	std::condition_variable added;
};

template <typename T>
Chan<T>::Chan() {}

template <typename T>
T Chan<T>::take()
{
	std::unique_lock<std::mutex> lock(mutex);
	// Wait for item to be added.
	added.wait(lock, [this] { return queue.size() > 0; });
	T out = queue.front();
	queue.pop();
	return out;
}

template <typename T>
std::pair<T, bool> Chan<T>::tryTake()
{
	std::unique_lock<std::mutex> lock(mutex);
	if (queue.size() == 0)
	{
		return std::make_pair(T{}, false);
	}
	T out = queue.front();
	queue.pop();
	return std::make_pair(out, true);
}

template <typename T>
void Chan<T>::put(const T &item)
{
	std::unique_lock<std::mutex> lock(mutex);
	queue.push(item);
	added.notify_one();
}

template <typename T>
size_t Chan<T>::count()
{
	std::unique_lock<std::mutex> lock(mutex);
	return queue.size();
}

class Trace
{
public:
    static constexpr size_t MaxEventNameLength = 64;

    Trace(std::ostream &out);
    ~Trace();

    static Trace* get();
    static void set(Trace *);

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
            Shutdown = 'S'
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

    struct BeginEvent : public Event { Type type() const override { return Type::Begin; } };
    struct EndEvent   : public Event { Type type() const override { return Type::End; } };
    struct Shutdown   : public Event { Type type() const override { return Type::Shutdown; } };

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

}  // namespace sw
