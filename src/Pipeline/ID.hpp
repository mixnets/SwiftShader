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

#ifndef sw_ID_hpp
#define sw_ID_hpp

#include <unordered_map>
#include <cstdint>

namespace sw
{
	// ID is a strongly-typed identifier backed by a numerical value of type I.
	// The template parameter T is not actually used by the implementation of
	// ID; instead it is used to prevent implicit casts between idenfitifers of
	// different T types.
	// IDs are typically used as a map key to value of type T.
	template <typename T, typename I = uint32_t>
	class ID
	{
	public:
		ID() : id(0) {}
		ID(uint32_t id) : id(id) {}
		bool operator == (const ID<T>& rhs) const { return id == rhs.id; }
		bool operator < (const ID<T>& rhs) const { return id < rhs.id; }

		// value returns the numerical value of the identifier.
		I value() const { return id; }
	private:
		I id;
	};

	// HandleMap<T> is an unordered map of ID<T> to T, which each identifier
	// using I as the backing numerical value.
	template <typename T, typename I = uint32_t>
	using HandleMap = std::unordered_map<ID<T>, T>;
}

namespace std
{
	// std::hash implementation for sw::ID<T>
	template<typename T, typename I>
	struct hash< sw::ID<T, I> >
	{
		std::size_t operator()(const sw::ID<T>& id) const noexcept
		{
			return std::hash<I>()(id.value());
		}
	};
}

#endif  // sw_ID_hpp
