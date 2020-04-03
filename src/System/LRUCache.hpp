// Copyright 2016 The SwiftShader Authors. All Rights Reserved.
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

#ifndef sw_LRUCache_hpp
#define sw_LRUCache_hpp

#include "System/Debug.hpp"

#include <unordered_map>
#include <vector>

namespace sw {

template<typename KEY, typename DATA, typename HASH = std::hash<KEY> >
class LRUCache
{
	struct Entry
	{
		KEY key = {};
		DATA data = {};
		Entry *next = nullptr;
		Entry *prev = nullptr;
	};

public:
	using Key = KEY;
	using Data = DATA;
	using Hash = HASH;

	class view
	{
	public:
		inline view(Entry *);
		inline const Key &key() const;
		inline const Data &data() const;

	private:
		Entry *entry;
	};

	class iterator
	{
	public:
		inline iterator(Entry *);
		inline view operator*() const;
		inline iterator &operator++();
		inline bool operator==(const iterator &) const;
		inline bool operator!=(const iterator &) const;

	private:
		Entry *entry;
	};

	inline LRUCache(size_t capacity);
	inline ~LRUCache() = default;

	inline Data get(const Key &key);
	inline void add(const Key &key, const Data &data);

	inline iterator begin() const;
	inline iterator end() const;

protected:
	LRUCache(const LRUCache &) = delete;
	LRUCache(LRUCache &&) = delete;
	LRUCache &operator=(const LRUCache &) = delete;
	LRUCache &operator=(LRUCache &&) = delete;

	inline void unlink(Entry *);
	inline void link(Entry *);

	std::vector<Entry> storage;

	std::unordered_map<Key, Entry *, Hash> map;
	Entry *free = nullptr;
	Entry *head = nullptr;
	Entry *tail = nullptr;
};

}  // namespace sw

namespace sw {

template<typename KEY, typename DATA, typename HASH>
LRUCache<KEY, DATA, HASH>::view::view(Entry *entry)
    : entry(entry)
{}

template<typename KEY, typename DATA, typename HASH>
const KEY &LRUCache<KEY, DATA, HASH>::view::key() const
{
	return entry->key;
}

template<typename KEY, typename DATA, typename HASH>
const DATA &LRUCache<KEY, DATA, HASH>::view::data() const
{
	return entry->data;
}

template<typename KEY, typename DATA, typename HASH>
LRUCache<KEY, DATA, HASH>::iterator::iterator(Entry *entry)
    : entry(entry)
{}

template<typename KEY, typename DATA, typename HASH>
typename LRUCache<KEY, DATA, HASH>::view LRUCache<KEY, DATA, HASH>::iterator::operator*() const
{
	return view{ entry };
}

template<typename KEY, typename DATA, typename HASH>
typename LRUCache<KEY, DATA, HASH>::iterator &LRUCache<KEY, DATA, HASH>::iterator::operator++()
{
	entry = entry->next;
	return *this;
}

template<typename KEY, typename DATA, typename HASH>
bool LRUCache<KEY, DATA, HASH>::iterator::operator==(const iterator &rhs) const
{
	return entry == rhs.entry;
}

template<typename KEY, typename DATA, typename HASH>
bool LRUCache<KEY, DATA, HASH>::iterator::operator!=(const iterator &rhs) const
{
	return entry != rhs.entry;
}

template<typename KEY, typename DATA, typename HASH>
LRUCache<KEY, DATA, HASH>::LRUCache(size_t capacity)
    : storage(capacity)
{
	for(size_t i = 0; i < capacity; i++)
	{
		auto entry = &storage[i];
		entry->next = free;  // No need for back link here.
		free = entry;
	}
}

template<typename KEY, typename DATA, typename HASH>
DATA LRUCache<KEY, DATA, HASH>::get(const Key &key)
{
	auto it = map.find(key);
	if(it == map.end())
	{
		return {};
	}
	// re-link as most recently used.
	auto entry = it->second;
	unlink(entry);
	link(entry);
	return entry->data;
}

template<typename KEY, typename DATA, typename HASH>
void LRUCache<KEY, DATA, HASH>::add(const Key &key, const Data &data)
{
	auto mapIt = map.find(key);
	if(mapIt != map.end())
	{
		// Move entry to front.
		auto entry = mapIt->second;
		unlink(entry);
		link(entry);
		entry->data = data;
		return;
	}

	auto entry = free;
	if(entry)
	{
		// Unlink from free.
		free = entry->next;
		entry->next = nullptr;
	}
	else
	{
		// Unlink least recently used.
		entry = tail;
		unlink(entry);
		map.erase(entry->key);
	}

	// link as most recently used.
	link(entry);
	if(tail == nullptr)
	{
		tail = entry;
	}

	entry->key = key;
	entry->data = data;
	map[key] = entry;
}

template<typename KEY, typename DATA, typename HASH>
typename LRUCache<KEY, DATA, HASH>::iterator LRUCache<KEY, DATA, HASH>::begin() const
{
	return { head };
}

template<typename KEY, typename DATA, typename HASH>
typename LRUCache<KEY, DATA, HASH>::iterator LRUCache<KEY, DATA, HASH>::end() const
{
	return { nullptr };
}

template<typename KEY, typename DATA, typename HASH>
void LRUCache<KEY, DATA, HASH>::unlink(Entry *entry)
{
	if(head == entry) { head = entry->next; }
	if(tail == entry) { tail = entry->prev; }
	if(entry->prev) { entry->prev->next = entry->next; }
	if(entry->next) { entry->next->prev = entry->prev; }
	entry->prev = nullptr;
	entry->next = nullptr;
}

template<typename KEY, typename DATA, typename HASH>
void LRUCache<KEY, DATA, HASH>::link(Entry *entry)
{
	ASSERT_MSG(entry->next == nullptr, "link() called on entry already linked");
	ASSERT_MSG(entry->prev == nullptr, "link() called on entry already linked");
	if(head)
	{
		entry->next = head;
		head->prev = entry;
	}
	head = entry;
	if(!tail) { tail = entry; }
}

}  // namespace sw

#endif  // sw_LRUCache_hpp
