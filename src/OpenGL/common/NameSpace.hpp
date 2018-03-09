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

// NameSpace.h: Defines the NameSpace class, which is used to
// allocate GL object names.

#ifndef gl_NameSpace_hpp
#define gl_NameSpace_hpp

#include "Object.hpp"
#include "debug.h"

#include "Common/MutexLock.hpp"

#include <map>

namespace gl
{

template<class ObjectType>
class ScopedAtomic
{
public:
	ScopedAtomic(sw::MutexLock* l, ObjectType* obj) :
		lock(l), object(obj) {
		if (lock) lock->lock();
	}

	~ScopedAtomic() { if (lock) lock->unlock(); }

	ObjectType* get() { return object; }
	ObjectType* operator->() { return object; }
	operator bool() const { return object != nullptr; }

private:
	sw::MutexLock* lock = nullptr;
	ObjectType* object = nullptr;
};

template<class ObjectType, GLuint baseName = 1>
class NameSpace
{
public:
	NameSpace() : freeName(baseName)
	{
	}

	~NameSpace()
	{
		ASSERT(empty());
	}

	bool empty()
	{
		return map.empty();
	}

	GLuint firstName()
	{
		return map.begin()->first;
	}

	GLuint lastName()
	{
		return map.rbegin()->first;
	}

	GLuint allocate(ObjectType *object = nullptr)
	{
		GLuint name = freeName;

		while(isReserved(name))
		{
			name++;
		}

		map.insert({name, object});
		freeName = name + 1;

		return name;
	}

	bool isReserved(GLuint name) const
	{
		return map.find(name) != map.end();
	}

	void insert(GLuint name, ObjectType *object)
	{
		map[name] = object;

		if(name == freeName)
		{
			freeName++;
		}
	}

	void insertAtomic(GLuint name, ObjectType *object)
	{
		lock();
		insert(name, object);
		unlock();
	}

	ObjectType *remove(GLuint name)
	{
		auto element = map.find(name);

		if(element != map.end())
		{
			ObjectType *object = element->second;
			map.erase(element);

			if(name < freeName)
			{
				freeName = name;
			}

			return object;
		}

		return nullptr;
	}

	ObjectType *removeAtomic(GLuint name)
	{
		lock();
		auto res = remove(name);
		unlock();
		return res;
	}

	ObjectType *find(GLuint name) const
	{
		auto element = map.find(name);

		if(element == map.end())
		{
			return nullptr;
		}

		return element->second;
	}

	ObjectType *findAtomic(GLuint name) const
	{
		lock();
		auto res = find(name);
		unlock();
		return res;
	}

	ScopedAtomic<ObjectType> findScopedAtomic(GLuint name) const
	{
		ObjectType* o = findAtomic(name);
		return ScopedAtomic<ObjectType>(&mapLock, o);
	}

	void lock() const { mapLock.lock(); }
	void unlock() const { mapLock.unlock(); }

private:
	typedef std::map<GLuint, ObjectType*> Map;
	Map map;
	GLuint freeName;   // Lowest known potentially free name

	mutable sw::MutexLock mapLock; // Protects concurrent access
};

}

#endif   // gl_NameSpace_hpp
