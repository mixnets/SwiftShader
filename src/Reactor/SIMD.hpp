// Copyright 2022 The SwiftShader Authors. All Rights Reserved.
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

#ifndef rr_SIMD_hpp
#define rr_SIMD_hpp

#include "Reactor.hpp"

namespace rr::SIMD {

class W
{
public:
	operator int();
	operator RValue<rr::Int>();

	int get();
};

extern W Width;

class Int;
class UInt;
class Float;

class SIMD::Int : public LValue<SIMD::Int>
{
public:
	SIMD::Int(RValue<SIMD::Int> rhs);
	SIMD::Int(const Reference<SIMD::Int> &rhs);

	static Type *type();
};

RValue<SIMD::Int> operator+(RValue<SIMD::Int> lhs, RValue<SIMD::Int> rhs);

}  // namespace rr::SIMD

#endif  // rr_SIMD_hpp
