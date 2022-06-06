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

extern const int Width;

class Int;
class UInt;
class Float;

class SIMD::Int : public LValue<SIMD::Int>
{
public:
	explicit SIMD::Int(RValue<SIMD::Float> cast);

	SIMD::Int();
	SIMD::Int(int broadcast);
	SIMD::Int(RValue<SIMD::Int> rhs);
	SIMD::Int(const SIMD::Int &rhs);
	SIMD::Int(const Reference<SIMD::Int> &rhs);
	SIMD::Int(RValue<SIMD::UInt> rhs);
	SIMD::Int(const SIMD::UInt &rhs);
	SIMD::Int(const Reference<SIMD::UInt> &rhs);
	SIMD::Int(RValue<rr::Int2> lo, RValue<rr::Int2> hi);
	SIMD::Int(RValue<rr::Int> rhs);
	SIMD::Int(const rr::Int &rhs);
	SIMD::Int(const Reference<rr::Int> &rhs);

	RValue<SIMD::Int> operator=(int broadcast);
	RValue<SIMD::Int> operator=(RValue<SIMD::Int> rhs);
	RValue<SIMD::Int> operator=(const SIMD::Int &rhs);
	RValue<SIMD::Int> operator=(const Reference<SIMD::Int> &rhs);

	static Type *type();
};

RValue<SIMD::Int> operator+(RValue<SIMD::Int> lhs, RValue<SIMD::Int> rhs);
RValue<SIMD::Int> operator-(RValue<SIMD::Int> lhs, RValue<SIMD::Int> rhs);
RValue<SIMD::Int> operator*(RValue<SIMD::Int> lhs, RValue<SIMD::Int> rhs);
RValue<SIMD::Int> operator/(RValue<SIMD::Int> lhs, RValue<SIMD::Int> rhs);
RValue<SIMD::Int> operator%(RValue<SIMD::Int> lhs, RValue<SIMD::Int> rhs);
RValue<SIMD::Int> operator&(RValue<SIMD::Int> lhs, RValue<SIMD::Int> rhs);
RValue<SIMD::Int> operator|(RValue<SIMD::Int> lhs, RValue<SIMD::Int> rhs);
RValue<SIMD::Int> operator^(RValue<SIMD::Int> lhs, RValue<SIMD::Int> rhs);
RValue<SIMD::Int> operator<<(RValue<SIMD::Int> lhs, unsigned char rhs);
RValue<SIMD::Int> operator>>(RValue<SIMD::Int> lhs, unsigned char rhs);
RValue<SIMD::Int> operator<<(RValue<SIMD::Int> lhs, RValue<SIMD::Int> rhs);
RValue<SIMD::Int> operator>>(RValue<SIMD::Int> lhs, RValue<SIMD::Int> rhs);
RValue<SIMD::Int> operator+=(SIMD::Int &lhs, RValue<SIMD::Int> rhs);
RValue<SIMD::Int> operator-=(SIMD::Int &lhs, RValue<SIMD::Int> rhs);
RValue<SIMD::Int> operator*=(SIMD::Int &lhs, RValue<SIMD::Int> rhs);
//	RValue<SIMD::Int> operator/=(SIMD::Int &lhs, RValue<SIMD::Int> rhs);
//	RValue<SIMD::Int> operator%=(SIMD::Int &lhs, RValue<SIMD::Int> rhs);
RValue<SIMD::Int> operator&=(SIMD::Int &lhs, RValue<SIMD::Int> rhs);
RValue<SIMD::Int> operator|=(SIMD::Int &lhs, RValue<SIMD::Int> rhs);
RValue<SIMD::Int> operator^=(SIMD::Int &lhs, RValue<SIMD::Int> rhs);
RValue<SIMD::Int> operator<<=(SIMD::Int &lhs, unsigned char rhs);
RValue<SIMD::Int> operator>>=(SIMD::Int &lhs, unsigned char rhs);
RValue<SIMD::Int> operator+(RValue<SIMD::Int> val);
RValue<SIMD::Int> operator-(RValue<SIMD::Int> val);
RValue<SIMD::Int> operator~(RValue<SIMD::Int> val);
//	RValue<SIMD::Int> operator++(SIMD::Int &val, int);   // Post-increment
//	const SIMD::Int &operator++(SIMD::Int &val);   // Pre-increment
//	RValue<SIMD::Int> operator--(SIMD::Int &val, int);   // Post-decrement
//	const SIMD::Int &operator--(SIMD::Int &val);   // Pre-decrement
//	RValue<Bool> operator<(RValue<SIMD::Int> lhs, RValue<SIMD::Int> rhs);
//	RValue<Bool> operator<=(RValue<SIMD::Int> lhs, RValue<SIMD::Int> rhs);
//	RValue<Bool> operator>(RValue<SIMD::Int> lhs, RValue<SIMD::Int> rhs);
//	RValue<Bool> operator>=(RValue<SIMD::Int> lhs, RValue<SIMD::Int> rhs);
//	RValue<Bool> operator!=(RValue<SIMD::Int> lhs, RValue<SIMD::Int> rhs);
//	RValue<Bool> operator==(RValue<SIMD::Int> lhs, RValue<SIMD::Int> rhs);

RValue<SIMD::Int> CmpEQ(RValue<SIMD::Int> x, RValue<SIMD::Int> y);
RValue<SIMD::Int> CmpLT(RValue<SIMD::Int> x, RValue<SIMD::Int> y);
RValue<SIMD::Int> CmpLE(RValue<SIMD::Int> x, RValue<SIMD::Int> y);
RValue<SIMD::Int> CmpNEQ(RValue<SIMD::Int> x, RValue<SIMD::Int> y);
RValue<SIMD::Int> CmpNLT(RValue<SIMD::Int> x, RValue<SIMD::Int> y);
RValue<SIMD::Int> CmpNLE(RValue<SIMD::Int> x, RValue<SIMD::Int> y);
inline RValue<SIMD::Int> CmpGT(RValue<SIMD::Int> x, RValue<SIMD::Int> y)
{
	return CmpNLE(x, y);
}
inline RValue<SIMD::Int> CmpGE(RValue<SIMD::Int> x, RValue<SIMD::Int> y)
{
	return CmpNLT(x, y);
}
RValue<SIMD::Int> Max(RValue<SIMD::Int> x, RValue<SIMD::Int> y);
RValue<SIMD::Int> Min(RValue<SIMD::Int> x, RValue<SIMD::Int> y);
// Convert to nearest integer. If a converted value is outside of the integer
// range, the returned result is undefined.
RValue<SIMD::Int> RoundInt(RValue<SIMD::Float> cast);
// Rounds to the nearest integer, but clamps very large values to an
// implementation-dependent range.
// Specifically, on x86, values larger than 2147483583.0 are converted to
// 2147483583 (0x7FFFFFBF) instead of producing 0x80000000.
RValue<SIMD::Int> RoundIntClamped(RValue<SIMD::Float> cast);
RValue<Short8> PackSigned(RValue<SIMD::Int> x, RValue<SIMD::Int> y);
RValue<UShort8> PackUnsigned(RValue<SIMD::Int> x, RValue<SIMD::Int> y);
RValue<rr::Int> Extract(RValue<SIMD::Int> val, int i);
RValue<SIMD::Int> Insert(RValue<SIMD::Int> val, RValue<rr::Int> element, int i);

class SIMD::UInt : public LValue<SIMD::UInt>
{
public:
	explicit SIMD::UInt(RValue<SIMD::Float> cast);

	SIMD::UInt();
	SIMD::UInt(int broadcast);
	SIMD::UInt(RValue<SIMD::UInt> rhs);
	SIMD::UInt(const SIMD::UInt &rhs);
	SIMD::UInt(const Reference<SIMD::UInt> &rhs);
	SIMD::UInt(RValue<SIMD::Int> rhs);
	SIMD::UInt(const SIMD::Int &rhs);
	SIMD::UInt(const Reference<SIMD::Int> &rhs);
	SIMD::UInt(RValue<rr::UInt2> lo, RValue<rr::UInt2> hi);
	SIMD::UInt(RValue<rr::UInt> rhs);
	SIMD::UInt(const rr::UInt &rhs);
	SIMD::UInt(const Reference<rr::UInt> &rhs);

	RValue<SIMD::UInt> operator=(RValue<SIMD::UInt> rhs);
	RValue<SIMD::UInt> operator=(const SIMD::UInt &rhs);
	RValue<SIMD::UInt> operator=(const Reference<SIMD::UInt> &rhs);

	static Type *type();
};

RValue<SIMD::UInt> operator+(RValue<SIMD::UInt> lhs, RValue<SIMD::UInt> rhs);
RValue<SIMD::UInt> operator-(RValue<SIMD::UInt> lhs, RValue<SIMD::UInt> rhs);
RValue<SIMD::UInt> operator*(RValue<SIMD::UInt> lhs, RValue<SIMD::UInt> rhs);
RValue<SIMD::UInt> operator/(RValue<SIMD::UInt> lhs, RValue<SIMD::UInt> rhs);
RValue<SIMD::UInt> operator%(RValue<SIMD::UInt> lhs, RValue<SIMD::UInt> rhs);
RValue<SIMD::UInt> operator&(RValue<SIMD::UInt> lhs, RValue<SIMD::UInt> rhs);
RValue<SIMD::UInt> operator|(RValue<SIMD::UInt> lhs, RValue<SIMD::UInt> rhs);
RValue<SIMD::UInt> operator^(RValue<SIMD::UInt> lhs, RValue<SIMD::UInt> rhs);
RValue<SIMD::UInt> operator<<(RValue<SIMD::UInt> lhs, unsigned char rhs);
RValue<SIMD::UInt> operator>>(RValue<SIMD::UInt> lhs, unsigned char rhs);
RValue<SIMD::UInt> operator<<(RValue<SIMD::UInt> lhs, RValue<SIMD::UInt> rhs);
RValue<SIMD::UInt> operator>>(RValue<SIMD::UInt> lhs, RValue<SIMD::UInt> rhs);
RValue<SIMD::UInt> operator+=(SIMD::UInt &lhs, RValue<SIMD::UInt> rhs);
RValue<SIMD::UInt> operator-=(SIMD::UInt &lhs, RValue<SIMD::UInt> rhs);
RValue<SIMD::UInt> operator*=(SIMD::UInt &lhs, RValue<SIMD::UInt> rhs);
//	RValue<SIMD::UInt> operator/=(SIMD::UInt &lhs, RValue<SIMD::UInt> rhs);
//	RValue<SIMD::UInt> operator%=(SIMD::UInt &lhs, RValue<SIMD::UInt> rhs);
RValue<SIMD::UInt> operator&=(SIMD::UInt &lhs, RValue<SIMD::UInt> rhs);
RValue<SIMD::UInt> operator|=(SIMD::UInt &lhs, RValue<SIMD::UInt> rhs);
RValue<SIMD::UInt> operator^=(SIMD::UInt &lhs, RValue<SIMD::UInt> rhs);
RValue<SIMD::UInt> operator<<=(SIMD::UInt &lhs, unsigned char rhs);
RValue<SIMD::UInt> operator>>=(SIMD::UInt &lhs, unsigned char rhs);
RValue<SIMD::UInt> operator+(RValue<SIMD::UInt> val);
RValue<SIMD::UInt> operator-(RValue<SIMD::UInt> val);
RValue<SIMD::UInt> operator~(RValue<SIMD::UInt> val);
//	RValue<SIMD::UInt> operator++(SIMD::UInt &val, int);   // Post-increment
//	const SIMD::UInt &operator++(SIMD::UInt &val);   // Pre-increment
//	RValue<SIMD::UInt> operator--(SIMD::UInt &val, int);   // Post-decrement
//	const SIMD::UInt &operator--(SIMD::UInt &val);   // Pre-decrement
//	RValue<Bool> operator<(RValue<SIMD::UInt> lhs, RValue<SIMD::UInt> rhs);
//	RValue<Bool> operator<=(RValue<SIMD::UInt> lhs, RValue<SIMD::UInt> rhs);
//	RValue<Bool> operator>(RValue<SIMD::UInt> lhs, RValue<SIMD::UInt> rhs);
//	RValue<Bool> operator>=(RValue<SIMD::UInt> lhs, RValue<SIMD::UInt> rhs);
//	RValue<Bool> operator!=(RValue<SIMD::UInt> lhs, RValue<SIMD::UInt> rhs);
//	RValue<Bool> operator==(RValue<SIMD::UInt> lhs, RValue<SIMD::UInt> rhs);

RValue<SIMD::UInt> CmpEQ(RValue<SIMD::UInt> x, RValue<SIMD::UInt> y);
RValue<SIMD::UInt> CmpLT(RValue<SIMD::UInt> x, RValue<SIMD::UInt> y);
RValue<SIMD::UInt> CmpLE(RValue<SIMD::UInt> x, RValue<SIMD::UInt> y);
RValue<SIMD::UInt> CmpNEQ(RValue<SIMD::UInt> x, RValue<SIMD::UInt> y);
RValue<SIMD::UInt> CmpNLT(RValue<SIMD::UInt> x, RValue<SIMD::UInt> y);
RValue<SIMD::UInt> CmpNLE(RValue<SIMD::UInt> x, RValue<SIMD::UInt> y);
inline RValue<SIMD::UInt> CmpGT(RValue<SIMD::UInt> x, RValue<SIMD::UInt> y)
{
	return CmpNLE(x, y);
}
inline RValue<SIMD::UInt> CmpGE(RValue<SIMD::UInt> x, RValue<SIMD::UInt> y)
{
	return CmpNLT(x, y);
}
RValue<SIMD::UInt> Max(RValue<SIMD::UInt> x, RValue<SIMD::UInt> y);
RValue<SIMD::UInt> Min(RValue<SIMD::UInt> x, RValue<SIMD::UInt> y);
RValue<rr::UInt> Extract(RValue<SIMD::UInt> val, int i);
RValue<SIMD::UInt> Insert(RValue<SIMD::UInt> val, RValue<rr::UInt> element, int i);
//	RValue<SIMD::UInt> RoundInt(RValue<SIMD::Float> cast);

class SIMD::Float : public LValue<SIMD::Float>
{
public:
	explicit SIMD::Float(RValue<SIMD::Int> cast);
	explicit SIMD::Float(RValue<SIMD::UInt> cast);

	SIMD::Float();
	SIMD::Float(float broadcast);
	SIMD::Float(RValue<SIMD::Float> rhs);
	SIMD::Float(const SIMD::Float &rhs);
	SIMD::Float(const Reference<SIMD::Float> &rhs);
	SIMD::Float(RValue<rr::Float> rhs);
	SIMD::Float(const rr::Float &rhs);
	SIMD::Float(const Reference<rr::Float> &rhs);

	RValue<SIMD::Float> operator=(float broadcast);
	RValue<SIMD::Float> operator=(RValue<SIMD::Float> rhs);
	RValue<SIMD::Float> operator=(const SIMD::Float &rhs);
	RValue<SIMD::Float> operator=(const Reference<SIMD::Float> &rhs);
	RValue<SIMD::Float> operator=(RValue<rr::Float> rhs);
	RValue<SIMD::Float> operator=(const rr::Float &rhs);
	RValue<SIMD::Float> operator=(const Reference<rr::Float> &rhs);

	static SIMD::Float infinity();

	static Type *type();
};

RValue<SIMD::Float> operator+(RValue<SIMD::Float> lhs, RValue<SIMD::Float> rhs);
RValue<SIMD::Float> operator-(RValue<SIMD::Float> lhs, RValue<SIMD::Float> rhs);
RValue<SIMD::Float> operator*(RValue<SIMD::Float> lhs, RValue<SIMD::Float> rhs);
RValue<SIMD::Float> operator/(RValue<SIMD::Float> lhs, RValue<SIMD::Float> rhs);
RValue<SIMD::Float> operator%(RValue<SIMD::Float> lhs, RValue<SIMD::Float> rhs);
RValue<SIMD::Float> operator+=(SIMD::Float &lhs, RValue<SIMD::Float> rhs);
RValue<SIMD::Float> operator-=(SIMD::Float &lhs, RValue<SIMD::Float> rhs);
RValue<SIMD::Float> operator*=(SIMD::Float &lhs, RValue<SIMD::Float> rhs);
RValue<SIMD::Float> operator/=(SIMD::Float &lhs, RValue<SIMD::Float> rhs);
RValue<SIMD::Float> operator%=(SIMD::Float &lhs, RValue<SIMD::Float> rhs);
RValue<SIMD::Float> operator+(RValue<SIMD::Float> val);
RValue<SIMD::Float> operator-(RValue<SIMD::Float> val);

// Computes `x * y + z`, which may be fused into one operation to produce a higher-precision result.
RValue<SIMD::Float> MulAdd(RValue<SIMD::Float> x, RValue<SIMD::Float> y, RValue<SIMD::Float> z);
// Computes a fused `x * y + z` operation. Caps::fmaIsFast indicates whether it emits an FMA instruction.
RValue<SIMD::Float> FMA(RValue<SIMD::Float> x, RValue<SIMD::Float> y, RValue<SIMD::Float> z);

RValue<SIMD::Float> Abs(RValue<SIMD::Float> x);
RValue<SIMD::Float> Max(RValue<SIMD::Float> x, RValue<SIMD::Float> y);
RValue<SIMD::Float> Min(RValue<SIMD::Float> x, RValue<SIMD::Float> y);

RValue<SIMD::Float> Rcp(RValue<SIMD::Float> x, bool relaxedPrecision, bool exactAtPow2 = false);
RValue<SIMD::Float> RcpSqrt(RValue<SIMD::Float> x, bool relaxedPrecision);
RValue<SIMD::Float> Sqrt(RValue<SIMD::Float> x);
RValue<SIMD::Float> Insert(RValue<SIMD::Float> val, RValue<rr ::Float> element, int i);
RValue<rr ::Float> Extract(RValue<SIMD::Float> x, int i);

// Ordered comparison functions
RValue<SIMD::Int> CmpEQ(RValue<SIMD::Float> x, RValue<SIMD::Float> y);
RValue<SIMD::Int> CmpLT(RValue<SIMD::Float> x, RValue<SIMD::Float> y);
RValue<SIMD::Int> CmpLE(RValue<SIMD::Float> x, RValue<SIMD::Float> y);
RValue<SIMD::Int> CmpNEQ(RValue<SIMD::Float> x, RValue<SIMD::Float> y);
RValue<SIMD::Int> CmpNLT(RValue<SIMD::Float> x, RValue<SIMD::Float> y);
RValue<SIMD::Int> CmpNLE(RValue<SIMD::Float> x, RValue<SIMD::Float> y);
inline RValue<SIMD::Int> CmpGT(RValue<SIMD::Float> x, RValue<SIMD::Float> y)
{
	return CmpNLE(x, y);
}
inline RValue<SIMD::Int> CmpGE(RValue<SIMD::Float> x, RValue<SIMD::Float> y)
{
	return CmpNLT(x, y);
}

// Unordered comparison functions
RValue<SIMD::Int> CmpUEQ(RValue<SIMD::Float> x, RValue<SIMD::Float> y);
RValue<SIMD::Int> CmpULT(RValue<SIMD::Float> x, RValue<SIMD::Float> y);
RValue<SIMD::Int> CmpULE(RValue<SIMD::Float> x, RValue<SIMD::Float> y);
RValue<SIMD::Int> CmpUNEQ(RValue<SIMD::Float> x, RValue<SIMD::Float> y);
RValue<SIMD::Int> CmpUNLT(RValue<SIMD::Float> x, RValue<SIMD::Float> y);
RValue<SIMD::Int> CmpUNLE(RValue<SIMD::Float> x, RValue<SIMD::Float> y);
inline RValue<SIMD::Int> CmpUGT(RValue<SIMD::Float> x, RValue<SIMD::Float> y)
{
	return CmpUNLE(x, y);
}
inline RValue<SIMD::Int> CmpUGE(RValue<SIMD::Float> x, RValue<SIMD::Float> y)
{
	return CmpUNLT(x, y);
}

RValue<SIMD::Int> IsInf(RValue<SIMD::Float> x);
RValue<SIMD::Int> IsNan(RValue<SIMD::Float> x);
RValue<SIMD::Float> Round(RValue<SIMD::Float> x);
RValue<SIMD::Float> Trunc(RValue<SIMD::Float> x);
RValue<SIMD::Float> Frac(RValue<SIMD::Float> x);
RValue<SIMD::Float> Floor(RValue<SIMD::Float> x);
RValue<SIMD::Float> Ceil(RValue<SIMD::Float> x);

// Trigonometric functions
RValue<SIMD::Float> Sin(RValue<SIMD::Float> x);
RValue<SIMD::Float> Cos(RValue<SIMD::Float> x);
RValue<SIMD::Float> Tan(RValue<SIMD::Float> x);
RValue<SIMD::Float> Asin(RValue<SIMD::Float> x);
RValue<SIMD::Float> Acos(RValue<SIMD::Float> x);
RValue<SIMD::Float> Atan(RValue<SIMD::Float> x);
RValue<SIMD::Float> Sinh(RValue<SIMD::Float> x);
RValue<SIMD::Float> Cosh(RValue<SIMD::Float> x);
RValue<SIMD::Float> Tanh(RValue<SIMD::Float> x);
RValue<SIMD::Float> Asinh(RValue<SIMD::Float> x);
RValue<SIMD::Float> Acosh(RValue<SIMD::Float> x);
RValue<SIMD::Float> Atanh(RValue<SIMD::Float> x);
RValue<SIMD::Float> Atan2(RValue<SIMD::Float> x, RValue<SIMD::Float> y);

// Exponential functions
RValue<SIMD::Float> Pow(RValue<SIMD::Float> x, RValue<SIMD::Float> y);
RValue<SIMD::Float> Exp(RValue<SIMD::Float> x);
RValue<SIMD::Float> Log(RValue<SIMD::Float> x);
RValue<SIMD::Float> Exp2(RValue<SIMD::Float> x);
RValue<SIMD::Float> Log2(RValue<SIMD::Float> x);

}  // namespace rr::SIMD

#endif  // rr_SIMD_hpp
