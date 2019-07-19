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

class TicketQueue;

class Ticket
{
    friend class TicketQueue;

    struct Record
    {
        std::atomic<int> refcount = {0};
        void reference();
        void release();

        std::mutex *listMutex = nullptr;
        Record *next = nullptr; // guarded by listMutex
        Record *prev = nullptr; // guarded by listMutex
        void unlink(); // guarded by listMutex

        Event<EventType::ManualReset> called;
    };

public:
    Ticket(const Ticket& other);
    Ticket(Ticket&& other);
    ~Ticket();

    inline void wait() const;
    inline void done() const;

private:
    Ticket(Record*);
    Record * record;
};

class TicketQueue
{
public:
    inline Ticket take();

private:
	std::mutex listMutex;
    Ticket::Record tail;
};

Ticket TicketQueue::take()
{
    std::unique_lock<std::mutex> lock(listMutex);
    auto record = new Ticket::Record();
    record->listMutex = &listMutex;
    record->prev = tail.prev;
    record->next = &tail;
    tail.prev = record;
    if (record->prev == nullptr)
    {
        record->called.signal();
    }
    else
    {
        record->prev->next = record;
    }
    return record;
}

Ticket::Ticket(Record *record) : record(record)
{
    YARN_ASSERT(record != nullptr, "record is nullptr");
    record->reference();
}

Ticket::Ticket(const Ticket& other) : record(other.record)
{
    record->reference();
}

Ticket::Ticket(Ticket&& other) : record(other.record)
{
    other.record = nullptr;
}

Ticket::~Ticket()
{
    if (record != nullptr)
    {
        record->release();
    }
}

void Ticket::wait() const
{
    record->called.wait();
}

void Ticket::done() const
{
    YARN_ASSERT(record->called, "Calling done() when the ticket hasn't been called");
    std::unique_lock<std::mutex> listLock(*record->listMutex);
    if (record->next != nullptr)
    {
        record->next->called.signal();
        record->unlink();
    }
}


void Ticket::Record::reference()
{
    auto ref = refcount++;
    YARN_ASSERT(ref >= 0, "refcount: %d", ref); (void)ref;
}

void Ticket::Record::release()
{
    auto ref = --refcount;
    YARN_ASSERT(ref >= 0, "refcount: %d", ref);
    if (ref == 0)
    {
        std::unique_lock<std::mutex> lock(*listMutex);
        unlink();
        lock.unlock();
        delete this;
    }
}

void Ticket::Record::unlink()
{
    if (prev == nullptr && next != nullptr)
    {
        next->called.signal();
    }
    if (prev != nullptr) { prev->next = next; }
    if (next != nullptr) { next->prev = prev; }
    prev = nullptr;
    next = nullptr;
}

} // namespace yarn

#endif  // yarn_ticket_queue_hpp
