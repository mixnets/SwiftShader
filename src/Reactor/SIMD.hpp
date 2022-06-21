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

namespace scalar = rr;

extern const int Width;

class Int;
class UInt;
class Float;

class Int : public LValue<Int>
{
public:
	explicit Int(RValue<Float> cast);

	Int();
	Int(int broadcast);
	Int(RValue<Int> rhs);
	Int(const Int &rhs);
	Int(const Reference<Int> &rhs);
	Int(RValue<UInt> rhs);
	Int(const UInt &rhs);
	Int(const Reference<UInt> &rhs);
	Int(RValue<scalar::Int2> lo, RValue<scalar::Int2> hi);
	Int(RValue<scalar::Int> rhs);
	Int(const scalar::Int &rhs);
	Int(const Reference<scalar::Int> &rhs);

	RValue<Int> operator=(int broadcast);
	RValue<Int> operator=(RValue<Int> rhs);
	RValue<Int> operator=(const Int &rhs);
	RValue<Int> operator=(const Reference<Int> &rhs);

	static Type *type();
	static int element_count() { return SIMD::Width; }
};

RValue<Int> operator+(RValue<Int> lhs, RValue<Int> rhs);
RValue<Int> operator-(RValue<Int> lhs, RValue<Int> rhs);
RValue<Int> operator*(RValue<Int> lhs, RValue<Int> rhs);
RValue<Int> operator/(RValue<Int> lhs, RValue<Int> rhs);
RValue<Int> operator%(RValue<Int> lhs, RValue<Int> rhs);
RValue<Int> operator&(RValue<Int> lhs, RValue<Int> rhs);
RValue<Int> operator|(RValue<Int> lhs, RValue<Int> rhs);
RValue<Int> operator^(RValue<Int> lhs, RValue<Int> rhs);
RValue<Int> operator<<(RValue<Int> lhs, unsigned char rhs);
RValue<Int> operator>>(RValue<Int> lhs, unsigned char rhs);
RValue<Int> operator<<(RValue<Int> lhs, RValue<Int> rhs);
RValue<Int> operator>>(RValue<Int> lhs, RValue<Int> rhs);
RValue<Int> operator+=(Int &lhs, RValue<Int> rhs);
RValue<Int> operator-=(Int &lhs, RValue<Int> rhs);
RValue<Int> operator*=(Int &lhs, RValue<Int> rhs);
//	RValue<Int> operator/=(Int &lhs, RValue<Int> rhs);
//	RValue<Int> operator%=(Int &lhs, RValue<Int> rhs);
RValue<Int> operator&=(Int &lhs, RValue<Int> rhs);
RValue<Int> operator|=(Int &lhs, RValue<Int> rhs);
RValue<Int> operator^=(Int &lhs, RValue<Int> rhs);
RValue<Int> operator<<=(Int &lhs, unsigned char rhs);
RValue<Int> operator>>=(Int &lhs, unsigned char rhs);
RValue<Int> operator+(RValue<Int> val);
RValue<Int> operator-(RValue<Int> val);
RValue<Int> operator~(RValue<Int> val);
//	RValue<Int> operator++(Int &val, int);   // Post-increment
//	const Int &operator++(Int &val);   // Pre-increment
//	RValue<Int> operator--(Int &val, int);   // Post-decrement
//	const Int &operator--(Int &val);   // Pre-decrement
//	RValue<Bool> operator<(RValue<Int> lhs, RValue<Int> rhs);
//	RValue<Bool> operator<=(RValue<Int> lhs, RValue<Int> rhs);
//	RValue<Bool> operator>(RValue<Int> lhs, RValue<Int> rhs);
//	RValue<Bool> operator>=(RValue<Int> lhs, RValue<Int> rhs);
//	RValue<Bool> operator!=(RValue<Int> lhs, RValue<Int> rhs);
//	RValue<Bool> operator==(RValue<Int> lhs, RValue<Int> rhs);

RValue<Int> CmpEQ(RValue<Int> x, RValue<Int> y);
RValue<Int> CmpLT(RValue<Int> x, RValue<Int> y);
RValue<Int> CmpLE(RValue<Int> x, RValue<Int> y);
RValue<Int> CmpNEQ(RValue<Int> x, RValue<Int> y);
RValue<Int> CmpNLT(RValue<Int> x, RValue<Int> y);
RValue<Int> CmpNLE(RValue<Int> x, RValue<Int> y);
inline RValue<Int> CmpGT(RValue<Int> x, RValue<Int> y)
{
	return CmpNLE(x, y);
}
inline RValue<Int> CmpGE(RValue<Int> x, RValue<Int> y)
{
	return CmpNLT(x, y);
}
RValue<Int> Max(RValue<Int> x, RValue<Int> y);
RValue<Int> Min(RValue<Int> x, RValue<Int> y);
// Convert to nearest integer. If a converted value is outside of the integer
// range, the returned result is undefined.
RValue<Int> RoundInt(RValue<Float> cast);
// Rounds to the nearest integer, but clamps very large values to an
// implementation-dependent range.
// Specifically, on x86, values larger than 2147483583.0 are converted to
// 2147483583 (0x7FFFFFBF) instead of producing 0x80000000.
RValue<Int> RoundIntClamped(RValue<Float> cast);
RValue<Short8> PackSigned(RValue<Int> x, RValue<Int> y);
RValue<UShort8> PackUnsigned(RValue<Int> x, RValue<Int> y);
RValue<scalar::Int> Extract(RValue<Int> val, int i);
RValue<Int> Insert(RValue<Int> val, RValue<scalar::Int> element, int i);

class UInt : public LValue<UInt>
{
public:
	explicit UInt(RValue<Float> cast);

	UInt();
	UInt(int broadcast);
	UInt(RValue<UInt> rhs);
	UInt(const UInt &rhs);
	UInt(const Reference<UInt> &rhs);
	UInt(RValue<Int> rhs);
	UInt(const Int &rhs);
	UInt(const Reference<Int> &rhs);
	UInt(RValue<scalar::UInt2> lo, RValue<scalar::UInt2> hi);
	UInt(RValue<scalar::UInt> rhs);
	UInt(const scalar::UInt &rhs);
	UInt(const Reference<scalar::UInt> &rhs);

	RValue<UInt> operator=(RValue<UInt> rhs);
	RValue<UInt> operator=(const UInt &rhs);
	RValue<UInt> operator=(const Reference<UInt> &rhs);

	static Type *type();
	static int element_count() { return SIMD::Width; }
};

RValue<UInt> operator+(RValue<UInt> lhs, RValue<UInt> rhs);
RValue<UInt> operator-(RValue<UInt> lhs, RValue<UInt> rhs);
RValue<UInt> operator*(RValue<UInt> lhs, RValue<UInt> rhs);
RValue<UInt> operator/(RValue<UInt> lhs, RValue<UInt> rhs);
RValue<UInt> operator%(RValue<UInt> lhs, RValue<UInt> rhs);
RValue<UInt> operator&(RValue<UInt> lhs, RValue<UInt> rhs);
RValue<UInt> operator|(RValue<UInt> lhs, RValue<UInt> rhs);
RValue<UInt> operator^(RValue<UInt> lhs, RValue<UInt> rhs);
RValue<UInt> operator<<(RValue<UInt> lhs, unsigned char rhs);
RValue<UInt> operator>>(RValue<UInt> lhs, unsigned char rhs);
RValue<UInt> operator<<(RValue<UInt> lhs, RValue<UInt> rhs);
RValue<UInt> operator>>(RValue<UInt> lhs, RValue<UInt> rhs);
RValue<UInt> operator+=(UInt &lhs, RValue<UInt> rhs);
RValue<UInt> operator-=(UInt &lhs, RValue<UInt> rhs);
RValue<UInt> operator*=(UInt &lhs, RValue<UInt> rhs);
//	RValue<UInt> operator/=(UInt &lhs, RValue<UInt> rhs);
//	RValue<UInt> operator%=(UInt &lhs, RValue<UInt> rhs);
RValue<UInt> operator&=(UInt &lhs, RValue<UInt> rhs);
RValue<UInt> operator|=(UInt &lhs, RValue<UInt> rhs);
RValue<UInt> operator^=(UInt &lhs, RValue<UInt> rhs);
RValue<UInt> operator<<=(UInt &lhs, unsigned char rhs);
RValue<UInt> operator>>=(UInt &lhs, unsigned char rhs);
RValue<UInt> operator+(RValue<UInt> val);
RValue<UInt> operator-(RValue<UInt> val);
RValue<UInt> operator~(RValue<UInt> val);
//	RValue<UInt> operator++(UInt &val, int);   // Post-increment
//	const UInt &operator++(UInt &val);   // Pre-increment
//	RValue<UInt> operator--(UInt &val, int);   // Post-decrement
//	const UInt &operator--(UInt &val);   // Pre-decrement
//	RValue<Bool> operator<(RValue<UInt> lhs, RValue<UInt> rhs);
//	RValue<Bool> operator<=(RValue<UInt> lhs, RValue<UInt> rhs);
//	RValue<Bool> operator>(RValue<UInt> lhs, RValue<UInt> rhs);
//	RValue<Bool> operator>=(RValue<UInt> lhs, RValue<UInt> rhs);
//	RValue<Bool> operator!=(RValue<UInt> lhs, RValue<UInt> rhs);
//	RValue<Bool> operator==(RValue<UInt> lhs, RValue<UInt> rhs);

RValue<UInt> CmpEQ(RValue<UInt> x, RValue<UInt> y);
RValue<UInt> CmpLT(RValue<UInt> x, RValue<UInt> y);
RValue<UInt> CmpLE(RValue<UInt> x, RValue<UInt> y);
RValue<UInt> CmpNEQ(RValue<UInt> x, RValue<UInt> y);
RValue<UInt> CmpNLT(RValue<UInt> x, RValue<UInt> y);
RValue<UInt> CmpNLE(RValue<UInt> x, RValue<UInt> y);
inline RValue<UInt> CmpGT(RValue<UInt> x, RValue<UInt> y)
{
	return CmpNLE(x, y);
}
inline RValue<UInt> CmpGE(RValue<UInt> x, RValue<UInt> y)
{
	return CmpNLT(x, y);
}
RValue<UInt> Max(RValue<UInt> x, RValue<UInt> y);
RValue<UInt> Min(RValue<UInt> x, RValue<UInt> y);
RValue<scalar::UInt> Extract(RValue<UInt> val, int i);
RValue<UInt> Insert(RValue<UInt> val, RValue<scalar::UInt> element, int i);
//	RValue<UInt> RoundInt(RValue<Float> cast);

class Float : public LValue<Float>
{
public:
	explicit Float(RValue<Int> cast);
	explicit Float(RValue<UInt> cast);

	Float();
	Float(float broadcast);
	Float(RValue<Float> rhs);
	Float(const Float &rhs);
	Float(const Reference<Float> &rhs);
	Float(RValue<scalar::Float> rhs);
	Float(const scalar::Float &rhs);
	Float(const Reference<scalar::Float> &rhs);

	RValue<Float> operator=(float broadcast);
	RValue<Float> operator=(RValue<Float> rhs);
	RValue<Float> operator=(const Float &rhs);
	RValue<Float> operator=(const Reference<Float> &rhs);
	RValue<Float> operator=(RValue<scalar::Float> rhs);
	RValue<Float> operator=(const scalar::Float &rhs);
	RValue<Float> operator=(const Reference<scalar::Float> &rhs);

	static Float infinity();

	static Type *type();
	static int element_count() { return SIMD::Width; }
};

RValue<Float> operator+(RValue<Float> lhs, RValue<Float> rhs);
RValue<Float> operator-(RValue<Float> lhs, RValue<Float> rhs);
RValue<Float> operator*(RValue<Float> lhs, RValue<Float> rhs);
RValue<Float> operator/(RValue<Float> lhs, RValue<Float> rhs);
RValue<Float> operator%(RValue<Float> lhs, RValue<Float> rhs);
RValue<Float> operator+=(Float &lhs, RValue<Float> rhs);
RValue<Float> operator-=(Float &lhs, RValue<Float> rhs);
RValue<Float> operator*=(Float &lhs, RValue<Float> rhs);
RValue<Float> operator/=(Float &lhs, RValue<Float> rhs);
RValue<Float> operator%=(Float &lhs, RValue<Float> rhs);
RValue<Float> operator+(RValue<Float> val);
RValue<Float> operator-(RValue<Float> val);

// Computes `x * y + z`, which may be fused into one operation to produce a higher-precision result.
RValue<Float> MulAdd(RValue<Float> x, RValue<Float> y, RValue<Float> z);
// Computes a fused `x * y + z` operation. Caps::fmaIsFast indicates whether it emits an FMA instruction.
RValue<Float> FMA(RValue<Float> x, RValue<Float> y, RValue<Float> z);

RValue<Float> Abs(RValue<Float> x);
RValue<Float> Max(RValue<Float> x, RValue<Float> y);
RValue<Float> Min(RValue<Float> x, RValue<Float> y);

RValue<Float> Rcp(RValue<Float> x, bool relaxedPrecision, bool exactAtPow2 = false);
RValue<Float> RcpSqrt(RValue<Float> x, bool relaxedPrecision);
RValue<Float> Sqrt(RValue<Float> x);
RValue<Float> Insert(RValue<Float> val, RValue<rr ::Float> element, int i);
RValue<rr ::Float> Extract(RValue<Float> x, int i);

// Ordered comparison functions
RValue<Int> CmpEQ(RValue<Float> x, RValue<Float> y);
RValue<Int> CmpLT(RValue<Float> x, RValue<Float> y);
RValue<Int> CmpLE(RValue<Float> x, RValue<Float> y);
RValue<Int> CmpNEQ(RValue<Float> x, RValue<Float> y);
RValue<Int> CmpNLT(RValue<Float> x, RValue<Float> y);
RValue<Int> CmpNLE(RValue<Float> x, RValue<Float> y);
inline RValue<Int> CmpGT(RValue<Float> x, RValue<Float> y)
{
	return CmpNLE(x, y);
}
inline RValue<Int> CmpGE(RValue<Float> x, RValue<Float> y)
{
	return CmpNLT(x, y);
}

// Unordered comparison functions
RValue<Int> CmpUEQ(RValue<Float> x, RValue<Float> y);
RValue<Int> CmpULT(RValue<Float> x, RValue<Float> y);
RValue<Int> CmpULE(RValue<Float> x, RValue<Float> y);
RValue<Int> CmpUNEQ(RValue<Float> x, RValue<Float> y);
RValue<Int> CmpUNLT(RValue<Float> x, RValue<Float> y);
RValue<Int> CmpUNLE(RValue<Float> x, RValue<Float> y);
inline RValue<Int> CmpUGT(RValue<Float> x, RValue<Float> y)
{
	return CmpUNLE(x, y);
}
inline RValue<Int> CmpUGE(RValue<Float> x, RValue<Float> y)
{
	return CmpUNLT(x, y);
}

RValue<Int> IsInf(RValue<Float> x);
RValue<Int> IsNan(RValue<Float> x);
RValue<Float> Round(RValue<Float> x);
RValue<Float> Trunc(RValue<Float> x);
RValue<Float> Frac(RValue<Float> x);
RValue<Float> Floor(RValue<Float> x);
RValue<Float> Ceil(RValue<Float> x);

// Trigonometric functions
RValue<Float> Sin(RValue<Float> x);
RValue<Float> Cos(RValue<Float> x);
RValue<Float> Tan(RValue<Float> x);
RValue<Float> Asin(RValue<Float> x);
RValue<Float> Acos(RValue<Float> x);
RValue<Float> Atan(RValue<Float> x);
RValue<Float> Sinh(RValue<Float> x);
RValue<Float> Cosh(RValue<Float> x);
RValue<Float> Tanh(RValue<Float> x);
RValue<Float> Asinh(RValue<Float> x);
RValue<Float> Acosh(RValue<Float> x);
RValue<Float> Atanh(RValue<Float> x);
RValue<Float> Atan2(RValue<Float> x, RValue<Float> y);

// Exponential functions
RValue<Float> Pow(RValue<Float> x, RValue<Float> y);
RValue<Float> Exp(RValue<Float> x);
RValue<Float> Log(RValue<Float> x);
RValue<Float> Exp2(RValue<Float> x);
RValue<Float> Log2(RValue<Float> x);

}  // namespace rr::SIMD

#endif  // rr_SIMD_hpp
