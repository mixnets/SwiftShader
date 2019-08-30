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

#ifndef sw_Hash_hpp
#define sw_Hash_hpp

#include "Math.hpp"

#include <type_traits>
#include <string>

namespace sw
{

using HashValue = uint64_t;

class ShiftXorCombiner
{
public:
	static inline HashValue combine(std::initializer_list<HashValue> vals)
	{
		HashValue val = 0;
		for (HashValue v : vals)
		{
			val = (val << 3) + (val >> 2) ^ v;
		}
		return val;
	}
};

template<typename Combiner>
class BasicHash
{
public:
	using Hasher = BasicHash<Combiner>;

	template <typename T, typename = typename std::enable_if<std::is_integral<T>::value>::type>
	static inline HashValue hash(T v) { return bit_cast<HashValue>(v); }

	static inline HashValue hash(float v) { return bit_cast<HashValue>(v); }
	static inline HashValue hash(double v) { return bit_cast<HashValue>(v); }
	static inline HashValue hash(const char *str) { return std::hash<const char*>()(str); }
	static inline HashValue hash(const std::string &str) { return std::hash<std::string>()(str); }

	template <typename T, typename = typename std::enable_if<std::is_same<decltype(std::declval<T>().template hash<Hasher>()), HashValue>::value>::type>
	static inline HashValue hash(const T &v) { return v.template hash<Hasher>(); }

	template <typename T0, typename ... Tn>
	static inline HashValue hash(const T0 &v0, const Tn & ... vN)
	{
		return Combiner::combine({hash(v0), hash(vN)...});
	}

	template <typename T>
	std::size_t operator()(const T &v) const noexcept { return hash<T>(v); }
};

using FastHash = BasicHash<ShiftXorCombiner>;

// SW_DECLARE_COMPARABLE() declares an comparison operators and hash
// function for all the member fields passed as macro variadic parameters.
#define SW_DECLARE_COMPARABLE(TYPE, ...)                                       \
	inline auto fields() const -> decltype(std::make_tuple(__VA_ARGS__))       \
	{                                                                          \
		return std::make_tuple(__VA_ARGS__);                                   \
	}                                                                          \
	inline bool operator == (const TYPE& rhs) const                            \
	{                                                                          \
	    return fields() == rhs.fields();                                       \
	}                                                                          \
	inline bool operator != (const TYPE& rhs) const                            \
	{                                                                          \
	    return fields() != rhs.fields();                                       \
	}                                                                          \
	inline bool operator <  (const TYPE& rhs) const                            \
	{                                                                          \
	    return fields() <  rhs.fields();                                       \
	}                                                                          \
	inline bool operator <= (const TYPE& rhs) const                            \
	{                                                                          \
	    return fields() <= rhs.fields();                                       \
	}                                                                          \
	inline bool operator >  (const TYPE& rhs) const                            \
	{                                                                          \
	    return fields() >  rhs.fields();                                       \
	}                                                                          \
	inline bool operator >= (const TYPE& rhs) const                            \
	{                                                                          \
	    return fields() >= rhs.fields();                                       \
	}                                                                          \
	template <typename Hasher> inline sw::HashValue hash() const               \
	{                                                                          \
		return Hasher::hash(__VA_ARGS__);                                      \
	}                                                                          \
	union {} // dummy union so that the closing ')' expects a ';'

}

#endif   // sw_Hash_hpp
