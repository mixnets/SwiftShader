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

#ifndef yarn_pool_hpp
#define yarn_pool_hpp

#include "ConditionVariable.hpp"

#include <atomic>
#include <mutex>

namespace yarn {

enum class PoolPolicy
{
	// Call the Pool items constructor on borrow(), and destruct the item
	// when the item is returned.
	Reconstruct,

	// Construct and destruct all items once for the lifetime of the Pool.
	// Items will keep their state between loans.
	Preserve,
};

template <typename T>
class Pool
{
protected:
	struct Item
	{
		using DataStorage = typename std::aligned_storage<sizeof(T), alignof(T)>::type;
		inline T* get();
		inline void construct();
		inline void destruct();
		DataStorage data;
		std::atomic<int> refcount = {0};
		union
		{
			Item *next;
			Pool *pool;
		};
	};

public:

	class Loan
	{
	public:
		inline Loan();
		inline Loan(Item*);
		inline Loan(const Loan&);
		inline Loan(Loan&&);
		inline ~Loan();
		inline Loan& operator = (const Loan&);
		inline Loan& operator = (Loan&&);
		inline T& operator * ();
		inline T* operator -> () const;
		inline T* get() const;

	private:
		void release();
		Item *item;
	};

protected:
	Pool(PoolPolicy policy);
	inline Loan borrow();
	inline std::pair<Loan, bool> tryBorrow();
	inline void return_(Item *data);

	const PoolPolicy policy;
	Item *free = nullptr;
	std::mutex mutex;
	ConditionVariable returned;
};

template <typename T>
T* Pool<T>::Item::get()
{
	return reinterpret_cast<T*>(&data);
}

template <typename T>
void Pool<T>::Item::construct()
{
	new (&data) T();
}

template <typename T>
void Pool<T>::Item::destruct()
{
	get()->~T();
}

template <typename T>
using Loan = typename Pool<T>::Loan;

template <typename T>
Pool<T>::Loan::Loan() : item(nullptr) {}

template <typename T>
Pool<T>::Loan::Loan(Item* item) : item(item) { item->refcount++; }

template <typename T>
Pool<T>::Loan::Loan(const Loan& other) : item(other.item)
{
	if (item != nullptr)
	{
		item->refcount++;
	}
}

template <typename T>
Pool<T>::Loan::Loan(Loan&& other) : item(other.item)
{
	other.item = nullptr;
}

template <typename T>
Pool<T>::Loan::~Loan()
{
	release();
}

template <typename T>
void Pool<T>::Loan::release()
{
	if (item != nullptr)
	{
		auto refs = --item->refcount;
		assert(refs >= 0);
		if (refs == 0)
		{
			item->pool->return_(item);
		}
		item = nullptr;
	}
}

template <typename T>
typename Pool<T>::Loan& Pool<T>::Loan::operator = (const Pool<T>::Loan& rhs)
{
	release();
	if (rhs.item != nullptr)
	{
		item = rhs.item;
		rhs.item->refcount++;
	}
	return *this;
}

template <typename T>
typename Pool<T>::Loan& Pool<T>::Loan::operator = (Pool<T>::Loan&& rhs)
{
	release();
	item = rhs.item;
	rhs.item = nullptr;
	return *this;
}

template <typename T>
T& Pool<T>::Loan::operator * () { return *item->get(); }

template <typename T>
T* Pool<T>::Loan::operator -> () const { return item->get(); }

template <typename T>
T* Pool<T>::Loan::get() const { return item->get(); }

template <typename T>
typename Pool<T>::Loan Pool<T>::borrow()
{
	std::unique_lock<std::mutex> lock(mutex);
	returned.wait(lock, [&] { return free != nullptr; });
	auto item = free;
	free = free->next;
	item->pool = this;
	lock.unlock();
	if (policy == PoolPolicy::Reconstruct)
	{
		item->construct();
	}
	return Loan(item);
}

template <typename T>
std::pair<typename Pool<T>::Loan, bool> Pool<T>::tryBorrow()
{
	std::unique_lock<std::mutex> lock(mutex);
	if (free == nullptr)
	{
		return std::make_pair(Loan(), false);
	}
	auto item = free;
	free = free->next;
	item->pool = this;
	lock.unlock();
	if (policy == PoolPolicy::Reconstruct)
	{
		item->construct();
	}
	return std::make_pair(Loan(item), true);
}

template <typename T>
Pool<T>::Pool(PoolPolicy policy) : policy(policy) {}

template <typename T>
void Pool<T>::return_(Item* item)
{
	if (policy == PoolPolicy::Reconstruct)
	{
		item->destruct();
	}
	std::unique_lock<std::mutex> lock(mutex);
	item->next = free;
	free = item;
	lock.unlock();
	returned.notify_one();
}

////////////////////////////////////////////////////////////////////////////////
// FixedSizePool
////////////////////////////////////////////////////////////////////////////////
template <typename T, int N, PoolPolicy POLICY = PoolPolicy::Reconstruct>
class FixedSizePool : public Pool<T>
{
public:
	using Item = typename Pool<T>::Item;
	using Loan = typename Pool<T>::Loan;

	FixedSizePool();
	~FixedSizePool();
	inline Loan borrow();
	inline std::pair<Loan, bool> tryBorrow();

private:
	Item items[N];
};

template <typename T, int N, PoolPolicy POLICY>
FixedSizePool<T, N, POLICY>::FixedSizePool() : Pool<T>(POLICY)
{
	for (int i = 0; i < N; i++)
	{
		if (POLICY == PoolPolicy::Preserve)
		{
			items[i].construct();
		}
		items[i].next = this->free;
		this->free = &items[i];
	}
}

template <typename T, int N, PoolPolicy POLICY>
FixedSizePool<T, N, POLICY>::~FixedSizePool()
{
	if (POLICY == PoolPolicy::Preserve)
	{
		for (int i = 0; i < N; i++)
		{
			items[i].destruct();
		}
	}
}

template <typename T, int N, PoolPolicy POLICY>
typename FixedSizePool<T, N, POLICY>::Loan FixedSizePool<T, N, POLICY>::borrow()
{
	return Pool<T>::borrow();
}

template <typename T, int N, PoolPolicy POLICY>
std::pair<typename FixedSizePool<T, N, POLICY>::Loan, bool> FixedSizePool<T, N, POLICY>::tryBorrow()
{
	return Pool<T>::tryBorrow();
}

////////////////////////////////////////////////////////////////////////////////
// UnboundedPool
////////////////////////////////////////////////////////////////////////////////
template <typename T, PoolPolicy POLICY = PoolPolicy::Reconstruct>
class UnboundedPool : public Pool<T>
{
public:
	using Item = typename Pool<T>::Item;
	using Loan = typename Pool<T>::Loan;

	inline UnboundedPool();
	inline ~UnboundedPool();
	inline Loan borrow();
private:
	std::vector<Item*> items;
};

template <typename T, PoolPolicy POLICY>
UnboundedPool<T, POLICY>::UnboundedPool() : Pool<T>(POLICY)
{}

template <typename T, PoolPolicy POLICY>
UnboundedPool<T, POLICY>::~UnboundedPool()
{
	for (auto item : items)
	{
		if (POLICY == PoolPolicy::Preserve)
		{
			item->destruct();
		}
		delete item;
	}
}

template <typename T, PoolPolicy POLICY>
typename UnboundedPool<T, POLICY>::Loan UnboundedPool<T, POLICY>::borrow()
{
	std::unique_lock<std::mutex> lock(this->mutex);
	if (this->free == nullptr)
	{
		auto count = std::max<size_t>(items.size(), 32);
		for (size_t i = 0; i < count; i++)
		{
			auto item = new Item();
			if (POLICY == PoolPolicy::Preserve)
			{
				item->construct();
			}
			items.push_back(item);
			item->next = this->free;
			this->free = item;
		}
	}
	auto item = this->free;
	this->free = this->free->next;
	item->pool = this;
	lock.unlock();
	if (POLICY == PoolPolicy::Reconstruct)
	{
		item->construct();
	}
	return Loan(item);
}

} // namespace yarn

#endif  // yarn_pool_hpp
