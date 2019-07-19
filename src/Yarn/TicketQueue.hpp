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
        inline std::vector<Ticket> takeN(size_t n);

    private:
		inline Ticket takeLocked();
		std::shared_ptr<Shared> shared = std::make_shared<Shared>();
    };

    inline Ticket() = default;
    inline Ticket(const Ticket& other) = default;
    inline Ticket(Ticket&& other) = default;
    inline Ticket& operator = (const Ticket& other) = default;

    inline void wait() const;
    inline void done() const;

private:
    inline Ticket(std::shared_ptr<Record> &record);
    std::shared_ptr<Record> record;
};

Ticket Ticket::Queue::take()
{
    std::unique_lock<std::mutex> lock(shared->mutex);
	return takeLocked();
}

std::vector<Ticket> Ticket::Queue::takeN(size_t n)
{
	std::vector<Ticket> out;
    out.reserve(n);
	std::unique_lock<std::mutex> lock(shared->mutex);
	for (size_t i = 0; i < n; i++)
    {
        out.emplace_back(takeLocked());
    }
    return out;
}

Ticket Ticket::Queue::takeLocked()
{
	auto record = std::make_shared<Record>();
	record->shared = shared;
	record->prev = shared->tail.prev;
	record->next = &shared->tail;
	shared->tail.prev = record.get();
	if (record->prev == nullptr)
	{
		record->called.signal();
	}
	else
	{
		record->prev->next = record.get();
	}
	return Ticket(record);
}

Ticket::Ticket(std::shared_ptr<Record> &record) : record(record) {}

void Ticket::wait() const
{
    record->called.wait();
}

void Ticket::done() const
{
    YARN_ASSERT(record->called, "Calling done() when the ticket hasn't been called");
    std::unique_lock<std::mutex> listLock(record->shared->mutex);
    if (record->next != nullptr)
    {
        record->next->called.signal();
        record->unlink();
    }
}

Ticket::Record::~Record()
{
    if (shared != nullptr)
    {
        std::unique_lock<std::mutex> lock(shared->mutex);
        if (prev == nullptr && next != nullptr)
        {
            next->called.signal();
        }
        unlink();
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
