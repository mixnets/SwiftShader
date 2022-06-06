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

#include "SIMD.hpp"

#include "Debug.hpp"

namespace rr::SIMD {

SIMD::Int::Int()
{
}

SIMD::Int::Int(RValue<SIMD::Float> cast)
{
	Value *xyzw = Nucleus::createFPToSI(cast.value(), SIMD::Int::type());

	storeValue(xyzw);
}

SIMD::Int::Int(int broadcast)
{
	int64_t constantVector[16];
	for(auto &x : constantVector) { x = broadcast; }
	storeValue(Nucleus::createConstantVector(constantVector, type()));
}

SIMD::Int::Int(RValue<SIMD::Int> rhs)
{
	store(rhs);
}

SIMD::Int::Int(const SIMD::Int &rhs)
{
	store(rhs.load());
}

SIMD::Int::Int(const Reference<SIMD::Int> &rhs)
{
	store(rhs.load());
}

SIMD::Int::Int(RValue<SIMD::UInt> rhs)
{
	storeValue(rhs.value());
}

SIMD::Int::Int(const SIMD::UInt &rhs)
{
	storeValue(rhs.loadValue());
}

SIMD::Int::Int(const Reference<SIMD::UInt> &rhs)
{
	storeValue(rhs.loadValue());
}

SIMD::Int::Int(const rr::Int &rhs)
{
	*this = RValue<rr::Int>(rhs.loadValue());
}

SIMD::Int::Int(const Reference<rr::Int> &rhs)
{
	*this = RValue<rr::Int>(rhs.loadValue());
}

RValue<SIMD::Int> SIMD::Int::operator=(int x)
{
	return *this = SIMD::Int(x);
}

RValue<SIMD::Int> SIMD::Int::operator=(RValue<SIMD::Int> rhs)
{
	return store(rhs);
}

RValue<SIMD::Int> SIMD::Int::operator=(const SIMD::Int &rhs)
{
	return store(rhs.load());
}

RValue<SIMD::Int> SIMD::Int::operator=(const Reference<SIMD::Int> &rhs)
{
	return store(rhs.load());
}

RValue<SIMD::Int> operator+(RValue<SIMD::Int> lhs, RValue<SIMD::Int> rhs)
{
	return RValue<SIMD::Int>(Nucleus::createAdd(lhs.value(), rhs.value()));
}

RValue<SIMD::Int> operator-(RValue<SIMD::Int> lhs, RValue<SIMD::Int> rhs)
{
	return RValue<SIMD::Int>(Nucleus::createSub(lhs.value(), rhs.value()));
}

RValue<SIMD::Int> operator*(RValue<SIMD::Int> lhs, RValue<SIMD::Int> rhs)
{
	return RValue<SIMD::Int>(Nucleus::createMul(lhs.value(), rhs.value()));
}

RValue<SIMD::Int> operator/(RValue<SIMD::Int> lhs, RValue<SIMD::Int> rhs)
{
	return RValue<SIMD::Int>(Nucleus::createSDiv(lhs.value(), rhs.value()));
}

RValue<SIMD::Int> operator%(RValue<SIMD::Int> lhs, RValue<SIMD::Int> rhs)
{
	return RValue<SIMD::Int>(Nucleus::createSRem(lhs.value(), rhs.value()));
}

RValue<SIMD::Int> operator&(RValue<SIMD::Int> lhs, RValue<SIMD::Int> rhs)
{
	return RValue<SIMD::Int>(Nucleus::createAnd(lhs.value(), rhs.value()));
}

RValue<SIMD::Int> operator|(RValue<SIMD::Int> lhs, RValue<SIMD::Int> rhs)
{
	return RValue<SIMD::Int>(Nucleus::createOr(lhs.value(), rhs.value()));
}

RValue<SIMD::Int> operator^(RValue<SIMD::Int> lhs, RValue<SIMD::Int> rhs)
{
	return RValue<SIMD::Int>(Nucleus::createXor(lhs.value(), rhs.value()));
}

RValue<SIMD::Int> operator<<(RValue<SIMD::Int> lhs, RValue<SIMD::Int> rhs)
{
	return RValue<SIMD::Int>(Nucleus::createShl(lhs.value(), rhs.value()));
}

RValue<SIMD::Int> operator>>(RValue<SIMD::Int> lhs, RValue<SIMD::Int> rhs)
{
	return RValue<SIMD::Int>(Nucleus::createAShr(lhs.value(), rhs.value()));
}

RValue<SIMD::Int> operator+=(SIMD::Int &lhs, RValue<SIMD::Int> rhs)
{
	return lhs = lhs + rhs;
}

RValue<SIMD::Int> operator-=(SIMD::Int &lhs, RValue<SIMD::Int> rhs)
{
	return lhs = lhs - rhs;
}

RValue<SIMD::Int> operator*=(SIMD::Int &lhs, RValue<SIMD::Int> rhs)
{
	return lhs = lhs * rhs;
}

//	RValue<SIMD::Int> operator/=(SIMD::Int &lhs, RValue<SIMD::Int> rhs)
//	{
//		return lhs = lhs / rhs;
//	}

//	RValue<SIMD::Int> operator%=(SIMD::Int &lhs, RValue<SIMD::Int> rhs)
//	{
//		return lhs = lhs % rhs;
//	}

RValue<SIMD::Int> operator&=(SIMD::Int &lhs, RValue<SIMD::Int> rhs)
{
	return lhs = lhs & rhs;
}

RValue<SIMD::Int> operator|=(SIMD::Int &lhs, RValue<SIMD::Int> rhs)
{
	return lhs = lhs | rhs;
}

RValue<SIMD::Int> operator^=(SIMD::Int &lhs, RValue<SIMD::Int> rhs)
{
	return lhs = lhs ^ rhs;
}

RValue<SIMD::Int> operator<<=(SIMD::Int &lhs, unsigned char rhs)
{
	return lhs = lhs << rhs;
}

RValue<SIMD::Int> operator>>=(SIMD::Int &lhs, unsigned char rhs)
{
	return lhs = lhs >> rhs;
}

RValue<SIMD::Int> operator+(RValue<SIMD::Int> val)
{
	return val;
}

RValue<SIMD::Int> operator-(RValue<SIMD::Int> val)
{
	return RValue<SIMD::Int>(Nucleus::createNeg(val.value()));
}

RValue<SIMD::Int> operator~(RValue<SIMD::Int> val)
{
	return RValue<SIMD::Int>(Nucleus::createNot(val.value()));
}

RValue<rr::Int> Extract(RValue<SIMD::Int> x, int i)
{
	return RValue<rr::Int>(Nucleus::createExtractElement(x.value(), rr::Int::type(), i));
}

RValue<SIMD::Int> Insert(RValue<SIMD::Int> x, RValue<rr::Int> element, int i)
{
	return RValue<SIMD::Int>(Nucleus::createInsertElement(x.value(), element.value(), i));
}

SIMD::UInt::UInt()
{
}

SIMD::UInt::UInt(int broadcast)
{
	int64_t constantVector[16];
	for(auto &x : constantVector) { x = broadcast; }
	storeValue(Nucleus::createConstantVector(constantVector, type()));
}

SIMD::UInt::UInt(RValue<SIMD::UInt> rhs)
{
	store(rhs);
}

SIMD::UInt::UInt(const SIMD::UInt &rhs)
{
	store(rhs.load());
}

SIMD::UInt::UInt(const Reference<SIMD::UInt> &rhs)
{
	store(rhs.load());
}

SIMD::UInt::UInt(RValue<SIMD::Int> rhs)
{
	storeValue(rhs.value());
}

SIMD::UInt::UInt(const SIMD::Int &rhs)
{
	storeValue(rhs.loadValue());
}

SIMD::UInt::UInt(const Reference<SIMD::Int> &rhs)
{
	storeValue(rhs.loadValue());
}

SIMD::UInt::UInt(const rr::UInt &rhs)
{
	*this = RValue<rr::UInt>(rhs.loadValue());
}

SIMD::UInt::UInt(const Reference<rr::UInt> &rhs)
{
	*this = RValue<rr::UInt>(rhs.loadValue());
}

RValue<SIMD::UInt> SIMD::UInt::operator=(RValue<SIMD::UInt> rhs)
{
	return store(rhs);
}

RValue<SIMD::UInt> SIMD::UInt::operator=(const SIMD::UInt &rhs)
{
	return store(rhs.load());
}

RValue<SIMD::UInt> SIMD::UInt::operator=(const Reference<SIMD::UInt> &rhs)
{
	return store(rhs.load());
}

RValue<SIMD::UInt> operator+(RValue<SIMD::UInt> lhs, RValue<SIMD::UInt> rhs)
{
	return RValue<SIMD::UInt>(Nucleus::createAdd(lhs.value(), rhs.value()));
}

RValue<SIMD::UInt> operator-(RValue<SIMD::UInt> lhs, RValue<SIMD::UInt> rhs)
{
	return RValue<SIMD::UInt>(Nucleus::createSub(lhs.value(), rhs.value()));
}

RValue<SIMD::UInt> operator*(RValue<SIMD::UInt> lhs, RValue<SIMD::UInt> rhs)
{
	return RValue<SIMD::UInt>(Nucleus::createMul(lhs.value(), rhs.value()));
}

RValue<SIMD::UInt> operator/(RValue<SIMD::UInt> lhs, RValue<SIMD::UInt> rhs)
{
	return RValue<SIMD::UInt>(Nucleus::createUDiv(lhs.value(), rhs.value()));
}

RValue<SIMD::UInt> operator%(RValue<SIMD::UInt> lhs, RValue<SIMD::UInt> rhs)
{
	return RValue<SIMD::UInt>(Nucleus::createURem(lhs.value(), rhs.value()));
}

RValue<SIMD::UInt> operator&(RValue<SIMD::UInt> lhs, RValue<SIMD::UInt> rhs)
{
	return RValue<SIMD::UInt>(Nucleus::createAnd(lhs.value(), rhs.value()));
}

RValue<SIMD::UInt> operator|(RValue<SIMD::UInt> lhs, RValue<SIMD::UInt> rhs)
{
	return RValue<SIMD::UInt>(Nucleus::createOr(lhs.value(), rhs.value()));
}

RValue<SIMD::UInt> operator^(RValue<SIMD::UInt> lhs, RValue<SIMD::UInt> rhs)
{
	return RValue<SIMD::UInt>(Nucleus::createXor(lhs.value(), rhs.value()));
}

RValue<SIMD::UInt> operator<<(RValue<SIMD::UInt> lhs, RValue<SIMD::UInt> rhs)
{
	return RValue<SIMD::UInt>(Nucleus::createShl(lhs.value(), rhs.value()));
}

RValue<SIMD::UInt> operator>>(RValue<SIMD::UInt> lhs, RValue<SIMD::UInt> rhs)
{
	return RValue<SIMD::UInt>(Nucleus::createLShr(lhs.value(), rhs.value()));
}

RValue<SIMD::UInt> operator+=(SIMD::UInt &lhs, RValue<SIMD::UInt> rhs)
{
	return lhs = lhs + rhs;
}

RValue<SIMD::UInt> operator-=(SIMD::UInt &lhs, RValue<SIMD::UInt> rhs)
{
	return lhs = lhs - rhs;
}

RValue<SIMD::UInt> operator*=(SIMD::UInt &lhs, RValue<SIMD::UInt> rhs)
{
	return lhs = lhs * rhs;
}

//	RValue<SIMD::UInt> operator/=(SIMD::UInt &lhs, RValue<SIMD::UInt> rhs)
//	{
//		return lhs = lhs / rhs;
//	}

//	RValue<SIMD::UInt> operator%=(SIMD::UInt &lhs, RValue<SIMD::UInt> rhs)
//	{
//		return lhs = lhs % rhs;
//	}

RValue<SIMD::UInt> operator&=(SIMD::UInt &lhs, RValue<SIMD::UInt> rhs)
{
	return lhs = lhs & rhs;
}

RValue<SIMD::UInt> operator|=(SIMD::UInt &lhs, RValue<SIMD::UInt> rhs)
{
	return lhs = lhs | rhs;
}

RValue<SIMD::UInt> operator^=(SIMD::UInt &lhs, RValue<SIMD::UInt> rhs)
{
	return lhs = lhs ^ rhs;
}

RValue<SIMD::UInt> operator<<=(SIMD::UInt &lhs, unsigned char rhs)
{
	return lhs = lhs << rhs;
}

RValue<SIMD::UInt> operator>>=(SIMD::UInt &lhs, unsigned char rhs)
{
	return lhs = lhs >> rhs;
}

RValue<SIMD::UInt> operator+(RValue<SIMD::UInt> val)
{
	return val;
}

RValue<SIMD::UInt> operator-(RValue<SIMD::UInt> val)
{
	return RValue<SIMD::UInt>(Nucleus::createNeg(val.value()));
}

RValue<SIMD::UInt> operator~(RValue<SIMD::UInt> val)
{
	return RValue<SIMD::UInt>(Nucleus::createNot(val.value()));
}

RValue<rr::UInt> Extract(RValue<SIMD::UInt> x, int i)
{
	return RValue<rr::UInt>(Nucleus::createExtractElement(x.value(), rr::Int::type(), i));
}

RValue<SIMD::UInt> Insert(RValue<SIMD::UInt> x, RValue<rr::UInt> element, int i)
{
	return RValue<SIMD::UInt>(Nucleus::createInsertElement(x.value(), element.value(), i));
}

SIMD::Float::Float(RValue<SIMD::Int> cast)
{
	Value *xyzw = Nucleus::createSIToFP(cast.value(), SIMD::Float::type());

	storeValue(xyzw);
}

SIMD::Float::Float(RValue<SIMD::UInt> cast)
{
	RValue<SIMD::Float> result = SIMD::Float(SIMD::Int(cast & SIMD::UInt(0x7FFFFFFF))) +
	                             As<SIMD::Float>((As<SIMD::Int>(cast) >> 31) & As<SIMD::Int>(SIMD::Float(0x80000000u)));

	storeValue(result.value());
}

SIMD::Float::Float()
{
}

SIMD::Float::Float(float broadcast)
{
	// See rr::Float(float) constructor for the rationale behind this assert.
	ASSERT(std::isfinite(broadcast));

	double constantVector[16];
	for(auto &x : constantVector) { x = broadcast; }
	storeValue(Nucleus::createConstantVector(constantVector, type()));
}

SIMD::Float SIMD::Float::infinity()
{
	SIMD::Float result;

	constexpr double inf = std::numeric_limits<double>::infinity();
	double constantVector[4] = { inf, inf, inf, inf };
	result.storeValue(Nucleus::createConstantVector(constantVector, type()));

	return result;
}

SIMD::Float::Float(RValue<SIMD::Float> rhs)
{
	store(rhs);
}

SIMD::Float::Float(const SIMD::Float &rhs)
{
	store(rhs.load());
}

SIMD::Float::Float(const Reference<SIMD::Float> &rhs)
{
	store(rhs.load());
}

SIMD::Float::Float(const rr::Float &rhs)
{
	*this = RValue<rr::Float>(rhs.loadValue());
}

SIMD::Float::Float(const Reference<rr::Float> &rhs)
{
	*this = RValue<rr::Float>(rhs.loadValue());
}

RValue<SIMD::Float> SIMD::Float::operator=(float x)
{
	return *this = SIMD::Float(x);
}

RValue<SIMD::Float> SIMD::Float::operator=(RValue<SIMD::Float> rhs)
{
	return store(rhs);
}

RValue<SIMD::Float> SIMD::Float::operator=(const SIMD::Float &rhs)
{
	return store(rhs.load());
}

RValue<SIMD::Float> SIMD::Float::operator=(const Reference<SIMD::Float> &rhs)
{
	return store(rhs.load());
}

RValue<SIMD::Float> SIMD::Float::operator=(RValue<rr::Float> rhs)
{
	return *this = SIMD::Float(rhs);
}

RValue<SIMD::Float> SIMD::Float::operator=(const rr::Float &rhs)
{
	return *this = SIMD::Float(rhs);
}

RValue<SIMD::Float> SIMD::Float::operator=(const Reference<rr::Float> &rhs)
{
	return *this = SIMD::Float(rhs);
}

RValue<SIMD::Float> operator+(RValue<SIMD::Float> lhs, RValue<SIMD::Float> rhs)
{
	return RValue<SIMD::Float>(Nucleus::createFAdd(lhs.value(), rhs.value()));
}

RValue<SIMD::Float> operator-(RValue<SIMD::Float> lhs, RValue<SIMD::Float> rhs)
{
	return RValue<SIMD::Float>(Nucleus::createFSub(lhs.value(), rhs.value()));
}

RValue<SIMD::Float> operator*(RValue<SIMD::Float> lhs, RValue<SIMD::Float> rhs)
{
	return RValue<SIMD::Float>(Nucleus::createFMul(lhs.value(), rhs.value()));
}

RValue<SIMD::Float> operator/(RValue<SIMD::Float> lhs, RValue<SIMD::Float> rhs)
{
	return RValue<SIMD::Float>(Nucleus::createFDiv(lhs.value(), rhs.value()));
}

RValue<SIMD::Float> operator%(RValue<SIMD::Float> lhs, RValue<SIMD::Float> rhs)
{
	return RValue<SIMD::Float>(Nucleus::createFRem(lhs.value(), rhs.value()));
}

RValue<SIMD::Float> operator+=(SIMD::Float &lhs, RValue<SIMD::Float> rhs)
{
	return lhs = lhs + rhs;
}

RValue<SIMD::Float> operator-=(SIMD::Float &lhs, RValue<SIMD::Float> rhs)
{
	return lhs = lhs - rhs;
}

RValue<SIMD::Float> operator*=(SIMD::Float &lhs, RValue<SIMD::Float> rhs)
{
	return lhs = lhs * rhs;
}

RValue<SIMD::Float> operator/=(SIMD::Float &lhs, RValue<SIMD::Float> rhs)
{
	return lhs = lhs / rhs;
}

RValue<SIMD::Float> operator%=(SIMD::Float &lhs, RValue<SIMD::Float> rhs)
{
	return lhs = lhs % rhs;
}

RValue<SIMD::Float> operator+(RValue<SIMD::Float> val)
{
	return val;
}

RValue<SIMD::Float> operator-(RValue<SIMD::Float> val)
{
	return RValue<SIMD::Float>(Nucleus::createFNeg(val.value()));
}

RValue<SIMD::Float> Insert(RValue<SIMD::Float> x, RValue<rr::Float> element, int i)
{
	return RValue<SIMD::Float>(Nucleus::createInsertElement(x.value(), element.value(), i));
}

RValue<rr::Float> Extract(RValue<SIMD::Float> x, int i)
{
	return RValue<rr::Float>(Nucleus::createExtractElement(x.value(), rr::Float::type(), i));
}

RValue<SIMD::Int> IsInf(RValue<SIMD::Float> x)
{
	return CmpEQ(As<SIMD::Int>(x) & SIMD::Int(0x7FFFFFFF), SIMD::Int(0x7F800000));
}

RValue<SIMD::Int> IsNan(RValue<SIMD::Float> x)
{
	return ~CmpEQ(x, x);
}

}  // namespace rr::SIMD
