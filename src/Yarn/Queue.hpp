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

#ifndef yarn_queue_hpp
#define yarn_queue_hpp

#include "ConditionVariable.hpp"
#include "Storage.hpp"

namespace yarn {

// Queue implements a thread-safe FIFO queue.
template <typename STORAGE>
class Queue
{
public:
	using Storage = STORAGE;
	using Item = typename Storage::Item;

	Queue();

	// take returns the next item in the Queue, blocking until an item is
	// available.
	Item take();

	// tryTake returns a <Item, bool> pair.
	// If the Queue is not empty, then the next item and true are returned.
	// If the Queue is empty, then a default-initialized Item and false are returned.
	std::pair<Item, bool> tryTake();

	// put places an item into the Queue, blocking if the Queue is bounded and
	// full.
	void put(const Item &v);

	// Returns the number of items in the Queue.
	// Note: that this may change as soon as the function returns, so should
	// only be used for debugging.
	size_t count();

private:
	STORAGE storage;
	std::mutex mutex;
	ConditionVariable added;
	ConditionVariable removed;
};

template <typename Storage>
Queue<Storage>::Queue() {}

template <typename Storage>
typename Storage::Item Queue<Storage>::take()
{
	std::unique_lock<std::mutex> lock(mutex);
	// If the queue is empty, wait for an item to be added.
	added.wait(lock, [this] { return storage.size() > 0; });
	auto out = storage.take();
	removed.notify_one();
	return out;
}

template <typename Storage>
std::pair<typename Storage::Item, bool> Queue<Storage>::tryTake()
{
	std::unique_lock<std::mutex> lock(mutex);
	if (storage.size() == 0)
	{
		return std::make_pair(Item{}, false);
	}
	auto out = storage.take();
	removed.notify_one();
	return std::make_pair(out, true);
}

template <typename Storage>
void Queue<Storage>::put(const Item &item)
{
	std::unique_lock<std::mutex> lock(mutex);
	removed.wait(lock, [this] { return storage.isFull(); });
	storage.push(item);
	added.notify_one();
}

template <typename Storage>
size_t Queue<Storage>::count()
{
	std::unique_lock<std::mutex> lock(mutex);
	return storage.size();
}

template <typename Item, int N>
using FixedSizeQueue = Queue<FixedSizeStorage<Item, N>>;

} // namespace yarn

#endif  // yarn_queue_hpp
