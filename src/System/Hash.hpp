// Copyright 2020 The SwiftShader Authors. All Rights Reserved.
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

#include <functional>
#include <type_traits>

#include <stdint.h>

namespace sw {

uint64_t FNV_1a(const unsigned char *data, int size);  // Fowler-Noll-Vo hash function

// Hash function for values of type T. The non-specialized form simply forwards
// on to std::hash<T>.
template<typename T, typename ENABLE = void>
struct Hash
{
	template<typename V>
	inline uint64_t operator()(V &&val) const
	{
		return std::hash<T>()(std::forward<V>(val));
	}
};

// Generic hash that simply forwards on to Hash<T>.
template<typename T>
uint64_t hash(T &&val)
{
	return Hash<std::decay_t<T>>()(std::forward<T>(val));
}

// hash() returns a combined hash of all arguments. Order dependent.
template<typename T, typename... OTHERS>
uint64_t hash(T &&val, OTHERS &&... others)
{
	return (hash(std::forward<OTHERS>(others)...) * 1099511628211) ^ hash<T>(std::forward<T>(val));
}

}  // namespace sw

#endif  // sw_Hash_hpp
