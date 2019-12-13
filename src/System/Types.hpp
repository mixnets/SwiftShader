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

#ifndef sw_Types_hpp
#define sw_Types_hpp

#include <limits>
#include <type_traits>

// GCC warns against bitfields not fitting the entire range of an enum with a fixed underlying type of unsigned int, which gets promoted to an error with -Werror and cannot be suppressed.
// However, GCC already defaults to using unsigned int as the underlying type of an unscoped enum without a fixed underlying type. So we can just omit it.
#if defined(__GNUC__) && !defined(__clang__)
namespace {enum E {}; static_assert(!std::numeric_limits<std::underlying_type<E>::type>::is_signed, "expected unscoped enum whose underlying type is not fixed to be unsigned");}
#define ENUM_UNDERLYING_TYPE_UNSIGNED_INT
#else
#define ENUM_UNDERLYING_TYPE_UNSIGNED_INT : unsigned int
#endif

#if defined(_MSC_VER)
	typedef signed __int8 int8_t;
	typedef signed __int16 int16_t;
	typedef signed __int32 int32_t;
	typedef signed __int64 int64_t;
	typedef unsigned __int8 uint8_t;
	typedef unsigned __int16 uint16_t;
	typedef unsigned __int32 uint32_t;
	typedef unsigned __int64 uint64_t;
	#define ALIGN(bytes, type) __declspec(align(bytes)) type
#else
	#include <stdint.h>
	#define ALIGN(bytes, type) type __attribute__((aligned(bytes)))
#endif

namespace sw {

typedef ALIGN(1, uint8_t) byte;
typedef ALIGN(2, uint16_t) word;
typedef ALIGN(4, uint32_t) dword;
typedef ALIGN(8, uint64_t) qword;
typedef ALIGN(1, int8_t) sbyte;

template<typename T, int N>
struct alignas(sizeof(T) * N) vec_base
{
	T v[N];
};

template<typename T>
struct alignas(sizeof(T) * 4) vec_base<T, 4>
{
	union
	{
		T v[4];

		struct
		{
			T x;
			T y;
			T z;
			T w;
		};
	};
};

template<typename T, int N>
struct vec : public vec_base<T, N>
{
	using vec_base<T, N>::v;

	vec() = default;

	constexpr vec(T scalar)
	{
		for(int i = 0; i < N; i++)
		{
			v[i] = scalar;
		}
	}

	T& operator[](int i)
	{
		return v[i];
	}

	const T& operator[](int i) const
	{
		return v[i];
	}

	bool operator==(const vec &rhs)
	{
		for(int i = 0; i < N; i++)
		{
			if(v[i] != rhs.v[i])
			{
				return false;
			}
		}

		return true;
	}

	bool operator!=(const vec& rhs)
	{
		return !(*this == rhs);
	}
};

template<typename T>
struct vec4 : public vec<T, 4>
{
	using vec_base<T, 4>::x;
	using vec_base<T, 4>::y;
	using vec_base<T, 4>::z;
	using vec_base<T, 4>::w;

	vec4() = default;

	using vec::vec;

	constexpr vec4(T x, T y, T z, T w)
	{
		this->x = x;
		this->y = y;
		this->z = z;
		this->w = w;
	}
};

using int2 = vec<int, 2>;
using uint2 = vec<unsigned int, 2>;
using float2 = vec<float, 2>;
using dword2 = vec<dword, 2>;
using qword2 = vec<qword, 2>;

using int4 = vec4<int>;
using uint4 = vec4<unsigned int>;
using float4 = vec4<float>;
using byte4 = vec4<byte>;
using sbyte4 = vec4<sbyte>;
using short4 = vec4<short>;
using ushort4 = vec4<unsigned short>;
using word4 = vec4<word>;
using dword4 = vec4<dword>;

using byte8 = vec<byte, 8>;
using sbyte8 = vec<sbyte, 8>;
using short8 = vec<short, 8>;
using ushort8 = vec<unsigned short, 8>;

using byte16 = vec<byte, 16>;
using sbyte16 = vec<sbyte, 16>;

#define OFFSET(s,m) (int)(size_t)&reinterpret_cast<const volatile char&>((((s*)0)->m))

}  // namespace sw

#endif   // sw_Types_hpp
