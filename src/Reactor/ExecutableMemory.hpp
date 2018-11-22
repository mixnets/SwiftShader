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

#ifndef rr_ExecutableMemory_hpp
#define rr_ExecutableMemory_hpp

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace rr
{
size_t memoryPageSize();

void *allocateExecutable(size_t bytes);   // Allocates memory that can be made executable using markExecutable()
void markExecutable(void *memory, size_t bytes);
void deallocateExecutable(void *memory, size_t bytes);

template<typename T>
T unaligned_read(T *address)
{
	T value;
	memcpy(&value, address, sizeof(T));
	return value;
}

template<typename T, typename V>
void unaligned_write(T *address, V value)
{
	static_assert(sizeof(V) == sizeof(T), "value size must match pointee size");
	memcpy(address, &value, sizeof(T));
}

template<typename T>
struct unaligned_ref
{
	unaligned_ref(T &ref) : ptr(&ref) {}

	T operator=(T value)
	{
		unaligned_write(ptr, value);
		return value;
	}

	void *ptr;
};

template<typename T>
struct unaligned_ptr
{
	unaligned_ptr(T *ptr) : ptr(ptr) {}

	operator T()
	{
		return unaligned_read(ptr);
	}

	void *ptr;
};
}

#endif   // rr_ExecutableMemory_hpp
