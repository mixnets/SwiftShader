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

#ifndef yarn_ticket_queue_hpp
#define yarn_ticket_queue_hpp

#include "Event.hpp"
#include "Pool.hpp"
#include "Scheduler.hpp"

namespace yarn {

class Ticket
{
    struct Shared;

    struct Record
    {
        inline ~Record();

        std::shared_ptr<Shared> shared;
        Record *next = nullptr; // guarded by shared->mutex
        Record *prev = nullptr; // guarded by shared->mutex
        inline void unlink(); // guarded by shared->mutex

        inline void done();
        inline void call(std::unique_lock<std::mutex> &lock);

        Task onCall;

        Event<EventType::ManualReset> called;
    };

    struct Shared
    {
        std::mutex mutex;
        Record tail;
    };

public:

    class Queue
    {
    public:
        inline Ticket take();

        template <typename F>
        inline void takeN(size_t n, const F& f);

    private:
		std::shared_ptr<Shared> shared = std::make_shared<Shared>();
        UnboundedPool<Record> pool;
    };

    inline Ticket() = default;
    inline Ticket(const Ticket& other) = default;
    inline Ticket(Ticket&& other) = default;
    inline Ticket& operator = (const Ticket& other) = default;

    inline void wait() const;
    inline void done() const;

    template<typename Function>
    inline void onCall(Function&& f) const;

private:
    inline Ticket(Loan<Record>&& record);
    Loan<Record> record;
};

Ticket Ticket::Queue::take()
{
    Ticket out;
    takeN(1, [&](Ticket&& ticket) { out = std::move(ticket); });
	return out;
}

template <typename F>
void Ticket::Queue::takeN(size_t n, const F& f)
{
    Loan<Record> first, last;
    pool.borrowN(n, [&] (Loan<Record>&& record) {
        Loan<Record> rec = record;
        rec->shared = shared;
        if (first.get() == nullptr)
        {
            first = rec;
        }
        if (last.get() != nullptr)
        {
            last->next = rec.get();
            rec->prev = last.get();
        }
        last = rec;
        f(std::move(Ticket(std::move(rec))));
    });
	std::unique_lock<std::mutex> lock(shared->mutex);
    first->prev = shared->tail.prev;
    last->next = &shared->tail;
    shared->tail.prev = last.get();
    if (first->prev == nullptr)
    {
        first->call(lock);
    }
    else
    {
        first->prev->next = first.get();
    }
}


Ticket::Ticket(Loan<Record>&& record) : record(std::move(record)) {}

void Ticket::wait() const
{
    record->called.wait();
}

void Ticket::done() const
{
    record->done();
}

template<typename Function>
void Ticket::onCall(Function&& f) const
{
    std::unique_lock<std::mutex> lock(record->shared->mutex);
    if (record->called)
    {
        yarn::schedule(std::move(f));
        return;
    }
    if (record->onCall)
    {
		struct Joined
		{
			void operator() () const { a(); b(); }
			Task a, b;
		};
		record->onCall = std::move(Joined{ std::move(record->onCall), std::move(f) });
    }
    else
    {
        record->onCall = std::move(f);
    }
}

Ticket::Record::~Record()
{
    if (shared != nullptr)
    {
        done();
    }
}

void Ticket::Record::done()
{
    std::unique_lock<std::mutex> lock(shared->mutex);
    auto call = (prev == nullptr && next != nullptr) ? next : nullptr;
    unlink();
    if (call != nullptr)
    {
        call->call(lock);
    }
}

void Ticket::Record::call(std::unique_lock<std::mutex> &lock)
{
    called.signal();
    if (onCall)
    {
        yarn::schedule(std::move(onCall));
        onCall = Task();
    }
}

void Ticket::Record::unlink()
{
    if (prev != nullptr) { prev->next = next; }
    if (next != nullptr) { next->prev = prev; }
    prev = nullptr;
    next = nullptr;
}

} // namespace yarn

#endif  // yarn_ticket_queue_hpp
