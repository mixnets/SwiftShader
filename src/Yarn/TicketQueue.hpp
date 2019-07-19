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

#include "ConditionVariable.hpp"

namespace yarn {

class TicketQueue;

class Ticket
{
public:
    Ticket(const Ticket&) = default;

    inline void wait() const;
    inline void done() const;

private:
    friend TicketQueue;
    inline Ticket(TicketQueue* queue, uint64_t id);

    TicketQueue * const queue = nullptr;
    const uint64_t id;
};

class TicketQueue
{
public:
    inline Ticket take();

private:
    friend Ticket;

	std::mutex mutex;
    std::deque<uint64_t> pending;
    uint64_t nextFree = 0;
    ConditionVariable call;
};

Ticket TicketQueue::take()
{
    std::unique_lock<std::mutex> lock(mutex);
    auto id = nextFree;
    nextFree++;
    pending.push_back(id);
    return Ticket(this, id);
}

Ticket::Ticket(TicketQueue* queue, uint64_t id) : queue(queue), id(id) {}

void Ticket::wait() const
{
    std::unique_lock<std::mutex> lock(queue->mutex);
    queue->call.wait(lock, [this] { return queue->pending.front() == id; });
}

void Ticket::done() const
{
    std::unique_lock<std::mutex> lock(queue->mutex);
    auto it = std::find(queue->pending.begin(), queue->pending.end(), id);
    if (it != queue->pending.end())
    {
        queue->pending.erase(it);
        lock.unlock();
        queue->call.notify_all();
    }
}

} // namespace yarn

#endif  // yarn_ticket_queue_hpp
