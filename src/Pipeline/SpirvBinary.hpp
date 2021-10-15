// Copyright 2021 The SwiftShader Authors. All Rights Reserved.
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

#ifndef sw_SpirvBinary_hpp
#define sw_SpirvBinary_hpp

#include <atomic>
#include <cstdint>
#include <vector>

namespace sw {

template<typename T>
class stl_allocator
{
public:
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef T *pointer;
	typedef const T *const_pointer;
	typedef T &reference;
	typedef const T &const_reference;
	typedef T value_type;

	stl_allocator() {}
	~stl_allocator() {}

	template<class U>
	struct rebind
	{
		typedef stl_allocator<U> other;
	};
	template<class U>
	stl_allocator(const stl_allocator<U> &)
	{}

	pointer address(reference x) const { return &x; }
	const_pointer address(const_reference x) const { return &x; }
	size_type max_size() const throw() { return size_t(-1) / sizeof(value_type); }

	pointer allocate(size_type n)
	{
		return static_cast<pointer>(::malloc(n * sizeof(T)));
	}

	void deallocate(pointer p, size_type n)
	{
		::free(p);
	}

	void construct(pointer p, const T &val)
	{
		new(static_cast<void *>(p)) T(val);
	}

	void construct(pointer p)
	{
		new(static_cast<void *>(p)) T();
	}

	void destroy(pointer p)
	{
		p->~T();
	}
};

class SpirvBinary : public std::vector<uint32_t, stl_allocator<uint32_t>>
{
public:
	SpirvBinary()
	    : serialID(serialCounter++)
	{}

	SpirvBinary(const uint32_t *binary, uint32_t wordCount)
	    : std::vector<uint32_t, stl_allocator<uint32_t>>(binary, binary + wordCount, sw::stl_allocator<uint32_t>())
	    , serialID(serialCounter++)
	{
	}

	uint32_t getSerialID() const { return serialID; };

	//private:
	static std::atomic<uint32_t> serialCounter;

	uint32_t serialID;
};

}  // namespace sw

#endif  // sw_SpirvBinary_hpp
