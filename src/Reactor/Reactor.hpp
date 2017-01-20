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

#ifndef sw_Reactor_hpp
#define sw_Reactor_hpp

#include "Nucleus.hpp"
#include "Routine.hpp"

#include <cstddef>
#include <cwchar>
#undef Bool

namespace sw
{
	class Bool;
	class Byte;
	class SByte;
	class Byte4;
	class SByte4;
	class Byte8;
	class SByte8;
	class Byte16;
	class SByte16;
	class Short;
	class UShort;
	class Short2;
	class UShort2;
	class Short4;
	class UShort4;
	class Short8;
	class UShort8;
	class Int;
	class UInt;
	class Int2;
	class UInt2;
	class Int4;
	class UInt4;
	class Long;
	class Float;
	class Float2;
	class Float4;

	class Void
	{
	public:
		static Type *getType();

		static bool isVoid()
		{
			return true;
		}
	};

	template<class T>
	class RValue;

	template<class T>
	class Pointer;

	class Variable
	{
	protected:
		Value *address;
	};

	template<class T>
	class LValue : public Variable
	{
	public:
		LValue(int arraySize = 0);

		RValue<Pointer<T>> operator&();

		static bool isVoid()
		{
			return false;
		}

		Value *loadValue(unsigned int alignment = 0) const;
		Value *storeValue(Value *value, unsigned int alignment = 0) const;
		Value *getAddress(Value *index) const;
	};

	template<class T>
	class Reference
	{
	public:
		explicit Reference(Value *pointer, int alignment = 1);

		RValue<T> operator=(RValue<T> rhs) const;
		RValue<T> operator=(const Reference<T> &ref) const;

		RValue<T> operator+=(RValue<T> rhs) const;

		Value *loadValue() const;
		int getAlignment() const;

	private:
		Value *address;

		const int alignment;
	};

	template<class T>
	struct IntLiteral
	{
		struct type;
	};

	template<>
	struct IntLiteral<Bool>
	{
		typedef bool type;
	};

	template<>
	struct IntLiteral<Int>
	{
		typedef int type;
	};

	template<>
	struct IntLiteral<UInt>
	{
		typedef unsigned int type;
	};

	template<>
	struct IntLiteral<Long>
	{
		typedef int64_t type;
	};

	template<class T>
	struct FloatLiteral
	{
		struct type;
	};

	template<>
	struct FloatLiteral<Float>
	{
		typedef float type;
	};

	template<class T>
	class RValue
	{
	public:
		explicit RValue(Value *rvalue);

		RValue(const T &lvalue);
		RValue(typename IntLiteral<T>::type i);
		RValue(typename FloatLiteral<T>::type f);
		RValue(const Reference<T> &rhs);

		RValue<T> &operator=(const RValue<T>&) = delete;

		Value *value;   // FIXME: Make private
	};

	template<typename T>
	struct Argument
	{
		explicit Argument(Value *value) : value(value) {}

		Value *value;
	};

	class Bool : public LValue<Bool>
	{
	public:
		Bool(Argument<Bool> argument);

		Bool() = default;
		Bool(bool x);
		Bool(RValue<Bool> rhs);
		Bool(const Bool &rhs);
		Bool(const Reference<Bool> &rhs);

	//	RValue<Bool> operator=(bool rhs);   // FIXME: Implement
		RValue<Bool> operator=(RValue<Bool> rhs);
		RValue<Bool> operator=(const Bool &rhs);
		RValue<Bool> operator=(const Reference<Bool> &rhs);

		static Type *getType();
	};

	RValue<Bool> operator!(RValue<Bool> val);
	RValue<Bool> operator&&(RValue<Bool> lhs, RValue<Bool> rhs);
	RValue<Bool> operator||(RValue<Bool> lhs, RValue<Bool> rhs);

	class Byte : public LValue<Byte>
	{
	public:
		Byte(Argument<Byte> argument);

		explicit Byte(RValue<Int> cast);
		explicit Byte(RValue<UInt> cast);
		explicit Byte(RValue<UShort> cast);

		Byte() = default;
		Byte(int x);
		Byte(unsigned char x);
		Byte(RValue<Byte> rhs);
		Byte(const Byte &rhs);
		Byte(const Reference<Byte> &rhs);

	//	RValue<Byte> operator=(unsigned char rhs);   // FIXME: Implement
		RValue<Byte> operator=(RValue<Byte> rhs);
		RValue<Byte> operator=(const Byte &rhs);
		RValue<Byte> operator=(const Reference<Byte> &rhs);

		static Type *getType();
	};

	RValue<Byte> operator+(RValue<Byte> lhs, RValue<Byte> rhs);
	RValue<Byte> operator-(RValue<Byte> lhs, RValue<Byte> rhs);
	RValue<Byte> operator*(RValue<Byte> lhs, RValue<Byte> rhs);
	RValue<Byte> operator/(RValue<Byte> lhs, RValue<Byte> rhs);
	RValue<Byte> operator%(RValue<Byte> lhs, RValue<Byte> rhs);
	RValue<Byte> operator&(RValue<Byte> lhs, RValue<Byte> rhs);
	RValue<Byte> operator|(RValue<Byte> lhs, RValue<Byte> rhs);
	RValue<Byte> operator^(RValue<Byte> lhs, RValue<Byte> rhs);
	RValue<Byte> operator<<(RValue<Byte> lhs, RValue<Byte> rhs);
	RValue<Byte> operator>>(RValue<Byte> lhs, RValue<Byte> rhs);
	RValue<Byte> operator+=(Byte &lhs, RValue<Byte> rhs);
	RValue<Byte> operator-=(Byte &lhs, RValue<Byte> rhs);
	RValue<Byte> operator*=(Byte &lhs, RValue<Byte> rhs);
	RValue<Byte> operator/=(Byte &lhs, RValue<Byte> rhs);
	RValue<Byte> operator%=(Byte &lhs, RValue<Byte> rhs);
	RValue<Byte> operator&=(Byte &lhs, RValue<Byte> rhs);
	RValue<Byte> operator|=(Byte &lhs, RValue<Byte> rhs);
	RValue<Byte> operator^=(Byte &lhs, RValue<Byte> rhs);
	RValue<Byte> operator<<=(Byte &lhs, RValue<Byte> rhs);
	RValue<Byte> operator>>=(Byte &lhs, RValue<Byte> rhs);
	RValue<Byte> operator+(RValue<Byte> val);
	RValue<Byte> operator-(RValue<Byte> val);
	RValue<Byte> operator~(RValue<Byte> val);
	RValue<Byte> operator++(Byte &val, int);   // Post-increment
	const Byte &operator++(Byte &val);   // Pre-increment
	RValue<Byte> operator--(Byte &val, int);   // Post-decrement
	const Byte &operator--(Byte &val);   // Pre-decrement
	RValue<Bool> operator<(RValue<Byte> lhs, RValue<Byte> rhs);
	RValue<Bool> operator<=(RValue<Byte> lhs, RValue<Byte> rhs);
	RValue<Bool> operator>(RValue<Byte> lhs, RValue<Byte> rhs);
	RValue<Bool> operator>=(RValue<Byte> lhs, RValue<Byte> rhs);
	RValue<Bool> operator!=(RValue<Byte> lhs, RValue<Byte> rhs);
	RValue<Bool> operator==(RValue<Byte> lhs, RValue<Byte> rhs);

	class SByte : public LValue<SByte>
	{
	public:
		SByte(Argument<SByte> argument);

		explicit SByte(RValue<Int> cast);
		explicit SByte(RValue<Short> cast);

		SByte() = default;
		SByte(signed char x);
		SByte(RValue<SByte> rhs);
		SByte(const SByte &rhs);
		SByte(const Reference<SByte> &rhs);

	//	RValue<SByte> operator=(signed char rhs);   // FIXME: Implement
		RValue<SByte> operator=(RValue<SByte> rhs);
		RValue<SByte> operator=(const SByte &rhs);
		RValue<SByte> operator=(const Reference<SByte> &rhs);

		static Type *getType();
	};

	RValue<SByte> operator+(RValue<SByte> lhs, RValue<SByte> rhs);
	RValue<SByte> operator-(RValue<SByte> lhs, RValue<SByte> rhs);
	RValue<SByte> operator*(RValue<SByte> lhs, RValue<SByte> rhs);
	RValue<SByte> operator/(RValue<SByte> lhs, RValue<SByte> rhs);
	RValue<SByte> operator%(RValue<SByte> lhs, RValue<SByte> rhs);
	RValue<SByte> operator&(RValue<SByte> lhs, RValue<SByte> rhs);
	RValue<SByte> operator|(RValue<SByte> lhs, RValue<SByte> rhs);
	RValue<SByte> operator^(RValue<SByte> lhs, RValue<SByte> rhs);
	RValue<SByte> operator<<(RValue<SByte> lhs, RValue<SByte> rhs);
	RValue<SByte> operator>>(RValue<SByte> lhs, RValue<SByte> rhs);
	RValue<SByte> operator+=(SByte &lhs, RValue<SByte> rhs);
	RValue<SByte> operator-=(SByte &lhs, RValue<SByte> rhs);
	RValue<SByte> operator*=(SByte &lhs, RValue<SByte> rhs);
	RValue<SByte> operator/=(SByte &lhs, RValue<SByte> rhs);
	RValue<SByte> operator%=(SByte &lhs, RValue<SByte> rhs);
	RValue<SByte> operator&=(SByte &lhs, RValue<SByte> rhs);
	RValue<SByte> operator|=(SByte &lhs, RValue<SByte> rhs);
	RValue<SByte> operator^=(SByte &lhs, RValue<SByte> rhs);
	RValue<SByte> operator<<=(SByte &lhs, RValue<SByte> rhs);
	RValue<SByte> operator>>=(SByte &lhs, RValue<SByte> rhs);
	RValue<SByte> operator+(RValue<SByte> val);
	RValue<SByte> operator-(RValue<SByte> val);
	RValue<SByte> operator~(RValue<SByte> val);
	RValue<SByte> operator++(SByte &val, int);   // Post-increment
	const SByte &operator++(SByte &val);   // Pre-increment
	RValue<SByte> operator--(SByte &val, int);   // Post-decrement
	const SByte &operator--(SByte &val);   // Pre-decrement
	RValue<Bool> operator<(RValue<SByte> lhs, RValue<SByte> rhs);
	RValue<Bool> operator<=(RValue<SByte> lhs, RValue<SByte> rhs);
	RValue<Bool> operator>(RValue<SByte> lhs, RValue<SByte> rhs);
	RValue<Bool> operator>=(RValue<SByte> lhs, RValue<SByte> rhs);
	RValue<Bool> operator!=(RValue<SByte> lhs, RValue<SByte> rhs);
	RValue<Bool> operator==(RValue<SByte> lhs, RValue<SByte> rhs);

	class Short : public LValue<Short>
	{
	public:
		Short(Argument<Short> argument);

		explicit Short(RValue<Int> cast);

		Short() = default;
		Short(short x);
		Short(RValue<Short> rhs);
		Short(const Short &rhs);
		Short(const Reference<Short> &rhs);

	//	RValue<Short> operator=(short rhs);   // FIXME: Implement
		RValue<Short> operator=(RValue<Short> rhs);
		RValue<Short> operator=(const Short &rhs);
		RValue<Short> operator=(const Reference<Short> &rhs);

		static Type *getType();
	};

	RValue<Short> operator+(RValue<Short> lhs, RValue<Short> rhs);
	RValue<Short> operator-(RValue<Short> lhs, RValue<Short> rhs);
	RValue<Short> operator*(RValue<Short> lhs, RValue<Short> rhs);
	RValue<Short> operator/(RValue<Short> lhs, RValue<Short> rhs);
	RValue<Short> operator%(RValue<Short> lhs, RValue<Short> rhs);
	RValue<Short> operator&(RValue<Short> lhs, RValue<Short> rhs);
	RValue<Short> operator|(RValue<Short> lhs, RValue<Short> rhs);
	RValue<Short> operator^(RValue<Short> lhs, RValue<Short> rhs);
	RValue<Short> operator<<(RValue<Short> lhs, RValue<Short> rhs);
	RValue<Short> operator>>(RValue<Short> lhs, RValue<Short> rhs);
	RValue<Short> operator+=(Short &lhs, RValue<Short> rhs);
	RValue<Short> operator-=(Short &lhs, RValue<Short> rhs);
	RValue<Short> operator*=(Short &lhs, RValue<Short> rhs);
	RValue<Short> operator/=(Short &lhs, RValue<Short> rhs);
	RValue<Short> operator%=(Short &lhs, RValue<Short> rhs);
	RValue<Short> operator&=(Short &lhs, RValue<Short> rhs);
	RValue<Short> operator|=(Short &lhs, RValue<Short> rhs);
	RValue<Short> operator^=(Short &lhs, RValue<Short> rhs);
	RValue<Short> operator<<=(Short &lhs, RValue<Short> rhs);
	RValue<Short> operator>>=(Short &lhs, RValue<Short> rhs);
	RValue<Short> operator+(RValue<Short> val);
	RValue<Short> operator-(RValue<Short> val);
	RValue<Short> operator~(RValue<Short> val);
	RValue<Short> operator++(Short &val, int);   // Post-increment
	const Short &operator++(Short &val);   // Pre-increment
	RValue<Short> operator--(Short &val, int);   // Post-decrement
	const Short &operator--(Short &val);   // Pre-decrement
	RValue<Bool> operator<(RValue<Short> lhs, RValue<Short> rhs);
	RValue<Bool> operator<=(RValue<Short> lhs, RValue<Short> rhs);
	RValue<Bool> operator>(RValue<Short> lhs, RValue<Short> rhs);
	RValue<Bool> operator>=(RValue<Short> lhs, RValue<Short> rhs);
	RValue<Bool> operator!=(RValue<Short> lhs, RValue<Short> rhs);
	RValue<Bool> operator==(RValue<Short> lhs, RValue<Short> rhs);

	class UShort : public LValue<UShort>
	{
	public:
		UShort(Argument<UShort> argument);

		explicit UShort(RValue<UInt> cast);
		explicit UShort(RValue<Int> cast);

		UShort() = default;
		UShort(unsigned short x);
		UShort(RValue<UShort> rhs);
		UShort(const UShort &rhs);
		UShort(const Reference<UShort> &rhs);

	//	RValue<UShort> operator=(unsigned short rhs);   // FIXME: Implement
		RValue<UShort> operator=(RValue<UShort> rhs);
		RValue<UShort> operator=(const UShort &rhs);
		RValue<UShort> operator=(const Reference<UShort> &rhs);

		static Type *getType();
	};

	RValue<UShort> operator+(RValue<UShort> lhs, RValue<UShort> rhs);
	RValue<UShort> operator-(RValue<UShort> lhs, RValue<UShort> rhs);
	RValue<UShort> operator*(RValue<UShort> lhs, RValue<UShort> rhs);
	RValue<UShort> operator/(RValue<UShort> lhs, RValue<UShort> rhs);
	RValue<UShort> operator%(RValue<UShort> lhs, RValue<UShort> rhs);
	RValue<UShort> operator&(RValue<UShort> lhs, RValue<UShort> rhs);
	RValue<UShort> operator|(RValue<UShort> lhs, RValue<UShort> rhs);
	RValue<UShort> operator^(RValue<UShort> lhs, RValue<UShort> rhs);
	RValue<UShort> operator<<(RValue<UShort> lhs, RValue<UShort> rhs);
	RValue<UShort> operator>>(RValue<UShort> lhs, RValue<UShort> rhs);
	RValue<UShort> operator+=(UShort &lhs, RValue<UShort> rhs);
	RValue<UShort> operator-=(UShort &lhs, RValue<UShort> rhs);
	RValue<UShort> operator*=(UShort &lhs, RValue<UShort> rhs);
	RValue<UShort> operator/=(UShort &lhs, RValue<UShort> rhs);
	RValue<UShort> operator%=(UShort &lhs, RValue<UShort> rhs);
	RValue<UShort> operator&=(UShort &lhs, RValue<UShort> rhs);
	RValue<UShort> operator|=(UShort &lhs, RValue<UShort> rhs);
	RValue<UShort> operator^=(UShort &lhs, RValue<UShort> rhs);
	RValue<UShort> operator<<=(UShort &lhs, RValue<UShort> rhs);
	RValue<UShort> operator>>=(UShort &lhs, RValue<UShort> rhs);
	RValue<UShort> operator+(RValue<UShort> val);
	RValue<UShort> operator-(RValue<UShort> val);
	RValue<UShort> operator~(RValue<UShort> val);
	RValue<UShort> operator++(UShort &val, int);   // Post-increment
	const UShort &operator++(UShort &val);   // Pre-increment
	RValue<UShort> operator--(UShort &val, int);   // Post-decrement
	const UShort &operator--(UShort &val);   // Pre-decrement
	RValue<Bool> operator<(RValue<UShort> lhs, RValue<UShort> rhs);
	RValue<Bool> operator<=(RValue<UShort> lhs, RValue<UShort> rhs);
	RValue<Bool> operator>(RValue<UShort> lhs, RValue<UShort> rhs);
	RValue<Bool> operator>=(RValue<UShort> lhs, RValue<UShort> rhs);
	RValue<Bool> operator!=(RValue<UShort> lhs, RValue<UShort> rhs);
	RValue<Bool> operator==(RValue<UShort> lhs, RValue<UShort> rhs);

	class Byte4 : public LValue<Byte4>
	{
	public:
		explicit Byte4(RValue<Byte8> cast);

		Byte4() = default;
	//	Byte4(int x, int y, int z, int w);
	//	Byte4(RValue<Byte4> rhs);
	//	Byte4(const Byte4 &rhs);
		Byte4(const Reference<Byte4> &rhs);

	//	RValue<Byte4> operator=(RValue<Byte4> rhs);
	//	RValue<Byte4> operator=(const Byte4 &rhs);
	//	RValue<Byte4> operator=(const Reference<Byte4> &rhs);

		static Type *getType();
	};

//	RValue<Byte4> operator+(RValue<Byte4> lhs, RValue<Byte4> rhs);
//	RValue<Byte4> operator-(RValue<Byte4> lhs, RValue<Byte4> rhs);
//	RValue<Byte4> operator*(RValue<Byte4> lhs, RValue<Byte4> rhs);
//	RValue<Byte4> operator/(RValue<Byte4> lhs, RValue<Byte4> rhs);
//	RValue<Byte4> operator%(RValue<Byte4> lhs, RValue<Byte4> rhs);
//	RValue<Byte4> operator&(RValue<Byte4> lhs, RValue<Byte4> rhs);
//	RValue<Byte4> operator|(RValue<Byte4> lhs, RValue<Byte4> rhs);
//	RValue<Byte4> operator^(RValue<Byte4> lhs, RValue<Byte4> rhs);
//	RValue<Byte4> operator<<(RValue<Byte4> lhs, RValue<Byte4> rhs);
//	RValue<Byte4> operator>>(RValue<Byte4> lhs, RValue<Byte4> rhs);
//	RValue<Byte4> operator+=(Byte4 &lhs, RValue<Byte4> rhs);
//	RValue<Byte4> operator-=(Byte4 &lhs, RValue<Byte4> rhs);
//	RValue<Byte4> operator*=(Byte4 &lhs, RValue<Byte4> rhs);
//	RValue<Byte4> operator/=(Byte4 &lhs, RValue<Byte4> rhs);
//	RValue<Byte4> operator%=(Byte4 &lhs, RValue<Byte4> rhs);
//	RValue<Byte4> operator&=(Byte4 &lhs, RValue<Byte4> rhs);
//	RValue<Byte4> operator|=(Byte4 &lhs, RValue<Byte4> rhs);
//	RValue<Byte4> operator^=(Byte4 &lhs, RValue<Byte4> rhs);
//	RValue<Byte4> operator<<=(Byte4 &lhs, RValue<Byte4> rhs);
//	RValue<Byte4> operator>>=(Byte4 &lhs, RValue<Byte4> rhs);
//	RValue<Byte4> operator+(RValue<Byte4> val);
//	RValue<Byte4> operator-(RValue<Byte4> val);
//	RValue<Byte4> operator~(RValue<Byte4> val);
//	RValue<Byte4> operator++(Byte4 &val, int);   // Post-increment
//	const Byte4 &operator++(Byte4 &val);   // Pre-increment
//	RValue<Byte4> operator--(Byte4 &val, int);   // Post-decrement
//	const Byte4 &operator--(Byte4 &val);   // Pre-decrement

	class SByte4 : public LValue<SByte4>
	{
	public:
		SByte4() = default;
	//	SByte4(int x, int y, int z, int w);
	//	SByte4(RValue<SByte4> rhs);
	//	SByte4(const SByte4 &rhs);
	//	SByte4(const Reference<SByte4> &rhs);

	//	RValue<SByte4> operator=(RValue<SByte4> rhs);
	//	RValue<SByte4> operator=(const SByte4 &rhs);
	//	RValue<SByte4> operator=(const Reference<SByte4> &rhs);

		static Type *getType();
	};

//	RValue<SByte4> operator+(RValue<SByte4> lhs, RValue<SByte4> rhs);
//	RValue<SByte4> operator-(RValue<SByte4> lhs, RValue<SByte4> rhs);
//	RValue<SByte4> operator*(RValue<SByte4> lhs, RValue<SByte4> rhs);
//	RValue<SByte4> operator/(RValue<SByte4> lhs, RValue<SByte4> rhs);
//	RValue<SByte4> operator%(RValue<SByte4> lhs, RValue<SByte4> rhs);
//	RValue<SByte4> operator&(RValue<SByte4> lhs, RValue<SByte4> rhs);
//	RValue<SByte4> operator|(RValue<SByte4> lhs, RValue<SByte4> rhs);
//	RValue<SByte4> operator^(RValue<SByte4> lhs, RValue<SByte4> rhs);
//	RValue<SByte4> operator<<(RValue<SByte4> lhs, RValue<SByte4> rhs);
//	RValue<SByte4> operator>>(RValue<SByte4> lhs, RValue<SByte4> rhs);
//	RValue<SByte4> operator+=(SByte4 &lhs, RValue<SByte4> rhs);
//	RValue<SByte4> operator-=(SByte4 &lhs, RValue<SByte4> rhs);
//	RValue<SByte4> operator*=(SByte4 &lhs, RValue<SByte4> rhs);
//	RValue<SByte4> operator/=(SByte4 &lhs, RValue<SByte4> rhs);
//	RValue<SByte4> operator%=(SByte4 &lhs, RValue<SByte4> rhs);
//	RValue<SByte4> operator&=(SByte4 &lhs, RValue<SByte4> rhs);
//	RValue<SByte4> operator|=(SByte4 &lhs, RValue<SByte4> rhs);
//	RValue<SByte4> operator^=(SByte4 &lhs, RValue<SByte4> rhs);
//	RValue<SByte4> operator<<=(SByte4 &lhs, RValue<SByte4> rhs);
//	RValue<SByte4> operator>>=(SByte4 &lhs, RValue<SByte4> rhs);
//	RValue<SByte4> operator+(RValue<SByte4> val);
//	RValue<SByte4> operator-(RValue<SByte4> val);
//	RValue<SByte4> operator~(RValue<SByte4> val);
//	RValue<SByte4> operator++(SByte4 &val, int);   // Post-increment
//	const SByte4 &operator++(SByte4 &val);   // Pre-increment
//	RValue<SByte4> operator--(SByte4 &val, int);   // Post-decrement
//	const SByte4 &operator--(SByte4 &val);   // Pre-decrement

	class Byte8 : public LValue<Byte8>
	{
	public:
		Byte8() = default;
		Byte8(uint8_t x0, uint8_t x1, uint8_t x2, uint8_t x3, uint8_t x4, uint8_t x5, uint8_t x6, uint8_t x7);
		Byte8(RValue<Byte8> rhs);
		Byte8(const Byte8 &rhs);
		Byte8(const Reference<Byte8> &rhs);

		RValue<Byte8> operator=(RValue<Byte8> rhs);
		RValue<Byte8> operator=(const Byte8 &rhs);
		RValue<Byte8> operator=(const Reference<Byte8> &rhs);

		static Type *getType();
	};

	RValue<Byte8> operator+(RValue<Byte8> lhs, RValue<Byte8> rhs);
	RValue<Byte8> operator-(RValue<Byte8> lhs, RValue<Byte8> rhs);
//	RValue<Byte8> operator*(RValue<Byte8> lhs, RValue<Byte8> rhs);
//	RValue<Byte8> operator/(RValue<Byte8> lhs, RValue<Byte8> rhs);
//	RValue<Byte8> operator%(RValue<Byte8> lhs, RValue<Byte8> rhs);
	RValue<Byte8> operator&(RValue<Byte8> lhs, RValue<Byte8> rhs);
	RValue<Byte8> operator|(RValue<Byte8> lhs, RValue<Byte8> rhs);
	RValue<Byte8> operator^(RValue<Byte8> lhs, RValue<Byte8> rhs);
//	RValue<Byte8> operator<<(RValue<Byte8> lhs, RValue<Byte8> rhs);
//	RValue<Byte8> operator>>(RValue<Byte8> lhs, RValue<Byte8> rhs);
	RValue<Byte8> operator+=(Byte8 &lhs, RValue<Byte8> rhs);
	RValue<Byte8> operator-=(Byte8 &lhs, RValue<Byte8> rhs);
//	RValue<Byte8> operator*=(Byte8 &lhs, RValue<Byte8> rhs);
//	RValue<Byte8> operator/=(Byte8 &lhs, RValue<Byte8> rhs);
//	RValue<Byte8> operator%=(Byte8 &lhs, RValue<Byte8> rhs);
	RValue<Byte8> operator&=(Byte8 &lhs, RValue<Byte8> rhs);
	RValue<Byte8> operator|=(Byte8 &lhs, RValue<Byte8> rhs);
	RValue<Byte8> operator^=(Byte8 &lhs, RValue<Byte8> rhs);
//	RValue<Byte8> operator<<=(Byte8 &lhs, RValue<Byte8> rhs);
//	RValue<Byte8> operator>>=(Byte8 &lhs, RValue<Byte8> rhs);
//	RValue<Byte8> operator+(RValue<Byte8> val);
//	RValue<Byte8> operator-(RValue<Byte8> val);
	RValue<Byte8> operator~(RValue<Byte8> val);
//	RValue<Byte8> operator++(Byte8 &val, int);   // Post-increment
//	const Byte8 &operator++(Byte8 &val);   // Pre-increment
//	RValue<Byte8> operator--(Byte8 &val, int);   // Post-decrement
//	const Byte8 &operator--(Byte8 &val);   // Pre-decrement

	RValue<Byte8> AddSat(RValue<Byte8> x, RValue<Byte8> y);
	RValue<Byte8> SubSat(RValue<Byte8> x, RValue<Byte8> y);
	RValue<Short4> Unpack(RValue<Byte4> x);
	RValue<Short4> UnpackLow(RValue<Byte8> x, RValue<Byte8> y);
	RValue<Short4> UnpackHigh(RValue<Byte8> x, RValue<Byte8> y);
	RValue<Int> SignMask(RValue<Byte8> x);
//	RValue<Byte8> CmpGT(RValue<Byte8> x, RValue<Byte8> y);
	RValue<Byte8> CmpEQ(RValue<Byte8> x, RValue<Byte8> y);

	class SByte8 : public LValue<SByte8>
	{
	public:
		SByte8() = default;
		SByte8(uint8_t x0, uint8_t x1, uint8_t x2, uint8_t x3, uint8_t x4, uint8_t x5, uint8_t x6, uint8_t x7);
		SByte8(RValue<SByte8> rhs);
		SByte8(const SByte8 &rhs);
		SByte8(const Reference<SByte8> &rhs);

		RValue<SByte8> operator=(RValue<SByte8> rhs);
		RValue<SByte8> operator=(const SByte8 &rhs);
		RValue<SByte8> operator=(const Reference<SByte8> &rhs);

		static Type *getType();
	};

	RValue<SByte8> operator+(RValue<SByte8> lhs, RValue<SByte8> rhs);
	RValue<SByte8> operator-(RValue<SByte8> lhs, RValue<SByte8> rhs);
//	RValue<SByte8> operator*(RValue<SByte8> lhs, RValue<SByte8> rhs);
//	RValue<SByte8> operator/(RValue<SByte8> lhs, RValue<SByte8> rhs);
//	RValue<SByte8> operator%(RValue<SByte8> lhs, RValue<SByte8> rhs);
	RValue<SByte8> operator&(RValue<SByte8> lhs, RValue<SByte8> rhs);
	RValue<SByte8> operator|(RValue<SByte8> lhs, RValue<SByte8> rhs);
	RValue<SByte8> operator^(RValue<SByte8> lhs, RValue<SByte8> rhs);
//	RValue<SByte8> operator<<(RValue<SByte8> lhs, RValue<SByte8> rhs);
//	RValue<SByte8> operator>>(RValue<SByte8> lhs, RValue<SByte8> rhs);
	RValue<SByte8> operator+=(SByte8 &lhs, RValue<SByte8> rhs);
	RValue<SByte8> operator-=(SByte8 &lhs, RValue<SByte8> rhs);
//	RValue<SByte8> operator*=(SByte8 &lhs, RValue<SByte8> rhs);
//	RValue<SByte8> operator/=(SByte8 &lhs, RValue<SByte8> rhs);
//	RValue<SByte8> operator%=(SByte8 &lhs, RValue<SByte8> rhs);
	RValue<SByte8> operator&=(SByte8 &lhs, RValue<SByte8> rhs);
	RValue<SByte8> operator|=(SByte8 &lhs, RValue<SByte8> rhs);
	RValue<SByte8> operator^=(SByte8 &lhs, RValue<SByte8> rhs);
//	RValue<SByte8> operator<<=(SByte8 &lhs, RValue<SByte8> rhs);
//	RValue<SByte8> operator>>=(SByte8 &lhs, RValue<SByte8> rhs);
//	RValue<SByte8> operator+(RValue<SByte8> val);
//	RValue<SByte8> operator-(RValue<SByte8> val);
	RValue<SByte8> operator~(RValue<SByte8> val);
//	RValue<SByte8> operator++(SByte8 &val, int);   // Post-increment
//	const SByte8 &operator++(SByte8 &val);   // Pre-increment
//	RValue<SByte8> operator--(SByte8 &val, int);   // Post-decrement
//	const SByte8 &operator--(SByte8 &val);   // Pre-decrement

	RValue<SByte8> AddSat(RValue<SByte8> x, RValue<SByte8> y);
	RValue<SByte8> SubSat(RValue<SByte8> x, RValue<SByte8> y);
	RValue<Short4> UnpackLow(RValue<SByte8> x, RValue<SByte8> y);
	RValue<Short4> UnpackHigh(RValue<SByte8> x, RValue<SByte8> y);
	RValue<Int> SignMask(RValue<SByte8> x);
	RValue<Byte8> CmpGT(RValue<SByte8> x, RValue<SByte8> y);
	RValue<Byte8> CmpEQ(RValue<SByte8> x, RValue<SByte8> y);

	class Byte16 : public LValue<Byte16>
	{
	public:
		Byte16() = default;
	//	Byte16(int x, int y, int z, int w);
		Byte16(RValue<Byte16> rhs);
		Byte16(const Byte16 &rhs);
		Byte16(const Reference<Byte16> &rhs);

		RValue<Byte16> operator=(RValue<Byte16> rhs);
		RValue<Byte16> operator=(const Byte16 &rhs);
		RValue<Byte16> operator=(const Reference<Byte16> &rhs);

		static Type *getType();
	};

//	RValue<Byte16> operator+(RValue<Byte16> lhs, RValue<Byte16> rhs);
//	RValue<Byte16> operator-(RValue<Byte16> lhs, RValue<Byte16> rhs);
//	RValue<Byte16> operator*(RValue<Byte16> lhs, RValue<Byte16> rhs);
//	RValue<Byte16> operator/(RValue<Byte16> lhs, RValue<Byte16> rhs);
//	RValue<Byte16> operator%(RValue<Byte16> lhs, RValue<Byte16> rhs);
//	RValue<Byte16> operator&(RValue<Byte16> lhs, RValue<Byte16> rhs);
//	RValue<Byte16> operator|(RValue<Byte16> lhs, RValue<Byte16> rhs);
//	RValue<Byte16> operator^(RValue<Byte16> lhs, RValue<Byte16> rhs);
//	RValue<Byte16> operator<<(RValue<Byte16> lhs, RValue<Byte16> rhs);
//	RValue<Byte16> operator>>(RValue<Byte16> lhs, RValue<Byte16> rhs);
//	RValue<Byte16> operator+=(Byte16 &lhs, RValue<Byte16> rhs);
//	RValue<Byte16> operator-=(Byte16 &lhs, RValue<Byte16> rhs);
//	RValue<Byte16> operator*=(Byte16 &lhs, RValue<Byte16> rhs);
//	RValue<Byte16> operator/=(Byte16 &lhs, RValue<Byte16> rhs);
//	RValue<Byte16> operator%=(Byte16 &lhs, RValue<Byte16> rhs);
//	RValue<Byte16> operator&=(Byte16 &lhs, RValue<Byte16> rhs);
//	RValue<Byte16> operator|=(Byte16 &lhs, RValue<Byte16> rhs);
//	RValue<Byte16> operator^=(Byte16 &lhs, RValue<Byte16> rhs);
//	RValue<Byte16> operator<<=(Byte16 &lhs, RValue<Byte16> rhs);
//	RValue<Byte16> operator>>=(Byte16 &lhs, RValue<Byte16> rhs);
//	RValue<Byte16> operator+(RValue<Byte16> val);
//	RValue<Byte16> operator-(RValue<Byte16> val);
//	RValue<Byte16> operator~(RValue<Byte16> val);
//	RValue<Byte16> operator++(Byte16 &val, int);   // Post-increment
//	const Byte16 &operator++(Byte16 &val);   // Pre-increment
//	RValue<Byte16> operator--(Byte16 &val, int);   // Post-decrement
//	const Byte16 &operator--(Byte16 &val);   // Pre-decrement

	class SByte16 : public LValue<SByte16>
	{
	public:
		SByte16() = default;
	//	SByte16(int x, int y, int z, int w);
	//	SByte16(RValue<SByte16> rhs);
	//	SByte16(const SByte16 &rhs);
	//	SByte16(const Reference<SByte16> &rhs);

	//	RValue<SByte16> operator=(RValue<SByte16> rhs);
	//	RValue<SByte16> operator=(const SByte16 &rhs);
	//	RValue<SByte16> operator=(const Reference<SByte16> &rhs);

		static Type *getType();
	};

//	RValue<SByte16> operator+(RValue<SByte16> lhs, RValue<SByte16> rhs);
//	RValue<SByte16> operator-(RValue<SByte16> lhs, RValue<SByte16> rhs);
//	RValue<SByte16> operator*(RValue<SByte16> lhs, RValue<SByte16> rhs);
//	RValue<SByte16> operator/(RValue<SByte16> lhs, RValue<SByte16> rhs);
//	RValue<SByte16> operator%(RValue<SByte16> lhs, RValue<SByte16> rhs);
//	RValue<SByte16> operator&(RValue<SByte16> lhs, RValue<SByte16> rhs);
//	RValue<SByte16> operator|(RValue<SByte16> lhs, RValue<SByte16> rhs);
//	RValue<SByte16> operator^(RValue<SByte16> lhs, RValue<SByte16> rhs);
//	RValue<SByte16> operator<<(RValue<SByte16> lhs, RValue<SByte16> rhs);
//	RValue<SByte16> operator>>(RValue<SByte16> lhs, RValue<SByte16> rhs);
//	RValue<SByte16> operator+=(SByte16 &lhs, RValue<SByte16> rhs);
//	RValue<SByte16> operator-=(SByte16 &lhs, RValue<SByte16> rhs);
//	RValue<SByte16> operator*=(SByte16 &lhs, RValue<SByte16> rhs);
//	RValue<SByte16> operator/=(SByte16 &lhs, RValue<SByte16> rhs);
//	RValue<SByte16> operator%=(SByte16 &lhs, RValue<SByte16> rhs);
//	RValue<SByte16> operator&=(SByte16 &lhs, RValue<SByte16> rhs);
//	RValue<SByte16> operator|=(SByte16 &lhs, RValue<SByte16> rhs);
//	RValue<SByte16> operator^=(SByte16 &lhs, RValue<SByte16> rhs);
//	RValue<SByte16> operator<<=(SByte16 &lhs, RValue<SByte16> rhs);
//	RValue<SByte16> operator>>=(SByte16 &lhs, RValue<SByte16> rhs);
//	RValue<SByte16> operator+(RValue<SByte16> val);
//	RValue<SByte16> operator-(RValue<SByte16> val);
//	RValue<SByte16> operator~(RValue<SByte16> val);
//	RValue<SByte16> operator++(SByte16 &val, int);   // Post-increment
//	const SByte16 &operator++(SByte16 &val);   // Pre-increment
//	RValue<SByte16> operator--(SByte16 &val, int);   // Post-decrement
//	const SByte16 &operator--(SByte16 &val);   // Pre-decrement

	class Short2 : public LValue<Short2>
	{
	public:
		explicit Short2(RValue<Short4> cast);

		static Type *getType();
	};

	class UShort2 : public LValue<UShort2>
	{
	public:
		explicit UShort2(RValue<UShort4> cast);

		static Type *getType();
	};

	class Short4 : public LValue<Short4>
	{
	public:
		explicit Short4(RValue<Int> cast);
		explicit Short4(RValue<Int4> cast);
	//	explicit Short4(RValue<Float> cast);
		explicit Short4(RValue<Float4> cast);

		Short4() = default;
		Short4(short xyzw);
		Short4(short x, short y, short z, short w);
		Short4(RValue<Short4> rhs);
		Short4(const Short4 &rhs);
		Short4(const Reference<Short4> &rhs);
		Short4(RValue<UShort4> rhs);
		Short4(const UShort4 &rhs);
		Short4(const Reference<UShort4> &rhs);

		RValue<Short4> operator=(RValue<Short4> rhs);
		RValue<Short4> operator=(const Short4 &rhs);
		RValue<Short4> operator=(const Reference<Short4> &rhs);
		RValue<Short4> operator=(RValue<UShort4> rhs);
		RValue<Short4> operator=(const UShort4 &rhs);
		RValue<Short4> operator=(const Reference<UShort4> &rhs);

		static Type *getType();
	};

	RValue<Short4> operator+(RValue<Short4> lhs, RValue<Short4> rhs);
	RValue<Short4> operator-(RValue<Short4> lhs, RValue<Short4> rhs);
	RValue<Short4> operator*(RValue<Short4> lhs, RValue<Short4> rhs);
//	RValue<Short4> operator/(RValue<Short4> lhs, RValue<Short4> rhs);
//	RValue<Short4> operator%(RValue<Short4> lhs, RValue<Short4> rhs);
	RValue<Short4> operator&(RValue<Short4> lhs, RValue<Short4> rhs);
	RValue<Short4> operator|(RValue<Short4> lhs, RValue<Short4> rhs);
	RValue<Short4> operator^(RValue<Short4> lhs, RValue<Short4> rhs);
	RValue<Short4> operator<<(RValue<Short4> lhs, unsigned char rhs);
	RValue<Short4> operator>>(RValue<Short4> lhs, unsigned char rhs);
	RValue<Short4> operator+=(Short4 &lhs, RValue<Short4> rhs);
	RValue<Short4> operator-=(Short4 &lhs, RValue<Short4> rhs);
	RValue<Short4> operator*=(Short4 &lhs, RValue<Short4> rhs);
//	RValue<Short4> operator/=(Short4 &lhs, RValue<Short4> rhs);
//	RValue<Short4> operator%=(Short4 &lhs, RValue<Short4> rhs);
	RValue<Short4> operator&=(Short4 &lhs, RValue<Short4> rhs);
	RValue<Short4> operator|=(Short4 &lhs, RValue<Short4> rhs);
	RValue<Short4> operator^=(Short4 &lhs, RValue<Short4> rhs);
	RValue<Short4> operator<<=(Short4 &lhs, unsigned char rhs);
	RValue<Short4> operator>>=(Short4 &lhs, unsigned char rhs);
//	RValue<Short4> operator+(RValue<Short4> val);
	RValue<Short4> operator-(RValue<Short4> val);
	RValue<Short4> operator~(RValue<Short4> val);
//	RValue<Short4> operator++(Short4 &val, int);   // Post-increment
//	const Short4 &operator++(Short4 &val);   // Pre-increment
//	RValue<Short4> operator--(Short4 &val, int);   // Post-decrement
//	const Short4 &operator--(Short4 &val);   // Pre-decrement
//	RValue<Bool> operator<(RValue<Short4> lhs, RValue<Short4> rhs);
//	RValue<Bool> operator<=(RValue<Short4> lhs, RValue<Short4> rhs);
//	RValue<Bool> operator>(RValue<Short4> lhs, RValue<Short4> rhs);
//	RValue<Bool> operator>=(RValue<Short4> lhs, RValue<Short4> rhs);
//	RValue<Bool> operator!=(RValue<Short4> lhs, RValue<Short4> rhs);
//	RValue<Bool> operator==(RValue<Short4> lhs, RValue<Short4> rhs);

	RValue<Short4> RoundShort4(RValue<Float4> cast);
	RValue<Short4> Max(RValue<Short4> x, RValue<Short4> y);
	RValue<Short4> Min(RValue<Short4> x, RValue<Short4> y);
	RValue<Short4> AddSat(RValue<Short4> x, RValue<Short4> y);
	RValue<Short4> SubSat(RValue<Short4> x, RValue<Short4> y);
	RValue<Short4> MulHigh(RValue<Short4> x, RValue<Short4> y);
	RValue<Int2> MulAdd(RValue<Short4> x, RValue<Short4> y);
	RValue<SByte8> Pack(RValue<Short4> x, RValue<Short4> y);
	RValue<Int2> UnpackLow(RValue<Short4> x, RValue<Short4> y);
	RValue<Int2> UnpackHigh(RValue<Short4> x, RValue<Short4> y);
	RValue<Short4> Swizzle(RValue<Short4> x, unsigned char select);
	RValue<Short4> Insert(RValue<Short4> val, RValue<Short> element, int i);
	RValue<Short> Extract(RValue<Short4> val, int i);
	RValue<Short4> CmpGT(RValue<Short4> x, RValue<Short4> y);
	RValue<Short4> CmpEQ(RValue<Short4> x, RValue<Short4> y);

	class UShort4 : public LValue<UShort4>
	{
	public:
		explicit UShort4(RValue<Int4> cast);
		explicit UShort4(RValue<Float4> cast, bool saturate = false);

		UShort4() = default;
		UShort4(unsigned short xyzw);
		UShort4(unsigned short x, unsigned short y, unsigned short z, unsigned short w);
		UShort4(RValue<UShort4> rhs);
		UShort4(const UShort4 &rhs);
		UShort4(const Reference<UShort4> &rhs);
		UShort4(RValue<Short4> rhs);
		UShort4(const Short4 &rhs);
		UShort4(const Reference<Short4> &rhs);

		RValue<UShort4> operator=(RValue<UShort4> rhs);
		RValue<UShort4> operator=(const UShort4 &rhs);
		RValue<UShort4> operator=(const Reference<UShort4> &rhs);
		RValue<UShort4> operator=(RValue<Short4> rhs);
		RValue<UShort4> operator=(const Short4 &rhs);
		RValue<UShort4> operator=(const Reference<Short4> &rhs);

		static Type *getType();
	};

	RValue<UShort4> operator+(RValue<UShort4> lhs, RValue<UShort4> rhs);
	RValue<UShort4> operator-(RValue<UShort4> lhs, RValue<UShort4> rhs);
	RValue<UShort4> operator*(RValue<UShort4> lhs, RValue<UShort4> rhs);
//	RValue<UShort4> operator/(RValue<UShort4> lhs, RValue<UShort4> rhs);
//	RValue<UShort4> operator%(RValue<UShort4> lhs, RValue<UShort4> rhs);
	RValue<UShort4> operator&(RValue<UShort4> lhs, RValue<UShort4> rhs);
	RValue<UShort4> operator|(RValue<UShort4> lhs, RValue<UShort4> rhs);
	RValue<UShort4> operator^(RValue<UShort4> lhs, RValue<UShort4> rhs);
	RValue<UShort4> operator<<(RValue<UShort4> lhs, unsigned char rhs);
	RValue<UShort4> operator>>(RValue<UShort4> lhs, unsigned char rhs);
//	RValue<UShort4> operator+=(UShort4 &lhs, RValue<UShort4> rhs);
//	RValue<UShort4> operator-=(UShort4 &lhs, RValue<UShort4> rhs);
//	RValue<UShort4> operator*=(UShort4 &lhs, RValue<UShort4> rhs);
//	RValue<UShort4> operator/=(UShort4 &lhs, RValue<UShort4> rhs);
//	RValue<UShort4> operator%=(UShort4 &lhs, RValue<UShort4> rhs);
//	RValue<UShort4> operator&=(UShort4 &lhs, RValue<UShort4> rhs);
//	RValue<UShort4> operator|=(UShort4 &lhs, RValue<UShort4> rhs);
//	RValue<UShort4> operator^=(UShort4 &lhs, RValue<UShort4> rhs);
	RValue<UShort4> operator<<=(UShort4 &lhs, unsigned char rhs);
	RValue<UShort4> operator>>=(UShort4 &lhs, unsigned char rhs);
//	RValue<UShort4> operator+(RValue<UShort4> val);
//	RValue<UShort4> operator-(RValue<UShort4> val);
	RValue<UShort4> operator~(RValue<UShort4> val);
//	RValue<UShort4> operator++(UShort4 &val, int);   // Post-increment
//	const UShort4 &operator++(UShort4 &val);   // Pre-increment
//	RValue<UShort4> operator--(UShort4 &val, int);   // Post-decrement
//	const UShort4 &operator--(UShort4 &val);   // Pre-decrement

	RValue<UShort4> Max(RValue<UShort4> x, RValue<UShort4> y);
	RValue<UShort4> Min(RValue<UShort4> x, RValue<UShort4> y);
	RValue<UShort4> AddSat(RValue<UShort4> x, RValue<UShort4> y);
	RValue<UShort4> SubSat(RValue<UShort4> x, RValue<UShort4> y);
	RValue<UShort4> MulHigh(RValue<UShort4> x, RValue<UShort4> y);
	RValue<UShort4> Average(RValue<UShort4> x, RValue<UShort4> y);
	RValue<Byte8> Pack(RValue<UShort4> x, RValue<UShort4> y);

	class Short8 : public LValue<Short8>
	{
	public:
		Short8() = default;
		Short8(short c);
		Short8(short c0, short c1, short c2, short c3, short c4, short c5, short c6, short c7);
		Short8(RValue<Short8> rhs);
	//	Short8(const Short8 &rhs);
		Short8(const Reference<Short8> &rhs);
		Short8(RValue<Short4> lo, RValue<Short4> hi);

	//	RValue<Short8> operator=(RValue<Short8> rhs);
	//	RValue<Short8> operator=(const Short8 &rhs);
	//	RValue<Short8> operator=(const Reference<Short8> &rhs);

		static Type *getType();
	};

	RValue<Short8> operator+(RValue<Short8> lhs, RValue<Short8> rhs);
//	RValue<Short8> operator-(RValue<Short8> lhs, RValue<Short8> rhs);
//	RValue<Short8> operator*(RValue<Short8> lhs, RValue<Short8> rhs);
//	RValue<Short8> operator/(RValue<Short8> lhs, RValue<Short8> rhs);
//	RValue<Short8> operator%(RValue<Short8> lhs, RValue<Short8> rhs);
	RValue<Short8> operator&(RValue<Short8> lhs, RValue<Short8> rhs);
//	RValue<Short8> operator|(RValue<Short8> lhs, RValue<Short8> rhs);
//	RValue<Short8> operator^(RValue<Short8> lhs, RValue<Short8> rhs);
	RValue<Short8> operator<<(RValue<Short8> lhs, unsigned char rhs);
	RValue<Short8> operator>>(RValue<Short8> lhs, unsigned char rhs);
//	RValue<Short8> operator<<(RValue<Short8> lhs, RValue<Short8> rhs);
//	RValue<Short8> operator>>(RValue<Short8> lhs, RValue<Short8> rhs);
//	RValue<Short8> operator+=(Short8 &lhs, RValue<Short8> rhs);
//	RValue<Short8> operator-=(Short8 &lhs, RValue<Short8> rhs);
//	RValue<Short8> operator*=(Short8 &lhs, RValue<Short8> rhs);
//	RValue<Short8> operator/=(Short8 &lhs, RValue<Short8> rhs);
//	RValue<Short8> operator%=(Short8 &lhs, RValue<Short8> rhs);
//	RValue<Short8> operator&=(Short8 &lhs, RValue<Short8> rhs);
//	RValue<Short8> operator|=(Short8 &lhs, RValue<Short8> rhs);
//	RValue<Short8> operator^=(Short8 &lhs, RValue<Short8> rhs);
//	RValue<Short8> operator<<=(Short8 &lhs, RValue<Short8> rhs);
//	RValue<Short8> operator>>=(Short8 &lhs, RValue<Short8> rhs);
//	RValue<Short8> operator+(RValue<Short8> val);
//	RValue<Short8> operator-(RValue<Short8> val);
//	RValue<Short8> operator~(RValue<Short8> val);
//	RValue<Short8> operator++(Short8 &val, int);   // Post-increment
//	const Short8 &operator++(Short8 &val);   // Pre-increment
//	RValue<Short8> operator--(Short8 &val, int);   // Post-decrement
//	const Short8 &operator--(Short8 &val);   // Pre-decrement
//	RValue<Bool> operator<(RValue<Short8> lhs, RValue<Short8> rhs);
//	RValue<Bool> operator<=(RValue<Short8> lhs, RValue<Short8> rhs);
//	RValue<Bool> operator>(RValue<Short8> lhs, RValue<Short8> rhs);
//	RValue<Bool> operator>=(RValue<Short8> lhs, RValue<Short8> rhs);
//	RValue<Bool> operator!=(RValue<Short8> lhs, RValue<Short8> rhs);
//	RValue<Bool> operator==(RValue<Short8> lhs, RValue<Short8> rhs);

	RValue<Short8> MulHigh(RValue<Short8> x, RValue<Short8> y);
	RValue<Int4> MulAdd(RValue<Short8> x, RValue<Short8> y);
	RValue<Int4> Abs(RValue<Int4> x);

	class UShort8 : public LValue<UShort8>
	{
	public:
		UShort8() = default;
		UShort8(unsigned short c);
		UShort8(unsigned short c0, unsigned short c1, unsigned short c2, unsigned short c3, unsigned short c4, unsigned short c5, unsigned short c6, unsigned short c7);
		UShort8(RValue<UShort8> rhs);
	//	UShort8(const UShort8 &rhs);
		UShort8(const Reference<UShort8> &rhs);
		UShort8(RValue<UShort4> lo, RValue<UShort4> hi);

		RValue<UShort8> operator=(RValue<UShort8> rhs);
		RValue<UShort8> operator=(const UShort8 &rhs);
		RValue<UShort8> operator=(const Reference<UShort8> &rhs);

		static Type *getType();
	};

	RValue<UShort8> operator+(RValue<UShort8> lhs, RValue<UShort8> rhs);
//	RValue<UShort8> operator-(RValue<UShort8> lhs, RValue<UShort8> rhs);
	RValue<UShort8> operator*(RValue<UShort8> lhs, RValue<UShort8> rhs);
//	RValue<UShort8> operator/(RValue<UShort8> lhs, RValue<UShort8> rhs);
//	RValue<UShort8> operator%(RValue<UShort8> lhs, RValue<UShort8> rhs);
	RValue<UShort8> operator&(RValue<UShort8> lhs, RValue<UShort8> rhs);
//	RValue<UShort8> operator|(RValue<UShort8> lhs, RValue<UShort8> rhs);
//	RValue<UShort8> operator^(RValue<UShort8> lhs, RValue<UShort8> rhs);
	RValue<UShort8> operator<<(RValue<UShort8> lhs, unsigned char rhs);
	RValue<UShort8> operator>>(RValue<UShort8> lhs, unsigned char rhs);
//	RValue<UShort8> operator<<(RValue<UShort8> lhs, RValue<UShort8> rhs);
//	RValue<UShort8> operator>>(RValue<UShort8> lhs, RValue<UShort8> rhs);
	RValue<UShort8> operator+=(UShort8 &lhs, RValue<UShort8> rhs);
//	RValue<UShort8> operator-=(UShort8 &lhs, RValue<UShort8> rhs);
//	RValue<UShort8> operator*=(UShort8 &lhs, RValue<UShort8> rhs);
//	RValue<UShort8> operator/=(UShort8 &lhs, RValue<UShort8> rhs);
//	RValue<UShort8> operator%=(UShort8 &lhs, RValue<UShort8> rhs);
//	RValue<UShort8> operator&=(UShort8 &lhs, RValue<UShort8> rhs);
//	RValue<UShort8> operator|=(UShort8 &lhs, RValue<UShort8> rhs);
//	RValue<UShort8> operator^=(UShort8 &lhs, RValue<UShort8> rhs);
//	RValue<UShort8> operator<<=(UShort8 &lhs, RValue<UShort8> rhs);
//	RValue<UShort8> operator>>=(UShort8 &lhs, RValue<UShort8> rhs);
//	RValue<UShort8> operator+(RValue<UShort8> val);
//	RValue<UShort8> operator-(RValue<UShort8> val);
	RValue<UShort8> operator~(RValue<UShort8> val);
//	RValue<UShort8> operator++(UShort8 &val, int);   // Post-increment
//	const UShort8 &operator++(UShort8 &val);   // Pre-increment
//	RValue<UShort8> operator--(UShort8 &val, int);   // Post-decrement
//	const UShort8 &operator--(UShort8 &val);   // Pre-decrement
//	RValue<Bool> operator<(RValue<UShort8> lhs, RValue<UShort8> rhs);
//	RValue<Bool> operator<=(RValue<UShort8> lhs, RValue<UShort8> rhs);
//	RValue<Bool> operator>(RValue<UShort8> lhs, RValue<UShort8> rhs);
//	RValue<Bool> operator>=(RValue<UShort8> lhs, RValue<UShort8> rhs);
//	RValue<Bool> operator!=(RValue<UShort8> lhs, RValue<UShort8> rhs);
//	RValue<Bool> operator==(RValue<UShort8> lhs, RValue<UShort8> rhs);

	RValue<UShort8> Swizzle(RValue<UShort8> x, char select0, char select1, char select2, char select3, char select4, char select5, char select6, char select7);
	RValue<UShort8> MulHigh(RValue<UShort8> x, RValue<UShort8> y);

	class Int : public LValue<Int>
	{
	public:
		Int(Argument<Int> argument);

		explicit Int(RValue<Byte> cast);
		explicit Int(RValue<SByte> cast);
		explicit Int(RValue<Short> cast);
		explicit Int(RValue<UShort> cast);
		explicit Int(RValue<Int2> cast);
		explicit Int(RValue<Long> cast);
		explicit Int(RValue<Float> cast);

		Int() = default;
		Int(int x);
		Int(RValue<Int> rhs);
		Int(RValue<UInt> rhs);
		Int(const Int &rhs);
		Int(const UInt &rhs);
		Int(const Reference<Int> &rhs);
		Int(const Reference<UInt> &rhs);

		RValue<Int> operator=(int rhs);
		RValue<Int> operator=(RValue<Int> rhs);
		RValue<Int> operator=(RValue<UInt> rhs);
		RValue<Int> operator=(const Int &rhs);
		RValue<Int> operator=(const UInt &rhs);
		RValue<Int> operator=(const Reference<Int> &rhs);
		RValue<Int> operator=(const Reference<UInt> &rhs);

		static Type *getType();
	};

	RValue<Int> operator+(RValue<Int> lhs, RValue<Int> rhs);
	RValue<Int> operator-(RValue<Int> lhs, RValue<Int> rhs);
	RValue<Int> operator*(RValue<Int> lhs, RValue<Int> rhs);
	RValue<Int> operator/(RValue<Int> lhs, RValue<Int> rhs);
	RValue<Int> operator%(RValue<Int> lhs, RValue<Int> rhs);
	RValue<Int> operator&(RValue<Int> lhs, RValue<Int> rhs);
	RValue<Int> operator|(RValue<Int> lhs, RValue<Int> rhs);
	RValue<Int> operator^(RValue<Int> lhs, RValue<Int> rhs);
	RValue<Int> operator<<(RValue<Int> lhs, RValue<Int> rhs);
	RValue<Int> operator>>(RValue<Int> lhs, RValue<Int> rhs);
	RValue<Int> operator+=(Int &lhs, RValue<Int> rhs);
	RValue<Int> operator-=(Int &lhs, RValue<Int> rhs);
	RValue<Int> operator*=(Int &lhs, RValue<Int> rhs);
	RValue<Int> operator/=(Int &lhs, RValue<Int> rhs);
	RValue<Int> operator%=(Int &lhs, RValue<Int> rhs);
	RValue<Int> operator&=(Int &lhs, RValue<Int> rhs);
	RValue<Int> operator|=(Int &lhs, RValue<Int> rhs);
	RValue<Int> operator^=(Int &lhs, RValue<Int> rhs);
	RValue<Int> operator<<=(Int &lhs, RValue<Int> rhs);
	RValue<Int> operator>>=(Int &lhs, RValue<Int> rhs);
	RValue<Int> operator+(RValue<Int> val);
	RValue<Int> operator-(RValue<Int> val);
	RValue<Int> operator~(RValue<Int> val);
	RValue<Int> operator++(Int &val, int);   // Post-increment
	const Int &operator++(Int &val);   // Pre-increment
	RValue<Int> operator--(Int &val, int);   // Post-decrement
	const Int &operator--(Int &val);   // Pre-decrement
	RValue<Bool> operator<(RValue<Int> lhs, RValue<Int> rhs);
	RValue<Bool> operator<=(RValue<Int> lhs, RValue<Int> rhs);
	RValue<Bool> operator>(RValue<Int> lhs, RValue<Int> rhs);
	RValue<Bool> operator>=(RValue<Int> lhs, RValue<Int> rhs);
	RValue<Bool> operator!=(RValue<Int> lhs, RValue<Int> rhs);
	RValue<Bool> operator==(RValue<Int> lhs, RValue<Int> rhs);

	RValue<Int> Max(RValue<Int> x, RValue<Int> y);
	RValue<Int> Min(RValue<Int> x, RValue<Int> y);
	RValue<Int> Clamp(RValue<Int> x, RValue<Int> min, RValue<Int> max);
	RValue<Int> RoundInt(RValue<Float> cast);

	class Long : public LValue<Long>
	{
	public:
	//	Long(Argument<Long> argument);

	//	explicit Long(RValue<Short> cast);
	//	explicit Long(RValue<UShort> cast);
		explicit Long(RValue<Int> cast);
		explicit Long(RValue<UInt> cast);
	//	explicit Long(RValue<Float> cast);

		Long() = default;
	//	Long(qword x);
		Long(RValue<Long> rhs);
	//	Long(RValue<ULong> rhs);
	//	Long(const Long &rhs);
	//	Long(const Reference<Long> &rhs);
	//	Long(const ULong &rhs);
	//	Long(const Reference<ULong> &rhs);

		RValue<Long> operator=(int64_t rhs);
		RValue<Long> operator=(RValue<Long> rhs);
	//	RValue<Long> operator=(RValue<ULong> rhs);
		RValue<Long> operator=(const Long &rhs);
		RValue<Long> operator=(const Reference<Long> &rhs);
	//	RValue<Long> operator=(const ULong &rhs);
	//	RValue<Long> operator=(const Reference<ULong> &rhs);

		static Type *getType();
	};

	RValue<Long> operator+(RValue<Long> lhs, RValue<Long> rhs);
	RValue<Long> operator-(RValue<Long> lhs, RValue<Long> rhs);
//	RValue<Long> operator*(RValue<Long> lhs, RValue<Long> rhs);
//	RValue<Long> operator/(RValue<Long> lhs, RValue<Long> rhs);
//	RValue<Long> operator%(RValue<Long> lhs, RValue<Long> rhs);
//	RValue<Long> operator&(RValue<Long> lhs, RValue<Long> rhs);
//	RValue<Long> operator|(RValue<Long> lhs, RValue<Long> rhs);
//	RValue<Long> operator^(RValue<Long> lhs, RValue<Long> rhs);
//	RValue<Long> operator<<(RValue<Long> lhs, RValue<Long> rhs);
//	RValue<Long> operator>>(RValue<Long> lhs, RValue<Long> rhs);
	RValue<Long> operator+=(Long &lhs, RValue<Long> rhs);
	RValue<Long> operator-=(Long &lhs, RValue<Long> rhs);
//	RValue<Long> operator*=(Long &lhs, RValue<Long> rhs);
//	RValue<Long> operator/=(Long &lhs, RValue<Long> rhs);
//	RValue<Long> operator%=(Long &lhs, RValue<Long> rhs);
//	RValue<Long> operator&=(Long &lhs, RValue<Long> rhs);
//	RValue<Long> operator|=(Long &lhs, RValue<Long> rhs);
//	RValue<Long> operator^=(Long &lhs, RValue<Long> rhs);
//	RValue<Long> operator<<=(Long &lhs, RValue<Long> rhs);
//	RValue<Long> operator>>=(Long &lhs, RValue<Long> rhs);
//	RValue<Long> operator+(RValue<Long> val);
//	RValue<Long> operator-(RValue<Long> val);
//	RValue<Long> operator~(RValue<Long> val);
//	RValue<Long> operator++(Long &val, int);   // Post-increment
//	const Long &operator++(Long &val);   // Pre-increment
//	RValue<Long> operator--(Long &val, int);   // Post-decrement
//	const Long &operator--(Long &val);   // Pre-decrement
//	RValue<Bool> operator<(RValue<Long> lhs, RValue<Long> rhs);
//	RValue<Bool> operator<=(RValue<Long> lhs, RValue<Long> rhs);
//	RValue<Bool> operator>(RValue<Long> lhs, RValue<Long> rhs);
//	RValue<Bool> operator>=(RValue<Long> lhs, RValue<Long> rhs);
//	RValue<Bool> operator!=(RValue<Long> lhs, RValue<Long> rhs);
//	RValue<Bool> operator==(RValue<Long> lhs, RValue<Long> rhs);

//	RValue<Long> RoundLong(RValue<Float> cast);
	RValue<Long> AddAtomic( RValue<Pointer<Long>> x, RValue<Long> y);

	class UInt : public LValue<UInt>
	{
	public:
		UInt(Argument<UInt> argument);

		explicit UInt(RValue<UShort> cast);
		explicit UInt(RValue<Long> cast);
		explicit UInt(RValue<Float> cast);

		UInt() = default;
		UInt(int x);
		UInt(unsigned int x);
		UInt(RValue<UInt> rhs);
		UInt(RValue<Int> rhs);
		UInt(const UInt &rhs);
		UInt(const Int &rhs);
		UInt(const Reference<UInt> &rhs);
		UInt(const Reference<Int> &rhs);

		RValue<UInt> operator=(unsigned int rhs);
		RValue<UInt> operator=(RValue<UInt> rhs);
		RValue<UInt> operator=(RValue<Int> rhs);
		RValue<UInt> operator=(const UInt &rhs);
		RValue<UInt> operator=(const Int &rhs);
		RValue<UInt> operator=(const Reference<UInt> &rhs);
		RValue<UInt> operator=(const Reference<Int> &rhs);

		static Type *getType();
	};

	RValue<UInt> operator+(RValue<UInt> lhs, RValue<UInt> rhs);
	RValue<UInt> operator-(RValue<UInt> lhs, RValue<UInt> rhs);
	RValue<UInt> operator*(RValue<UInt> lhs, RValue<UInt> rhs);
	RValue<UInt> operator/(RValue<UInt> lhs, RValue<UInt> rhs);
	RValue<UInt> operator%(RValue<UInt> lhs, RValue<UInt> rhs);
	RValue<UInt> operator&(RValue<UInt> lhs, RValue<UInt> rhs);
	RValue<UInt> operator|(RValue<UInt> lhs, RValue<UInt> rhs);
	RValue<UInt> operator^(RValue<UInt> lhs, RValue<UInt> rhs);
	RValue<UInt> operator<<(RValue<UInt> lhs, RValue<UInt> rhs);
	RValue<UInt> operator>>(RValue<UInt> lhs, RValue<UInt> rhs);
	RValue<UInt> operator+=(UInt &lhs, RValue<UInt> rhs);
	RValue<UInt> operator-=(UInt &lhs, RValue<UInt> rhs);
	RValue<UInt> operator*=(UInt &lhs, RValue<UInt> rhs);
	RValue<UInt> operator/=(UInt &lhs, RValue<UInt> rhs);
	RValue<UInt> operator%=(UInt &lhs, RValue<UInt> rhs);
	RValue<UInt> operator&=(UInt &lhs, RValue<UInt> rhs);
	RValue<UInt> operator|=(UInt &lhs, RValue<UInt> rhs);
	RValue<UInt> operator^=(UInt &lhs, RValue<UInt> rhs);
	RValue<UInt> operator<<=(UInt &lhs, RValue<UInt> rhs);
	RValue<UInt> operator>>=(UInt &lhs, RValue<UInt> rhs);
	RValue<UInt> operator+(RValue<UInt> val);
	RValue<UInt> operator-(RValue<UInt> val);
	RValue<UInt> operator~(RValue<UInt> val);
	RValue<UInt> operator++(UInt &val, int);   // Post-increment
	const UInt &operator++(UInt &val);   // Pre-increment
	RValue<UInt> operator--(UInt &val, int);   // Post-decrement
	const UInt &operator--(UInt &val);   // Pre-decrement
	RValue<Bool> operator<(RValue<UInt> lhs, RValue<UInt> rhs);
	RValue<Bool> operator<=(RValue<UInt> lhs, RValue<UInt> rhs);
	RValue<Bool> operator>(RValue<UInt> lhs, RValue<UInt> rhs);
	RValue<Bool> operator>=(RValue<UInt> lhs, RValue<UInt> rhs);
	RValue<Bool> operator!=(RValue<UInt> lhs, RValue<UInt> rhs);
	RValue<Bool> operator==(RValue<UInt> lhs, RValue<UInt> rhs);

	RValue<UInt> Max(RValue<UInt> x, RValue<UInt> y);
	RValue<UInt> Min(RValue<UInt> x, RValue<UInt> y);
	RValue<UInt> Clamp(RValue<UInt> x, RValue<UInt> min, RValue<UInt> max);
//	RValue<UInt> RoundUInt(RValue<Float> cast);

	class Int2 : public LValue<Int2>
	{
	public:
	//	explicit Int2(RValue<Int> cast);
		explicit Int2(RValue<Int4> cast);

		Int2() = default;
		Int2(int x, int y);
		Int2(RValue<Int2> rhs);
		Int2(const Int2 &rhs);
		Int2(const Reference<Int2> &rhs);
		Int2(RValue<Int> lo, RValue<Int> hi);

		RValue<Int2> operator=(RValue<Int2> rhs);
		RValue<Int2> operator=(const Int2 &rhs);
		RValue<Int2> operator=(const Reference<Int2> &rhs);

		static Type *getType();
	};

	RValue<Int2> operator+(RValue<Int2> lhs, RValue<Int2> rhs);
	RValue<Int2> operator-(RValue<Int2> lhs, RValue<Int2> rhs);
//	RValue<Int2> operator*(RValue<Int2> lhs, RValue<Int2> rhs);
//	RValue<Int2> operator/(RValue<Int2> lhs, RValue<Int2> rhs);
//	RValue<Int2> operator%(RValue<Int2> lhs, RValue<Int2> rhs);
	RValue<Int2> operator&(RValue<Int2> lhs, RValue<Int2> rhs);
	RValue<Int2> operator|(RValue<Int2> lhs, RValue<Int2> rhs);
	RValue<Int2> operator^(RValue<Int2> lhs, RValue<Int2> rhs);
	RValue<Int2> operator<<(RValue<Int2> lhs, unsigned char rhs);
	RValue<Int2> operator>>(RValue<Int2> lhs, unsigned char rhs);
	RValue<Int2> operator+=(Int2 &lhs, RValue<Int2> rhs);
	RValue<Int2> operator-=(Int2 &lhs, RValue<Int2> rhs);
//	RValue<Int2> operator*=(Int2 &lhs, RValue<Int2> rhs);
//	RValue<Int2> operator/=(Int2 &lhs, RValue<Int2> rhs);
//	RValue<Int2> operator%=(Int2 &lhs, RValue<Int2> rhs);
	RValue<Int2> operator&=(Int2 &lhs, RValue<Int2> rhs);
	RValue<Int2> operator|=(Int2 &lhs, RValue<Int2> rhs);
	RValue<Int2> operator^=(Int2 &lhs, RValue<Int2> rhs);
	RValue<Int2> operator<<=(Int2 &lhs, unsigned char rhs);
	RValue<Int2> operator>>=(Int2 &lhs, unsigned char rhs);
//	RValue<Int2> operator+(RValue<Int2> val);
//	RValue<Int2> operator-(RValue<Int2> val);
	RValue<Int2> operator~(RValue<Int2> val);
//	RValue<Int2> operator++(Int2 &val, int);   // Post-increment
//	const Int2 &operator++(Int2 &val);   // Pre-increment
//	RValue<Int2> operator--(Int2 &val, int);   // Post-decrement
//	const Int2 &operator--(Int2 &val);   // Pre-decrement
//	RValue<Bool> operator<(RValue<Int2> lhs, RValue<Int2> rhs);
//	RValue<Bool> operator<=(RValue<Int2> lhs, RValue<Int2> rhs);
//	RValue<Bool> operator>(RValue<Int2> lhs, RValue<Int2> rhs);
//	RValue<Bool> operator>=(RValue<Int2> lhs, RValue<Int2> rhs);
//	RValue<Bool> operator!=(RValue<Int2> lhs, RValue<Int2> rhs);
//	RValue<Bool> operator==(RValue<Int2> lhs, RValue<Int2> rhs);

//	RValue<Int2> RoundInt(RValue<Float4> cast);
	RValue<Short4> UnpackLow(RValue<Int2> x, RValue<Int2> y);
	RValue<Short4> UnpackHigh(RValue<Int2> x, RValue<Int2> y);
	RValue<Int> Extract(RValue<Int2> val, int i);
	RValue<Int2> Insert(RValue<Int2> val, RValue<Int> element, int i);

	class UInt2 : public LValue<UInt2>
	{
	public:
		UInt2() = default;
		UInt2(unsigned int x, unsigned int y);
		UInt2(RValue<UInt2> rhs);
		UInt2(const UInt2 &rhs);
		UInt2(const Reference<UInt2> &rhs);

		RValue<UInt2> operator=(RValue<UInt2> rhs);
		RValue<UInt2> operator=(const UInt2 &rhs);
		RValue<UInt2> operator=(const Reference<UInt2> &rhs);

		static Type *getType();
	};

	RValue<UInt2> operator+(RValue<UInt2> lhs, RValue<UInt2> rhs);
	RValue<UInt2> operator-(RValue<UInt2> lhs, RValue<UInt2> rhs);
//	RValue<UInt2> operator*(RValue<UInt2> lhs, RValue<UInt2> rhs);
//	RValue<UInt2> operator/(RValue<UInt2> lhs, RValue<UInt2> rhs);
//	RValue<UInt2> operator%(RValue<UInt2> lhs, RValue<UInt2> rhs);
	RValue<UInt2> operator&(RValue<UInt2> lhs, RValue<UInt2> rhs);
	RValue<UInt2> operator|(RValue<UInt2> lhs, RValue<UInt2> rhs);
	RValue<UInt2> operator^(RValue<UInt2> lhs, RValue<UInt2> rhs);
	RValue<UInt2> operator<<(RValue<UInt2> lhs, unsigned char rhs);
	RValue<UInt2> operator>>(RValue<UInt2> lhs, unsigned char rhs);
	RValue<UInt2> operator+=(UInt2 &lhs, RValue<UInt2> rhs);
	RValue<UInt2> operator-=(UInt2 &lhs, RValue<UInt2> rhs);
//	RValue<UInt2> operator*=(UInt2 &lhs, RValue<UInt2> rhs);
//	RValue<UInt2> operator/=(UInt2 &lhs, RValue<UInt2> rhs);
//	RValue<UInt2> operator%=(UInt2 &lhs, RValue<UInt2> rhs);
	RValue<UInt2> operator&=(UInt2 &lhs, RValue<UInt2> rhs);
	RValue<UInt2> operator|=(UInt2 &lhs, RValue<UInt2> rhs);
	RValue<UInt2> operator^=(UInt2 &lhs, RValue<UInt2> rhs);
	RValue<UInt2> operator<<=(UInt2 &lhs, unsigned char rhs);
	RValue<UInt2> operator>>=(UInt2 &lhs, unsigned char rhs);
//	RValue<UInt2> operator+(RValue<UInt2> val);
//	RValue<UInt2> operator-(RValue<UInt2> val);
	RValue<UInt2> operator~(RValue<UInt2> val);
//	RValue<UInt2> operator++(UInt2 &val, int);   // Post-increment
//	const UInt2 &operator++(UInt2 &val);   // Pre-increment
//	RValue<UInt2> operator--(UInt2 &val, int);   // Post-decrement
//	const UInt2 &operator--(UInt2 &val);   // Pre-decrement
//	RValue<Bool> operator<(RValue<UInt2> lhs, RValue<UInt2> rhs);
//	RValue<Bool> operator<=(RValue<UInt2> lhs, RValue<UInt2> rhs);
//	RValue<Bool> operator>(RValue<UInt2> lhs, RValue<UInt2> rhs);
//	RValue<Bool> operator>=(RValue<UInt2> lhs, RValue<UInt2> rhs);
//	RValue<Bool> operator!=(RValue<UInt2> lhs, RValue<UInt2> rhs);
//	RValue<Bool> operator==(RValue<UInt2> lhs, RValue<UInt2> rhs);

//	RValue<UInt2> RoundInt(RValue<Float4> cast);

	class Int4 : public LValue<Int4>
	{
	public:
		explicit Int4(RValue<Byte4> cast);
		explicit Int4(RValue<SByte4> cast);
		explicit Int4(RValue<Float4> cast);
		explicit Int4(RValue<Short4> cast);
		explicit Int4(RValue<UShort4> cast);

		Int4() = default;
		Int4(int xyzw);
		Int4(int x, int yzw);
		Int4(int x, int y, int zw);
		Int4(int x, int y, int z, int w);
		Int4(RValue<Int4> rhs);
		Int4(const Int4 &rhs);
		Int4(const Reference<Int4> &rhs);
		Int4(RValue<UInt4> rhs);
		Int4(const UInt4 &rhs);
		Int4(const Reference<UInt4> &rhs);
		Int4(RValue<Int2> lo, RValue<Int2> hi);
		Int4(RValue<Int> rhs);
		Int4(const Int &rhs);
		Int4(const Reference<Int> &rhs);

		RValue<Int4> operator=(RValue<Int4> rhs);
		RValue<Int4> operator=(const Int4 &rhs);
		RValue<Int4> operator=(const Reference<Int4> &rhs);

		static Type *getType();

	private:
		void constant(int x, int y, int z, int w);
	};

	RValue<Int4> operator+(RValue<Int4> lhs, RValue<Int4> rhs);
	RValue<Int4> operator-(RValue<Int4> lhs, RValue<Int4> rhs);
	RValue<Int4> operator*(RValue<Int4> lhs, RValue<Int4> rhs);
	RValue<Int4> operator/(RValue<Int4> lhs, RValue<Int4> rhs);
	RValue<Int4> operator%(RValue<Int4> lhs, RValue<Int4> rhs);
	RValue<Int4> operator&(RValue<Int4> lhs, RValue<Int4> rhs);
	RValue<Int4> operator|(RValue<Int4> lhs, RValue<Int4> rhs);
	RValue<Int4> operator^(RValue<Int4> lhs, RValue<Int4> rhs);
	RValue<Int4> operator<<(RValue<Int4> lhs, unsigned char rhs);
	RValue<Int4> operator>>(RValue<Int4> lhs, unsigned char rhs);
	RValue<Int4> operator<<(RValue<Int4> lhs, RValue<Int4> rhs);
	RValue<Int4> operator>>(RValue<Int4> lhs, RValue<Int4> rhs);
	RValue<Int4> operator+=(Int4 &lhs, RValue<Int4> rhs);
	RValue<Int4> operator-=(Int4 &lhs, RValue<Int4> rhs);
	RValue<Int4> operator*=(Int4 &lhs, RValue<Int4> rhs);
//	RValue<Int4> operator/=(Int4 &lhs, RValue<Int4> rhs);
//	RValue<Int4> operator%=(Int4 &lhs, RValue<Int4> rhs);
	RValue<Int4> operator&=(Int4 &lhs, RValue<Int4> rhs);
	RValue<Int4> operator|=(Int4 &lhs, RValue<Int4> rhs);
	RValue<Int4> operator^=(Int4 &lhs, RValue<Int4> rhs);
	RValue<Int4> operator<<=(Int4 &lhs, unsigned char rhs);
	RValue<Int4> operator>>=(Int4 &lhs, unsigned char rhs);
	RValue<Int4> operator+(RValue<Int4> val);
	RValue<Int4> operator-(RValue<Int4> val);
	RValue<Int4> operator~(RValue<Int4> val);
//	RValue<Int4> operator++(Int4 &val, int);   // Post-increment
//	const Int4 &operator++(Int4 &val);   // Pre-increment
//	RValue<Int4> operator--(Int4 &val, int);   // Post-decrement
//	const Int4 &operator--(Int4 &val);   // Pre-decrement
//	RValue<Bool> operator<(RValue<Int4> lhs, RValue<Int4> rhs);
//	RValue<Bool> operator<=(RValue<Int4> lhs, RValue<Int4> rhs);
//	RValue<Bool> operator>(RValue<Int4> lhs, RValue<Int4> rhs);
//	RValue<Bool> operator>=(RValue<Int4> lhs, RValue<Int4> rhs);
//	RValue<Bool> operator!=(RValue<Int4> lhs, RValue<Int4> rhs);
//	RValue<Bool> operator==(RValue<Int4> lhs, RValue<Int4> rhs);

	RValue<Int4> CmpEQ(RValue<Int4> x, RValue<Int4> y);
	RValue<Int4> CmpLT(RValue<Int4> x, RValue<Int4> y);
	RValue<Int4> CmpLE(RValue<Int4> x, RValue<Int4> y);
	RValue<Int4> CmpNEQ(RValue<Int4> x, RValue<Int4> y);
	RValue<Int4> CmpNLT(RValue<Int4> x, RValue<Int4> y);
	RValue<Int4> CmpNLE(RValue<Int4> x, RValue<Int4> y);
	RValue<Int4> Max(RValue<Int4> x, RValue<Int4> y);
	RValue<Int4> Min(RValue<Int4> x, RValue<Int4> y);
	RValue<Int4> RoundInt(RValue<Float4> cast);
	RValue<Short8> Pack(RValue<Int4> x, RValue<Int4> y);
	RValue<Int> Extract(RValue<Int4> val, int i);
	RValue<Int4> Insert(RValue<Int4> val, RValue<Int> element, int i);
	RValue<Int> SignMask(RValue<Int4> x);
	RValue<Int4> Swizzle(RValue<Int4> x, unsigned char select);

	class UInt4 : public LValue<UInt4>
	{
	public:
		explicit UInt4(RValue<Float4> cast);

		UInt4() = default;
		UInt4(int xyzw);
		UInt4(int x, int yzw);
		UInt4(int x, int y, int zw);
		UInt4(int x, int y, int z, int w);
		UInt4(unsigned int x, unsigned int y, unsigned int z, unsigned int w);
		UInt4(RValue<UInt4> rhs);
		UInt4(const UInt4 &rhs);
		UInt4(const Reference<UInt4> &rhs);
		UInt4(RValue<Int4> rhs);
		UInt4(const Int4 &rhs);
		UInt4(const Reference<Int4> &rhs);
		UInt4(RValue<UInt2> lo, RValue<UInt2> hi);

		RValue<UInt4> operator=(RValue<UInt4> rhs);
		RValue<UInt4> operator=(const UInt4 &rhs);
		RValue<UInt4> operator=(const Reference<UInt4> &rhs);

		static Type *getType();

	private:
		void constant(int x, int y, int z, int w);
	};

	RValue<UInt4> operator+(RValue<UInt4> lhs, RValue<UInt4> rhs);
	RValue<UInt4> operator-(RValue<UInt4> lhs, RValue<UInt4> rhs);
	RValue<UInt4> operator*(RValue<UInt4> lhs, RValue<UInt4> rhs);
	RValue<UInt4> operator/(RValue<UInt4> lhs, RValue<UInt4> rhs);
	RValue<UInt4> operator%(RValue<UInt4> lhs, RValue<UInt4> rhs);
	RValue<UInt4> operator&(RValue<UInt4> lhs, RValue<UInt4> rhs);
	RValue<UInt4> operator|(RValue<UInt4> lhs, RValue<UInt4> rhs);
	RValue<UInt4> operator^(RValue<UInt4> lhs, RValue<UInt4> rhs);
	RValue<UInt4> operator<<(RValue<UInt4> lhs, unsigned char rhs);
	RValue<UInt4> operator>>(RValue<UInt4> lhs, unsigned char rhs);
	RValue<UInt4> operator<<(RValue<UInt4> lhs, RValue<UInt4> rhs);
	RValue<UInt4> operator>>(RValue<UInt4> lhs, RValue<UInt4> rhs);
	RValue<UInt4> operator+=(UInt4 &lhs, RValue<UInt4> rhs);
	RValue<UInt4> operator-=(UInt4 &lhs, RValue<UInt4> rhs);
	RValue<UInt4> operator*=(UInt4 &lhs, RValue<UInt4> rhs);
//	RValue<UInt4> operator/=(UInt4 &lhs, RValue<UInt4> rhs);
//	RValue<UInt4> operator%=(UInt4 &lhs, RValue<UInt4> rhs);
	RValue<UInt4> operator&=(UInt4 &lhs, RValue<UInt4> rhs);
	RValue<UInt4> operator|=(UInt4 &lhs, RValue<UInt4> rhs);
	RValue<UInt4> operator^=(UInt4 &lhs, RValue<UInt4> rhs);
	RValue<UInt4> operator<<=(UInt4 &lhs, unsigned char rhs);
	RValue<UInt4> operator>>=(UInt4 &lhs, unsigned char rhs);
	RValue<UInt4> operator+(RValue<UInt4> val);
	RValue<UInt4> operator-(RValue<UInt4> val);
	RValue<UInt4> operator~(RValue<UInt4> val);
//	RValue<UInt4> operator++(UInt4 &val, int);   // Post-increment
//	const UInt4 &operator++(UInt4 &val);   // Pre-increment
//	RValue<UInt4> operator--(UInt4 &val, int);   // Post-decrement
//	const UInt4 &operator--(UInt4 &val);   // Pre-decrement
//	RValue<Bool> operator<(RValue<UInt4> lhs, RValue<UInt4> rhs);
//	RValue<Bool> operator<=(RValue<UInt4> lhs, RValue<UInt4> rhs);
//	RValue<Bool> operator>(RValue<UInt4> lhs, RValue<UInt4> rhs);
//	RValue<Bool> operator>=(RValue<UInt4> lhs, RValue<UInt4> rhs);
//	RValue<Bool> operator!=(RValue<UInt4> lhs, RValue<UInt4> rhs);
//	RValue<Bool> operator==(RValue<UInt4> lhs, RValue<UInt4> rhs);

	RValue<UInt4> CmpEQ(RValue<UInt4> x, RValue<UInt4> y);
	RValue<UInt4> CmpLT(RValue<UInt4> x, RValue<UInt4> y);
	RValue<UInt4> CmpLE(RValue<UInt4> x, RValue<UInt4> y);
	RValue<UInt4> CmpNEQ(RValue<UInt4> x, RValue<UInt4> y);
	RValue<UInt4> CmpNLT(RValue<UInt4> x, RValue<UInt4> y);
	RValue<UInt4> CmpNLE(RValue<UInt4> x, RValue<UInt4> y);
	RValue<UInt4> Max(RValue<UInt4> x, RValue<UInt4> y);
	RValue<UInt4> Min(RValue<UInt4> x, RValue<UInt4> y);
//	RValue<UInt4> RoundInt(RValue<Float4> cast);
	RValue<UShort8> Pack(RValue<UInt4> x, RValue<UInt4> y);

	template<class T>
	struct Scalar;

	template<class Vector4>
	struct FloatXYZW;

	template<int T, class Vector4>
	class Swizzle2Float4
	{
		friend Vector4;

	public:
		operator RValue<Vector4>() const;

	private:
		Vector4 *parent;
	};

	template<int T, class Vector4>
	class SwizzleFloat4
	{
	public:
		operator RValue<Vector4>() const;

	private:
		Vector4 *parent;
	};

	template<int T, class Vector4>
	class SwizzleMaskFloat4
	{
		friend FloatXYZW<Vector4>;

	public:
		operator RValue<Vector4>() const;

		RValue<Vector4> operator=(RValue<Vector4> rhs);
		RValue<Vector4> operator=(RValue<typename Scalar<Vector4>::Type> rhs);

	private:
		Vector4 *parent;
	};

	template<>
	struct Scalar<Float4>
	{
		using Type = Float;
	};

	template<int T, class Vector4>
	class SwizzleMask1Float4
	{
	public:
		operator RValue<typename Scalar<Vector4>::Type>() const;
		operator RValue<Vector4>() const;

		RValue<Vector4> operator=(float x);
		RValue<Vector4> operator=(RValue<Vector4> rhs);
		RValue<Vector4> operator=(RValue<typename Scalar<Vector4>::Type> rhs);

	private:
		Float4 *parent;
	};

	template<int T, class Vector4>
	class SwizzleMask2Float4
	{
		friend class Float4;

	public:
		operator RValue<Vector4>() const;

		RValue<Vector4> operator=(RValue<Vector4> rhs);

	private:
		Float4 *parent;
	};

	class Float : public LValue<Float>
	{
	public:
		explicit Float(RValue<Int> cast);

		Float() = default;
		Float(float x);
		Float(RValue<Float> rhs);
		Float(const Float &rhs);
		Float(const Reference<Float> &rhs);

		template<int T>
		Float(const SwizzleMask1Float4<T, Float4> &rhs);

	//	RValue<Float> operator=(float rhs);   // FIXME: Implement
		RValue<Float> operator=(RValue<Float> rhs);
		RValue<Float> operator=(const Float &rhs);
		RValue<Float> operator=(const Reference<Float> &rhs);

		template<int T>
		RValue<Float> operator=(const SwizzleMask1Float4<T, Float4> &rhs);

		static Type *getType();
	};

	RValue<Float> operator+(RValue<Float> lhs, RValue<Float> rhs);
	RValue<Float> operator-(RValue<Float> lhs, RValue<Float> rhs);
	RValue<Float> operator*(RValue<Float> lhs, RValue<Float> rhs);
	RValue<Float> operator/(RValue<Float> lhs, RValue<Float> rhs);
	RValue<Float> operator+=(Float &lhs, RValue<Float> rhs);
	RValue<Float> operator-=(Float &lhs, RValue<Float> rhs);
	RValue<Float> operator*=(Float &lhs, RValue<Float> rhs);
	RValue<Float> operator/=(Float &lhs, RValue<Float> rhs);
	RValue<Float> operator+(RValue<Float> val);
	RValue<Float> operator-(RValue<Float> val);
	RValue<Bool> operator<(RValue<Float> lhs, RValue<Float> rhs);
	RValue<Bool> operator<=(RValue<Float> lhs, RValue<Float> rhs);
	RValue<Bool> operator>(RValue<Float> lhs, RValue<Float> rhs);
	RValue<Bool> operator>=(RValue<Float> lhs, RValue<Float> rhs);
	RValue<Bool> operator!=(RValue<Float> lhs, RValue<Float> rhs);
	RValue<Bool> operator==(RValue<Float> lhs, RValue<Float> rhs);

	RValue<Float> Abs(RValue<Float> x);
	RValue<Float> Max(RValue<Float> x, RValue<Float> y);
	RValue<Float> Min(RValue<Float> x, RValue<Float> y);
	RValue<Float> Rcp_pp(RValue<Float> val, bool exactAtPow2 = false);
	RValue<Float> RcpSqrt_pp(RValue<Float> val);
	RValue<Float> Sqrt(RValue<Float> x);
	RValue<Float> Round(RValue<Float> val);
	RValue<Float> Trunc(RValue<Float> val);
	RValue<Float> Frac(RValue<Float> val);
	RValue<Float> Floor(RValue<Float> val);
	RValue<Float> Ceil(RValue<Float> val);

	class Float2 : public LValue<Float2>
	{
	public:
	//	explicit Float2(RValue<Byte2> cast);
	//	explicit Float2(RValue<Short2> cast);
	//	explicit Float2(RValue<UShort2> cast);
	//	explicit Float2(RValue<Int2> cast);
	//	explicit Float2(RValue<UInt2> cast);
		explicit Float2(RValue<Float4> cast);

		Float2() = default;
	//	Float2(float x, float y);
	//	Float2(RValue<Float2> rhs);
	//	Float2(const Float2 &rhs);
	//	Float2(const Reference<Float2> &rhs);
	//	Float2(RValue<Float> rhs);
	//	Float2(const Float &rhs);
	//	Float2(const Reference<Float> &rhs);

	//	template<int T>
	//	Float2(const SwizzleMask1Float4<T> &rhs);

	//	RValue<Float2> operator=(float replicate);
	//	RValue<Float2> operator=(RValue<Float2> rhs);
	//	RValue<Float2> operator=(const Float2 &rhs);
	//	RValue<Float2> operator=(const Reference<Float2> &rhs);
	//	RValue<Float2> operator=(RValue<Float> rhs);
	//	RValue<Float2> operator=(const Float &rhs);
	//	RValue<Float2> operator=(const Reference<Float> &rhs);

	//	template<int T>
	//	RValue<Float2> operator=(const SwizzleMask1Float4<T> &rhs);

		static Type *getType();
	};

//	RValue<Float2> operator+(RValue<Float2> lhs, RValue<Float2> rhs);
//	RValue<Float2> operator-(RValue<Float2> lhs, RValue<Float2> rhs);
//	RValue<Float2> operator*(RValue<Float2> lhs, RValue<Float2> rhs);
//	RValue<Float2> operator/(RValue<Float2> lhs, RValue<Float2> rhs);
//	RValue<Float2> operator%(RValue<Float2> lhs, RValue<Float2> rhs);
//	RValue<Float2> operator+=(Float2 &lhs, RValue<Float2> rhs);
//	RValue<Float2> operator-=(Float2 &lhs, RValue<Float2> rhs);
//	RValue<Float2> operator*=(Float2 &lhs, RValue<Float2> rhs);
//	RValue<Float2> operator/=(Float2 &lhs, RValue<Float2> rhs);
//	RValue<Float2> operator%=(Float2 &lhs, RValue<Float2> rhs);
//	RValue<Float2> operator+(RValue<Float2> val);
//	RValue<Float2> operator-(RValue<Float2> val);

//	RValue<Float2> Abs(RValue<Float2> x);
//	RValue<Float2> Max(RValue<Float2> x, RValue<Float2> y);
//	RValue<Float2> Min(RValue<Float2> x, RValue<Float2> y);
//	RValue<Float2> Swizzle(RValue<Float2> x, unsigned char select);
//	RValue<Float2> Mask(Float2 &lhs, RValue<Float2> rhs, unsigned char select);

	template<class Vector4>
	struct FloatXYZW
	{
		friend Vector4;

	private:
		FloatXYZW(Vector4 *parent)
		{
			xyzw.parent = parent;
		}

	public:
		union
		{
			SwizzleMask1Float4<0x00, Vector4> x;
			SwizzleMask1Float4<0x55, Vector4> y;
			SwizzleMask1Float4<0xAA, Vector4> z;
			SwizzleMask1Float4<0xFF, Vector4> w;
			Swizzle2Float4<0x00, Vector4>     xx;
			Swizzle2Float4<0x01, Vector4>     yx;
			Swizzle2Float4<0x02, Vector4>     zx;
			Swizzle2Float4<0x03, Vector4>     wx;
			SwizzleMask2Float4<0x54, Vector4> xy;
			Swizzle2Float4<0x55, Vector4>     yy;
			Swizzle2Float4<0x56, Vector4>     zy;
			Swizzle2Float4<0x57, Vector4>     wy;
			SwizzleMask2Float4<0xA8, Vector4> xz;
			SwizzleMask2Float4<0xA9, Vector4> yz;
			Swizzle2Float4<0xAA, Vector4>     zz;
			Swizzle2Float4<0xAB, Vector4>     wz;
			SwizzleMask2Float4<0xFC, Vector4> xw;
			SwizzleMask2Float4<0xFD, Vector4> yw;
			SwizzleMask2Float4<0xFE, Vector4> zw;
			Swizzle2Float4<0xFF, Vector4>     ww;
			SwizzleFloat4<0x00, Vector4>      xxx;
			SwizzleFloat4<0x01, Vector4>      yxx;
			SwizzleFloat4<0x02, Vector4>      zxx;
			SwizzleFloat4<0x03, Vector4>      wxx;
			SwizzleFloat4<0x04, Vector4>      xyx;
			SwizzleFloat4<0x05, Vector4>      yyx;
			SwizzleFloat4<0x06, Vector4>      zyx;
			SwizzleFloat4<0x07, Vector4>      wyx;
			SwizzleFloat4<0x08, Vector4>      xzx;
			SwizzleFloat4<0x09, Vector4>      yzx;
			SwizzleFloat4<0x0A, Vector4>      zzx;
			SwizzleFloat4<0x0B, Vector4>      wzx;
			SwizzleFloat4<0x0C, Vector4>      xwx;
			SwizzleFloat4<0x0D, Vector4>      ywx;
			SwizzleFloat4<0x0E, Vector4>      zwx;
			SwizzleFloat4<0x0F, Vector4>      wwx;
			SwizzleFloat4<0x50, Vector4>      xxy;
			SwizzleFloat4<0x51, Vector4>      yxy;
			SwizzleFloat4<0x52, Vector4>      zxy;
			SwizzleFloat4<0x53, Vector4>      wxy;
			SwizzleFloat4<0x54, Vector4>      xyy;
			SwizzleFloat4<0x55, Vector4>      yyy;
			SwizzleFloat4<0x56, Vector4>      zyy;
			SwizzleFloat4<0x57, Vector4>      wyy;
			SwizzleFloat4<0x58, Vector4>      xzy;
			SwizzleFloat4<0x59, Vector4>      yzy;
			SwizzleFloat4<0x5A, Vector4>      zzy;
			SwizzleFloat4<0x5B, Vector4>      wzy;
			SwizzleFloat4<0x5C, Vector4>      xwy;
			SwizzleFloat4<0x5D, Vector4>      ywy;
			SwizzleFloat4<0x5E, Vector4>      zwy;
			SwizzleFloat4<0x5F, Vector4>      wwy;
			SwizzleFloat4<0xA0, Vector4>      xxz;
			SwizzleFloat4<0xA1, Vector4>      yxz;
			SwizzleFloat4<0xA2, Vector4>      zxz;
			SwizzleFloat4<0xA3, Vector4>      wxz;
			SwizzleMaskFloat4<0xA4, Vector4>  xyz;
			SwizzleFloat4<0xA5, Vector4>      yyz;
			SwizzleFloat4<0xA6, Vector4>      zyz;
			SwizzleFloat4<0xA7, Vector4>      wyz;
			SwizzleFloat4<0xA8, Vector4>      xzz;
			SwizzleFloat4<0xA9, Vector4>      yzz;
			SwizzleFloat4<0xAA, Vector4>      zzz;
			SwizzleFloat4<0xAB, Vector4>      wzz;
			SwizzleFloat4<0xAC, Vector4>      xwz;
			SwizzleFloat4<0xAD, Vector4>      ywz;
			SwizzleFloat4<0xAE, Vector4>      zwz;
			SwizzleFloat4<0xAF, Vector4>      wwz;
			SwizzleFloat4<0xF0, Vector4>      xxw;
			SwizzleFloat4<0xF1, Vector4>      yxw;
			SwizzleFloat4<0xF2, Vector4>      zxw;
			SwizzleFloat4<0xF3, Vector4>      wxw;
			SwizzleMaskFloat4<0xF4, Vector4>  xyw;
			SwizzleFloat4<0xF5, Vector4>      yyw;
			SwizzleFloat4<0xF6, Vector4>      zyw;
			SwizzleFloat4<0xF7, Vector4>      wyw;
			SwizzleMaskFloat4<0xF8, Vector4>  xzw;
			SwizzleMaskFloat4<0xF9, Vector4>  yzw;
			SwizzleFloat4<0xFA, Vector4>      zzw;
			SwizzleFloat4<0xFB, Vector4>      wzw;
			SwizzleFloat4<0xFC, Vector4>      xww;
			SwizzleFloat4<0xFD, Vector4>      yww;
			SwizzleFloat4<0xFE, Vector4>      zww;
			SwizzleFloat4<0xFF, Vector4>      www;
			SwizzleFloat4<0x00, Vector4>      xxxx;
			SwizzleFloat4<0x01, Vector4>      yxxx;
			SwizzleFloat4<0x02, Vector4>      zxxx;
			SwizzleFloat4<0x03, Vector4>      wxxx;
			SwizzleFloat4<0x04, Vector4>      xyxx;
			SwizzleFloat4<0x05, Vector4>      yyxx;
			SwizzleFloat4<0x06, Vector4>      zyxx;
			SwizzleFloat4<0x07, Vector4>      wyxx;
			SwizzleFloat4<0x08, Vector4>      xzxx;
			SwizzleFloat4<0x09, Vector4>      yzxx;
			SwizzleFloat4<0x0A, Vector4>      zzxx;
			SwizzleFloat4<0x0B, Vector4>      wzxx;
			SwizzleFloat4<0x0C, Vector4>      xwxx;
			SwizzleFloat4<0x0D, Vector4>      ywxx;
			SwizzleFloat4<0x0E, Vector4>      zwxx;
			SwizzleFloat4<0x0F, Vector4>      wwxx;
			SwizzleFloat4<0x10, Vector4>      xxyx;
			SwizzleFloat4<0x11, Vector4>      yxyx;
			SwizzleFloat4<0x12, Vector4>      zxyx;
			SwizzleFloat4<0x13, Vector4>      wxyx;
			SwizzleFloat4<0x14, Vector4>      xyyx;
			SwizzleFloat4<0x15, Vector4>      yyyx;
			SwizzleFloat4<0x16, Vector4>      zyyx;
			SwizzleFloat4<0x17, Vector4>      wyyx;
			SwizzleFloat4<0x18, Vector4>      xzyx;
			SwizzleFloat4<0x19, Vector4>      yzyx;
			SwizzleFloat4<0x1A, Vector4>      zzyx;
			SwizzleFloat4<0x1B, Vector4>      wzyx;
			SwizzleFloat4<0x1C, Vector4>      xwyx;
			SwizzleFloat4<0x1D, Vector4>      ywyx;
			SwizzleFloat4<0x1E, Vector4>      zwyx;
			SwizzleFloat4<0x1F, Vector4>      wwyx;
			SwizzleFloat4<0x20, Vector4>      xxzx;
			SwizzleFloat4<0x21, Vector4>      yxzx;
			SwizzleFloat4<0x22, Vector4>      zxzx;
			SwizzleFloat4<0x23, Vector4>      wxzx;
			SwizzleFloat4<0x24, Vector4>      xyzx;
			SwizzleFloat4<0x25, Vector4>      yyzx;
			SwizzleFloat4<0x26, Vector4>      zyzx;
			SwizzleFloat4<0x27, Vector4>      wyzx;
			SwizzleFloat4<0x28, Vector4>      xzzx;
			SwizzleFloat4<0x29, Vector4>      yzzx;
			SwizzleFloat4<0x2A, Vector4>      zzzx;
			SwizzleFloat4<0x2B, Vector4>      wzzx;
			SwizzleFloat4<0x2C, Vector4>      xwzx;
			SwizzleFloat4<0x2D, Vector4>      ywzx;
			SwizzleFloat4<0x2E, Vector4>      zwzx;
			SwizzleFloat4<0x2F, Vector4>      wwzx;
			SwizzleFloat4<0x30, Vector4>      xxwx;
			SwizzleFloat4<0x31, Vector4>      yxwx;
			SwizzleFloat4<0x32, Vector4>      zxwx;
			SwizzleFloat4<0x33, Vector4>      wxwx;
			SwizzleFloat4<0x34, Vector4>      xywx;
			SwizzleFloat4<0x35, Vector4>      yywx;
			SwizzleFloat4<0x36, Vector4>      zywx;
			SwizzleFloat4<0x37, Vector4>      wywx;
			SwizzleFloat4<0x38, Vector4>      xzwx;
			SwizzleFloat4<0x39, Vector4>      yzwx;
			SwizzleFloat4<0x3A, Vector4>      zzwx;
			SwizzleFloat4<0x3B, Vector4>      wzwx;
			SwizzleFloat4<0x3C, Vector4>      xwwx;
			SwizzleFloat4<0x3D, Vector4>      ywwx;
			SwizzleFloat4<0x3E, Vector4>      zwwx;
			SwizzleFloat4<0x3F, Vector4>      wwwx;
			SwizzleFloat4<0x40, Vector4>      xxxy;
			SwizzleFloat4<0x41, Vector4>      yxxy;
			SwizzleFloat4<0x42, Vector4>      zxxy;
			SwizzleFloat4<0x43, Vector4>      wxxy;
			SwizzleFloat4<0x44, Vector4>      xyxy;
			SwizzleFloat4<0x45, Vector4>      yyxy;
			SwizzleFloat4<0x46, Vector4>      zyxy;
			SwizzleFloat4<0x47, Vector4>      wyxy;
			SwizzleFloat4<0x48, Vector4>      xzxy;
			SwizzleFloat4<0x49, Vector4>      yzxy;
			SwizzleFloat4<0x4A, Vector4>      zzxy;
			SwizzleFloat4<0x4B, Vector4>      wzxy;
			SwizzleFloat4<0x4C, Vector4>      xwxy;
			SwizzleFloat4<0x4D, Vector4>      ywxy;
			SwizzleFloat4<0x4E, Vector4>      zwxy;
			SwizzleFloat4<0x4F, Vector4>      wwxy;
			SwizzleFloat4<0x50, Vector4>      xxyy;
			SwizzleFloat4<0x51, Vector4>      yxyy;
			SwizzleFloat4<0x52, Vector4>      zxyy;
			SwizzleFloat4<0x53, Vector4>      wxyy;
			SwizzleFloat4<0x54, Vector4>      xyyy;
			SwizzleFloat4<0x55, Vector4>      yyyy;
			SwizzleFloat4<0x56, Vector4>      zyyy;
			SwizzleFloat4<0x57, Vector4>      wyyy;
			SwizzleFloat4<0x58, Vector4>      xzyy;
			SwizzleFloat4<0x59, Vector4>      yzyy;
			SwizzleFloat4<0x5A, Vector4>      zzyy;
			SwizzleFloat4<0x5B, Vector4>      wzyy;
			SwizzleFloat4<0x5C, Vector4>      xwyy;
			SwizzleFloat4<0x5D, Vector4>      ywyy;
			SwizzleFloat4<0x5E, Vector4>      zwyy;
			SwizzleFloat4<0x5F, Vector4>      wwyy;
			SwizzleFloat4<0x60, Vector4>      xxzy;
			SwizzleFloat4<0x61, Vector4>      yxzy;
			SwizzleFloat4<0x62, Vector4>      zxzy;
			SwizzleFloat4<0x63, Vector4>      wxzy;
			SwizzleFloat4<0x64, Vector4>      xyzy;
			SwizzleFloat4<0x65, Vector4>      yyzy;
			SwizzleFloat4<0x66, Vector4>      zyzy;
			SwizzleFloat4<0x67, Vector4>      wyzy;
			SwizzleFloat4<0x68, Vector4>      xzzy;
			SwizzleFloat4<0x69, Vector4>      yzzy;
			SwizzleFloat4<0x6A, Vector4>      zzzy;
			SwizzleFloat4<0x6B, Vector4>      wzzy;
			SwizzleFloat4<0x6C, Vector4>      xwzy;
			SwizzleFloat4<0x6D, Vector4>      ywzy;
			SwizzleFloat4<0x6E, Vector4>      zwzy;
			SwizzleFloat4<0x6F, Vector4>      wwzy;
			SwizzleFloat4<0x70, Vector4>      xxwy;
			SwizzleFloat4<0x71, Vector4>      yxwy;
			SwizzleFloat4<0x72, Vector4>      zxwy;
			SwizzleFloat4<0x73, Vector4>      wxwy;
			SwizzleFloat4<0x74, Vector4>      xywy;
			SwizzleFloat4<0x75, Vector4>      yywy;
			SwizzleFloat4<0x76, Vector4>      zywy;
			SwizzleFloat4<0x77, Vector4>      wywy;
			SwizzleFloat4<0x78, Vector4>      xzwy;
			SwizzleFloat4<0x79, Vector4>      yzwy;
			SwizzleFloat4<0x7A, Vector4>      zzwy;
			SwizzleFloat4<0x7B, Vector4>      wzwy;
			SwizzleFloat4<0x7C, Vector4>      xwwy;
			SwizzleFloat4<0x7D, Vector4>      ywwy;
			SwizzleFloat4<0x7E, Vector4>      zwwy;
			SwizzleFloat4<0x7F, Vector4>      wwwy;
			SwizzleFloat4<0x80, Vector4>      xxxz;
			SwizzleFloat4<0x81, Vector4>      yxxz;
			SwizzleFloat4<0x82, Vector4>      zxxz;
			SwizzleFloat4<0x83, Vector4>      wxxz;
			SwizzleFloat4<0x84, Vector4>      xyxz;
			SwizzleFloat4<0x85, Vector4>      yyxz;
			SwizzleFloat4<0x86, Vector4>      zyxz;
			SwizzleFloat4<0x87, Vector4>      wyxz;
			SwizzleFloat4<0x88, Vector4>      xzxz;
			SwizzleFloat4<0x89, Vector4>      yzxz;
			SwizzleFloat4<0x8A, Vector4>      zzxz;
			SwizzleFloat4<0x8B, Vector4>      wzxz;
			SwizzleFloat4<0x8C, Vector4>      xwxz;
			SwizzleFloat4<0x8D, Vector4>      ywxz;
			SwizzleFloat4<0x8E, Vector4>      zwxz;
			SwizzleFloat4<0x8F, Vector4>      wwxz;
			SwizzleFloat4<0x90, Vector4>      xxyz;
			SwizzleFloat4<0x91, Vector4>      yxyz;
			SwizzleFloat4<0x92, Vector4>      zxyz;
			SwizzleFloat4<0x93, Vector4>      wxyz;
			SwizzleFloat4<0x94, Vector4>      xyyz;
			SwizzleFloat4<0x95, Vector4>      yyyz;
			SwizzleFloat4<0x96, Vector4>      zyyz;
			SwizzleFloat4<0x97, Vector4>      wyyz;
			SwizzleFloat4<0x98, Vector4>      xzyz;
			SwizzleFloat4<0x99, Vector4>      yzyz;
			SwizzleFloat4<0x9A, Vector4>      zzyz;
			SwizzleFloat4<0x9B, Vector4>      wzyz;
			SwizzleFloat4<0x9C, Vector4>      xwyz;
			SwizzleFloat4<0x9D, Vector4>      ywyz;
			SwizzleFloat4<0x9E, Vector4>      zwyz;
			SwizzleFloat4<0x9F, Vector4>      wwyz;
			SwizzleFloat4<0xA0, Vector4>      xxzz;
			SwizzleFloat4<0xA1, Vector4>      yxzz;
			SwizzleFloat4<0xA2, Vector4>      zxzz;
			SwizzleFloat4<0xA3, Vector4>      wxzz;
			SwizzleFloat4<0xA4, Vector4>      xyzz;
			SwizzleFloat4<0xA5, Vector4>      yyzz;
			SwizzleFloat4<0xA6, Vector4>      zyzz;
			SwizzleFloat4<0xA7, Vector4>      wyzz;
			SwizzleFloat4<0xA8, Vector4>      xzzz;
			SwizzleFloat4<0xA9, Vector4>      yzzz;
			SwizzleFloat4<0xAA, Vector4>      zzzz;
			SwizzleFloat4<0xAB, Vector4>      wzzz;
			SwizzleFloat4<0xAC, Vector4>      xwzz;
			SwizzleFloat4<0xAD, Vector4>      ywzz;
			SwizzleFloat4<0xAE, Vector4>      zwzz;
			SwizzleFloat4<0xAF, Vector4>      wwzz;
			SwizzleFloat4<0xB0, Vector4>      xxwz;
			SwizzleFloat4<0xB1, Vector4>      yxwz;
			SwizzleFloat4<0xB2, Vector4>      zxwz;
			SwizzleFloat4<0xB3, Vector4>      wxwz;
			SwizzleFloat4<0xB4, Vector4>      xywz;
			SwizzleFloat4<0xB5, Vector4>      yywz;
			SwizzleFloat4<0xB6, Vector4>      zywz;
			SwizzleFloat4<0xB7, Vector4>      wywz;
			SwizzleFloat4<0xB8, Vector4>      xzwz;
			SwizzleFloat4<0xB9, Vector4>      yzwz;
			SwizzleFloat4<0xBA, Vector4>      zzwz;
			SwizzleFloat4<0xBB, Vector4>      wzwz;
			SwizzleFloat4<0xBC, Vector4>      xwwz;
			SwizzleFloat4<0xBD, Vector4>      ywwz;
			SwizzleFloat4<0xBE, Vector4>      zwwz;
			SwizzleFloat4<0xBF, Vector4>      wwwz;
			SwizzleFloat4<0xC0, Vector4>      xxxw;
			SwizzleFloat4<0xC1, Vector4>      yxxw;
			SwizzleFloat4<0xC2, Vector4>      zxxw;
			SwizzleFloat4<0xC3, Vector4>      wxxw;
			SwizzleFloat4<0xC4, Vector4>      xyxw;
			SwizzleFloat4<0xC5, Vector4>      yyxw;
			SwizzleFloat4<0xC6, Vector4>      zyxw;
			SwizzleFloat4<0xC7, Vector4>      wyxw;
			SwizzleFloat4<0xC8, Vector4>      xzxw;
			SwizzleFloat4<0xC9, Vector4>      yzxw;
			SwizzleFloat4<0xCA, Vector4>      zzxw;
			SwizzleFloat4<0xCB, Vector4>      wzxw;
			SwizzleFloat4<0xCC, Vector4>      xwxw;
			SwizzleFloat4<0xCD, Vector4>      ywxw;
			SwizzleFloat4<0xCE, Vector4>      zwxw;
			SwizzleFloat4<0xCF, Vector4>      wwxw;
			SwizzleFloat4<0xD0, Vector4>      xxyw;
			SwizzleFloat4<0xD1, Vector4>      yxyw;
			SwizzleFloat4<0xD2, Vector4>      zxyw;
			SwizzleFloat4<0xD3, Vector4>      wxyw;
			SwizzleFloat4<0xD4, Vector4>      xyyw;
			SwizzleFloat4<0xD5, Vector4>      yyyw;
			SwizzleFloat4<0xD6, Vector4>      zyyw;
			SwizzleFloat4<0xD7, Vector4>      wyyw;
			SwizzleFloat4<0xD8, Vector4>      xzyw;
			SwizzleFloat4<0xD9, Vector4>      yzyw;
			SwizzleFloat4<0xDA, Vector4>      zzyw;
			SwizzleFloat4<0xDB, Vector4>      wzyw;
			SwizzleFloat4<0xDC, Vector4>      xwyw;
			SwizzleFloat4<0xDD, Vector4>      ywyw;
			SwizzleFloat4<0xDE, Vector4>      zwyw;
			SwizzleFloat4<0xDF, Vector4>      wwyw;
			SwizzleFloat4<0xE0, Vector4>      xxzw;
			SwizzleFloat4<0xE1, Vector4>      yxzw;
			SwizzleFloat4<0xE2, Vector4>      zxzw;
			SwizzleFloat4<0xE3, Vector4>      wxzw;
			SwizzleMaskFloat4<0xE4, Vector4>  xyzw;
			SwizzleFloat4<0xE5, Vector4>      yyzw;
			SwizzleFloat4<0xE6, Vector4>      zyzw;
			SwizzleFloat4<0xE7, Vector4>      wyzw;
			SwizzleFloat4<0xE8, Vector4>      xzzw;
			SwizzleFloat4<0xE9, Vector4>      yzzw;
			SwizzleFloat4<0xEA, Vector4>      zzzw;
			SwizzleFloat4<0xEB, Vector4>      wzzw;
			SwizzleFloat4<0xEC, Vector4>      xwzw;
			SwizzleFloat4<0xED, Vector4>      ywzw;
			SwizzleFloat4<0xEE, Vector4>      zwzw;
			SwizzleFloat4<0xEF, Vector4>      wwzw;
			SwizzleFloat4<0xF0, Vector4>      xxww;
			SwizzleFloat4<0xF1, Vector4>      yxww;
			SwizzleFloat4<0xF2, Vector4>      zxww;
			SwizzleFloat4<0xF3, Vector4>      wxww;
			SwizzleFloat4<0xF4, Vector4>      xyww;
			SwizzleFloat4<0xF5, Vector4>      yyww;
			SwizzleFloat4<0xF6, Vector4>      zyww;
			SwizzleFloat4<0xF7, Vector4>      wyww;
			SwizzleFloat4<0xF8, Vector4>      xzww;
			SwizzleFloat4<0xF9, Vector4>      yzww;
			SwizzleFloat4<0xFA, Vector4>      zzww;
			SwizzleFloat4<0xFB, Vector4>      wzww;
			SwizzleFloat4<0xFC, Vector4>      xwww;
			SwizzleFloat4<0xFD, Vector4>      ywww;
			SwizzleFloat4<0xFE, Vector4>      zwww;
			SwizzleFloat4<0xFF, Vector4>      wwww;
		};
	};

	class Float4 : public LValue<Float4>, public FloatXYZW<Float4>
	{
	public:
		explicit Float4(RValue<Byte4> cast);
		explicit Float4(RValue<SByte4> cast);
		explicit Float4(RValue<Short4> cast);
		explicit Float4(RValue<UShort4> cast);
		explicit Float4(RValue<Int4> cast);
		explicit Float4(RValue<UInt4> cast);

		Float4();
		Float4(float xyzw);
		Float4(float x, float yzw);
		Float4(float x, float y, float zw);
		Float4(float x, float y, float z, float w);
		Float4(RValue<Float4> rhs);
		Float4(const Float4 &rhs);
		Float4(const Reference<Float4> &rhs);
		Float4(RValue<Float> rhs);
		Float4(const Float &rhs);
		Float4(const Reference<Float> &rhs);

		template<int T>
		Float4(const SwizzleMask1Float4<T, Float4> &rhs);
		template<int T>
		Float4(const SwizzleFloat4<T, Float4> &rhs);
		template<int X, int Y>
		Float4(const Swizzle2Float4<X, Float4> &x, const Swizzle2Float4<Y, Float4> &y);
		template<int X, int Y>
		Float4(const SwizzleMask2Float4<X, Float4> &x, const Swizzle2Float4<Y, Float4> &y);
		template<int X, int Y>
		Float4(const Swizzle2Float4<X, Float4> &x, const SwizzleMask2Float4<Y, Float4> &y);
		template<int X, int Y>
		Float4(const SwizzleMask2Float4<X, Float4> &x, const SwizzleMask2Float4<Y, Float4> &y);

		RValue<Float4> operator=(float replicate);
		RValue<Float4> operator=(RValue<Float4> rhs);
		RValue<Float4> operator=(const Float4 &rhs);
		RValue<Float4> operator=(const Reference<Float4> &rhs);
		RValue<Float4> operator=(RValue<Float> rhs);
		RValue<Float4> operator=(const Float &rhs);
		RValue<Float4> operator=(const Reference<Float> &rhs);

		template<int T>
		RValue<Float4> operator=(const SwizzleMask1Float4<T, Float4> &rhs);
		template<int T>
		RValue<Float4> operator=(const SwizzleFloat4<T, Float4> &rhs);

		static Type *getType();

	private:
		void constant(float x, float y, float z, float w);
	};

	RValue<Float4> operator+(RValue<Float4> lhs, RValue<Float4> rhs);
	RValue<Float4> operator-(RValue<Float4> lhs, RValue<Float4> rhs);
	RValue<Float4> operator*(RValue<Float4> lhs, RValue<Float4> rhs);
	RValue<Float4> operator/(RValue<Float4> lhs, RValue<Float4> rhs);
	RValue<Float4> operator%(RValue<Float4> lhs, RValue<Float4> rhs);
	RValue<Float4> operator+=(Float4 &lhs, RValue<Float4> rhs);
	RValue<Float4> operator-=(Float4 &lhs, RValue<Float4> rhs);
	RValue<Float4> operator*=(Float4 &lhs, RValue<Float4> rhs);
	RValue<Float4> operator/=(Float4 &lhs, RValue<Float4> rhs);
	RValue<Float4> operator%=(Float4 &lhs, RValue<Float4> rhs);
	RValue<Float4> operator+(RValue<Float4> val);
	RValue<Float4> operator-(RValue<Float4> val);

	RValue<Float4> Abs(RValue<Float4> x);
	RValue<Float4> Max(RValue<Float4> x, RValue<Float4> y);
	RValue<Float4> Min(RValue<Float4> x, RValue<Float4> y);
	RValue<Float4> Rcp_pp(RValue<Float4> val, bool exactAtPow2 = false);
	RValue<Float4> RcpSqrt_pp(RValue<Float4> val);
	RValue<Float4> Sqrt(RValue<Float4> x);
	RValue<Float4> Insert(RValue<Float4> val, RValue<Float> element, int i);
	RValue<Float> Extract(RValue<Float4> x, int i);
	RValue<Float4> Swizzle(RValue<Float4> x, unsigned char select);
	RValue<Float4> ShuffleLowHigh(RValue<Float4> x, RValue<Float4> y, unsigned char imm);
	RValue<Float4> UnpackLow(RValue<Float4> x, RValue<Float4> y);
	RValue<Float4> UnpackHigh(RValue<Float4> x, RValue<Float4> y);
	RValue<Float4> Mask(Float4 &lhs, RValue<Float4> rhs, unsigned char select);
	RValue<Int> SignMask(RValue<Float4> x);
	RValue<Int4> CmpEQ(RValue<Float4> x, RValue<Float4> y);
	RValue<Int4> CmpLT(RValue<Float4> x, RValue<Float4> y);
	RValue<Int4> CmpLE(RValue<Float4> x, RValue<Float4> y);
	RValue<Int4> CmpNEQ(RValue<Float4> x, RValue<Float4> y);
	RValue<Int4> CmpNLT(RValue<Float4> x, RValue<Float4> y);
	RValue<Int4> CmpNLE(RValue<Float4> x, RValue<Float4> y);
	RValue<Float4> Round(RValue<Float4> x);
	RValue<Float4> Trunc(RValue<Float4> x);
	RValue<Float4> Frac(RValue<Float4> x);
	RValue<Float4> Floor(RValue<Float4> x);
	RValue<Float4> Ceil(RValue<Float4> x);

	template<class T>
	class Pointer : public LValue<Pointer<T>>
	{
	public:
		template<class S>
		Pointer(RValue<Pointer<S>> pointerS, int alignment = 1) : alignment(alignment)
		{
			Value *pointerT = Nucleus::createBitCast(pointerS.value, Nucleus::getPointerType(T::getType()));
			LValue<Pointer<T>>::storeValue(pointerT);
		}

		template<class S>
		Pointer(const Pointer<S> &pointer, int alignment = 1) : alignment(alignment)
		{
			Value *pointerS = pointer.loadValue(alignment);
			Value *pointerT = Nucleus::createBitCast(pointerS, Nucleus::getPointerType(T::getType()));
			LValue<Pointer<T>>::storeValue(pointerT);
		}

		Pointer(Argument<Pointer<T>> argument);

		Pointer();
		Pointer(RValue<Pointer<T>> rhs);
		Pointer(const Pointer<T> &rhs);
		Pointer(const Reference<Pointer<T>> &rhs);

		RValue<Pointer<T>> operator=(RValue<Pointer<T>> rhs);
		RValue<Pointer<T>> operator=(const Pointer<T> &rhs);
		RValue<Pointer<T>> operator=(const Reference<Pointer<T>> &rhs);

		Reference<T> operator*();
		Reference<T> operator[](int index);
		Reference<T> operator[](RValue<Int> index);

		static Type *getType();

	private:
		const int alignment;
	};

	RValue<Pointer<Byte>> operator+(RValue<Pointer<Byte>> lhs, int offset);
	RValue<Pointer<Byte>> operator+(RValue<Pointer<Byte>> lhs, RValue<Int> offset);
	RValue<Pointer<Byte>> operator+(RValue<Pointer<Byte>> lhs, RValue<UInt> offset);
	RValue<Pointer<Byte>> operator+=(Pointer<Byte> &lhs, int offset);
	RValue<Pointer<Byte>> operator+=(Pointer<Byte> &lhs, RValue<Int> offset);
	RValue<Pointer<Byte>> operator+=(Pointer<Byte> &lhs, RValue<UInt> offset);

	RValue<Pointer<Byte>> operator-(RValue<Pointer<Byte>> lhs, int offset);
	RValue<Pointer<Byte>> operator-(RValue<Pointer<Byte>> lhs, RValue<Int> offset);
	RValue<Pointer<Byte>> operator-(RValue<Pointer<Byte>> lhs, RValue<UInt> offset);
	RValue<Pointer<Byte>> operator-=(Pointer<Byte> &lhs, int offset);
	RValue<Pointer<Byte>> operator-=(Pointer<Byte> &lhs, RValue<Int> offset);
	RValue<Pointer<Byte>> operator-=(Pointer<Byte> &lhs, RValue<UInt> offset);

	template<class T, int S = 1>
	class Array : public LValue<T>
	{
	public:
		Array(int size = S);

		Reference<T> operator[](int index);
		Reference<T> operator[](RValue<Int> index);
	};

//	RValue<Array<T>> operator++(Array<T> &val, int);   // Post-increment
//	const Array<T> &operator++(Array<T> &val);   // Pre-increment
//	RValue<Array<T>> operator--(Array<T> &val, int);   // Post-decrement
//	const Array<T> &operator--(Array<T> &val);   // Pre-decrement

	bool branch(RValue<Bool> cmp, BasicBlock *bodyBB, BasicBlock *endBB);

	void Return();
	void Return(RValue<Int> ret);

	template<class T>
	void Return(const Pointer<T> &ret);

	template<class T>
	void Return(RValue<Pointer<T>> ret);

	template<unsigned int index, typename... Arguments>
	struct ArgI;

	template<typename Arg0, typename... Arguments>
	struct ArgI<0, Arg0, Arguments...>
	{
		typedef Arg0 Type;
	};

	template<unsigned int index, typename Arg0, typename... Arguments>
	struct ArgI<index, Arg0, Arguments...>
	{
		typedef typename ArgI<index - 1, Arguments...>::Type Type;
	};

	// Generic template, leave undefined!
	template<typename FunctionType>
	class Function;

	// Specialized for function types
	template<typename Return, typename... Arguments>
	class Function<Return(Arguments...)>
	{
	public:
		Function();

		virtual ~Function();

		template<int index>
		Argument<typename ArgI<index, Arguments...>::Type> Arg() const
		{
			Value *arg = Nucleus::getArgument(index);
			return Argument<typename ArgI<index, Arguments...>::Type>(arg);
		}

		Routine *operator()(const wchar_t *name, ...);

	protected:
		Nucleus *core;
		std::vector<Type*> arguments;
	};

	template<typename Return>
	class Function<Return()> : public Function<Return(Void)>
	{
	};

	template<int index, typename Return, typename... Arguments>
	Argument<typename ArgI<index, Arguments...>::Type> Arg(Function<Return(Arguments...)> &function)
	{
		return Argument<typename ArgI<index, Arguments...>::Type>(function.arg(index));
	}

	RValue<Long> Ticks();
}

namespace sw
{
	template<class T>
	LValue<T>::LValue(int arraySize)
	{
		address = Nucleus::allocateStackVariable(T::getType(), arraySize);
	}

	template<class T>
	Value *LValue<T>::loadValue(unsigned int alignment) const
	{
		return Nucleus::createLoad(address, T::getType(), false, alignment);
	}

	template<class T>
	Value *LValue<T>::storeValue(Value *value, unsigned int alignment) const
	{
		return Nucleus::createStore(value, address, T::getType(), false, alignment);
	}

	template<class T>
	Value *LValue<T>::getAddress(Value *index) const
	{
		return Nucleus::createGEP(address, T::getType(), index);
	}

	template<class T>
	RValue<Pointer<T>> LValue<T>::operator&()
	{
		return RValue<Pointer<T>>(address);
	}

	template<class T>
	Reference<T>::Reference(Value *pointer, int alignment) : alignment(alignment)
	{
		address = pointer;
	}

	template<class T>
	RValue<T> Reference<T>::operator=(RValue<T> rhs) const
	{
		Nucleus::createStore(rhs.value, address, T::getType(), false, alignment);

		return rhs;
	}

	template<class T>
	RValue<T> Reference<T>::operator=(const Reference<T> &ref) const
	{
		Value *tmp = Nucleus::createLoad(ref.address, T::getType(), false, ref.alignment);
		Nucleus::createStore(tmp, address, T::getType(), false, alignment);

		return RValue<T>(tmp);
	}

	template<class T>
	RValue<T> Reference<T>::operator+=(RValue<T> rhs) const
	{
		return *this = *this + rhs;
	}

	template<class T>
	Value *Reference<T>::loadValue() const
	{
		return Nucleus::createLoad(address, T::getType(), false, alignment);
	}

	template<class T>
	int Reference<T>::getAlignment() const
	{
		return alignment;
	}

	template<class T>
	RValue<T>::RValue(Value *rvalue)
	{
		value = rvalue;
	}

	template<class T>
	RValue<T>::RValue(const T &lvalue)
	{
		value = lvalue.loadValue();
	}

	template<class T>
	RValue<T>::RValue(typename IntLiteral<T>::type i)
	{
		value = Nucleus::createConstantInt(i);
	}

	template<class T>
	RValue<T>::RValue(typename FloatLiteral<T>::type f)
	{
		value = Nucleus::createConstantFloat(f);
	}

	template<class T>
	RValue<T>::RValue(const Reference<T> &ref)
	{
		value = ref.loadValue();
	}

	template<int T, class Vector4>
	Swizzle2Float4<T, Vector4>::operator RValue<Vector4>() const
	{
		Value *vector = parent->loadValue();

		return Swizzle(RValue<Vector4>(vector), T);
	}

	template<int T, class Vector4>
	SwizzleFloat4<T, Vector4>::operator RValue<Vector4>() const
	{
		Value *vector = parent->loadValue();

		return Swizzle(RValue<Vector4>(vector), T);
	}

	template<int T, class Vector4>
	SwizzleMaskFloat4<T, Vector4>::operator RValue<Vector4>() const
	{
		Value *vector = parent->loadValue();

		return Swizzle(RValue<Vector4>(vector), T);
	}

	template<int T, class Vector4>
	RValue<Vector4> SwizzleMaskFloat4<T, Vector4>::operator=(RValue<Vector4> rhs)
	{
		return Mask(*parent, rhs, T);
	}

	template<int T, class Vector4>
	RValue<Vector4> SwizzleMaskFloat4<T, Vector4>::operator=(RValue<typename Scalar<Vector4>::Type> rhs)
	{
		return Mask(*parent, Vector4(rhs), T);
	}

	template<int T, class Vector4>
	SwizzleMask1Float4<T, Vector4>::operator RValue<typename Scalar<Vector4>::Type>() const   // FIXME: Call a non-template function
	{
		return Extract(*parent, T & 0x3);
	}

	template<int T, class Vector4>
	SwizzleMask1Float4<T, Vector4>::operator RValue<Vector4>() const
	{
		Value *vector = parent->loadValue();

		return Swizzle(RValue<Vector4>(vector), T);
	}

	template<int T, class Vector4>
	RValue<Vector4> SwizzleMask1Float4<T, Vector4>::operator=(float x)
	{
		return *parent = Insert(*parent, Float(x), T & 0x3);
	}

	template<int T, class Vector4>
	RValue<Vector4> SwizzleMask1Float4<T, Vector4>::operator=(RValue<Vector4> rhs)
	{
		return Mask(*parent, Float4(rhs), T);
	}

	template<int T, class Vector4>
	RValue<Vector4> SwizzleMask1Float4<T, Vector4>::operator=(RValue<typename Scalar<Vector4>::Type> rhs)   // FIXME: Call a non-template function
	{
		return *parent = Insert(*parent, rhs, T & 0x3);
	}

	template<int T, class Vector4>
	SwizzleMask2Float4<T, Vector4>::operator RValue<Vector4>() const
	{
		Value *vector = parent->loadValue();

		return Swizzle(RValue<Float4>(vector), T);
	}

	template<int T, class Vector4>
	RValue<Vector4> SwizzleMask2Float4<T, Vector4>::operator=(RValue<Vector4> rhs)
	{
		return Mask(*parent, Float4(rhs), T);
	}

	template<int T>
	Float::Float(const SwizzleMask1Float4<T, Float4> &rhs)
	{
		*this = rhs.operator RValue<Float>();
	}

	template<int T>
	RValue<Float> Float::operator=(const SwizzleMask1Float4<T, Float4> &rhs)
	{
		return *this = rhs.operator RValue<Float>();
	}

	template<int T>
	Float4::Float4(const SwizzleMask1Float4<T, Float4> &rhs) : FloatXYZW(this)
	{
		*this = rhs.operator RValue<Float4>();
	}

	template<int T>
	Float4::Float4(const SwizzleFloat4<T, Float4> &rhs) : FloatXYZW(this)
	{
		*this = rhs.operator RValue<Float4>();
	}

	template<int X, int Y>
	Float4::Float4(const Swizzle2Float4<X, Float4> &x, const Swizzle2Float4<Y, Float4> &y) : FloatXYZW(this)
	{
		*this = ShuffleLowHigh(*x.parent, *y.parent, (X & 0xF) | (Y & 0xF) << 4);
	}

	template<int X, int Y>
	Float4::Float4(const SwizzleMask2Float4<X, Float4> &x, const Swizzle2Float4<Y, Float4> &y) : FloatXYZW(this)
	{
		*this = ShuffleLowHigh(*x.parent, *y.parent, (X & 0xF) | (Y & 0xF) << 4);
	}

	template<int X, int Y>
	Float4::Float4(const Swizzle2Float4<X, Float4> &x, const SwizzleMask2Float4<Y, Float4> &y) : FloatXYZW(this)
	{
		*this = ShuffleLowHigh(*x.parent, *y.parent, (X & 0xF) | (Y & 0xF) << 4);
	}

	template<int X, int Y>
	Float4::Float4(const SwizzleMask2Float4<X, Float4> &x, const SwizzleMask2Float4<Y, Float4> &y) : FloatXYZW(this)
	{
		*this = ShuffleLowHigh(*x.parent, *y.parent, (X & 0xF) | (Y & 0xF) << 4);
	}

	template<int T>
	RValue<Float4> Float4::operator=(const SwizzleMask1Float4<T, Float4> &rhs)
	{
		return *this = rhs.operator RValue<Float4>();
	}

	template<int T>
	RValue<Float4> Float4::operator=(const SwizzleFloat4<T, Float4> &rhs)
	{
		return *this = rhs.operator RValue<Float4>();
	}

	template<class T>
	Pointer<T>::Pointer(Argument<Pointer<T>> argument) : alignment(1)
	{
		LValue<Pointer<T>>::storeValue(argument.value);
	}

	template<class T>
	Pointer<T>::Pointer() : alignment(1)
	{
		LValue<Pointer<T>>::storeValue(Nucleus::createNullPointer(T::getType()));
	}

	template<class T>
	Pointer<T>::Pointer(RValue<Pointer<T>> rhs) : alignment(1)
	{
		LValue<Pointer<T>>::storeValue(rhs.value);
	}

	template<class T>
	Pointer<T>::Pointer(const Pointer<T> &rhs) : alignment(rhs.alignment)
	{
		Value *value = rhs.loadValue();
		LValue<Pointer<T>>::storeValue(value);
	}

	template<class T>
	Pointer<T>::Pointer(const Reference<Pointer<T>> &rhs) : alignment(rhs.getAlignment())
	{
		Value *value = rhs.loadValue();
		LValue<Pointer<T>>::storeValue(value);
	}

	template<class T>
	RValue<Pointer<T>> Pointer<T>::operator=(RValue<Pointer<T>> rhs)
	{
		LValue<Pointer<T>>::storeValue(rhs.value);

		return rhs;
	}

	template<class T>
	RValue<Pointer<T>> Pointer<T>::operator=(const Pointer<T> &rhs)
	{
		Value *value = rhs.loadValue();
		LValue<Pointer<T>>::storeValue(value);

		return RValue<Pointer<T>>(value);
	}

	template<class T>
	RValue<Pointer<T>> Pointer<T>::operator=(const Reference<Pointer<T>> &rhs)
	{
		Value *value = rhs.loadValue();
		LValue<Pointer<T>>::storeValue(value);

		return RValue<Pointer<T>>(value);
	}

	template<class T>
	Reference<T> Pointer<T>::operator*()
	{
		return Reference<T>(LValue<Pointer<T>>::loadValue(), alignment);
	}

	template<class T>
	Reference<T> Pointer<T>::operator[](int index)
	{
		Value *element = Nucleus::createGEP(LValue<Pointer<T>>::loadValue(), T::getType(), Nucleus::createConstantInt(index));

		return Reference<T>(element, alignment);
	}

	template<class T>
	Reference<T> Pointer<T>::operator[](RValue<Int> index)
	{
		Value *element = Nucleus::createGEP(LValue<Pointer<T>>::loadValue(), T::getType(), index.value);

		return Reference<T>(element, alignment);
	}

	template<class T>
	Type *Pointer<T>::getType()
	{
		return Nucleus::getPointerType(T::getType());
	}

	template<class T, int S>
	Array<T, S>::Array(int size) : LValue<T>(size)
	{
	}

	template<class T, int S>
	Reference<T> Array<T, S>::operator[](int index)
	{
		Value *element = LValue<T>::getAddress(Nucleus::createConstantInt(index));

		return Reference<T>(element);
	}

	template<class T, int S>
	Reference<T> Array<T, S>::operator[](RValue<Int> index)
	{
		Value *element = LValue<T>::getAddress(index.value);

		return Reference<T>(element);
	}

//	template<class T>
//	RValue<Array<T>> operator++(Array<T> &val, int)
//	{
//		// FIXME: Requires storing the address of the array
//	}

//	template<class T>
//	const Array<T> &operator++(Array<T> &val)
//	{
//		// FIXME: Requires storing the address of the array
//	}

//	template<class T>
//	RValue<Array<T>> operator--(Array<T> &val, int)
//	{
//		// FIXME: Requires storing the address of the array
//	}

//	template<class T>
//	const Array<T> &operator--(Array<T> &val)
//	{
//		// FIXME: Requires storing the address of the array
//	}

	template<class T>
	RValue<T> IfThenElse(RValue<Bool> condition, RValue<T> ifTrue, RValue<T> ifFalse)
	{
		return RValue<T>(Nucleus::createSelect(condition.value, ifTrue.value, ifFalse.value));
	}

	template<class T>
	RValue<T> IfThenElse(RValue<Bool> condition, const T &ifTrue, RValue<T> ifFalse)
	{
		Value *trueValue = ifTrue.loadValue();

		return RValue<T>(Nucleus::createSelect(condition.value, trueValue, ifFalse.value));
	}

	template<class T>
	RValue<T> IfThenElse(RValue<Bool> condition, RValue<T> ifTrue, const T &ifFalse)
	{
		Value *falseValue = ifFalse.loadValue();

		return RValue<T>(Nucleus::createSelect(condition.value, ifTrue.value, falseValue));
	}

	template<class T>
	RValue<T> IfThenElse(RValue<Bool> condition, const T &ifTrue, const T &ifFalse)
	{
		Value *trueValue = ifTrue.loadValue();
		Value *falseValue = ifFalse.loadValue();

		return RValue<T>(Nucleus::createSelect(condition.value, trueValue, falseValue));
	}

	template<class T>
	void Return(const Pointer<T> &ret)
	{
		Nucleus::createRet(Nucleus::createLoad(ret.address, Pointer<T>::getType()));
		Nucleus::setInsertBlock(Nucleus::createBasicBlock());
	}

	template<class T>
	void Return(RValue<Pointer<T>> ret)
	{
		Nucleus::createRet(ret.value);
		Nucleus::setInsertBlock(Nucleus::createBasicBlock());
	}

	template<typename Return, typename... Arguments>
	Function<Return(Arguments...)>::Function()
	{
		core = new Nucleus();

		Type *types[] = {Arguments::getType()...};
		for(Type *type : types)
		{
			if(type != Void::getType())
			{
				arguments.push_back(type);
			}
		}

		Nucleus::createFunction(Return::getType(), arguments);
	}

	template<typename Return, typename... Arguments>
	Function<Return(Arguments...)>::~Function()
	{
		delete core;
	}

	template<typename Return, typename... Arguments>
	Routine *Function<Return(Arguments...)>::operator()(const wchar_t *name, ...)
	{
		wchar_t fullName[1024 + 1];

		va_list vararg;
		va_start(vararg, name);
		vswprintf(fullName, 1024, name, vararg);
		va_end(vararg);

		return core->acquireRoutine(fullName, true);
	}

	template<class T, class S>
	RValue<T> ReinterpretCast(RValue<S> val)
	{
		return RValue<T>(Nucleus::createBitCast(val.value, T::getType()));
	}

	template<class T, class S>
	RValue<T> ReinterpretCast(const LValue<S> &var)
	{
		Value *val = var.loadValue();

		return RValue<T>(Nucleus::createBitCast(val, T::getType()));
	}

	template<class T, class S>
	RValue<T> ReinterpretCast(const Reference<S> &var)
	{
		return ReinterpretCast<T>(RValue<S>(var));
	}

	template<class T>
	RValue<T> As(Value *val)
	{
		return RValue<T>(Nucleus::createBitCast(val, T::getType()));
	}

	template<class T, class S>
	RValue<T> As(RValue<S> val)
	{
		return ReinterpretCast<T>(val);
	}

	template<class T, class S>
	RValue<T> As(const LValue<S> &var)
	{
		return ReinterpretCast<T>(var);
	}

	template<class T, class S>
	RValue<T> As(const Reference<S> &val)
	{
		return ReinterpretCast<T>(val);
	}

	class ForData
	{
	public:
		ForData(bool init) : loopOnce(init)
		{
		}

		operator bool()
		{
			return loopOnce;
		}

		bool operator=(bool value)
		{
			return loopOnce = value;
		}

		bool setup()
		{
			if(Nucleus::getInsertBlock() != endBB)
			{
				testBB = Nucleus::createBasicBlock();

				Nucleus::createBr(testBB);
				Nucleus::setInsertBlock(testBB);

				return true;
			}

			return false;
		}

		bool test(RValue<Bool> cmp)
		{
			BasicBlock *bodyBB = Nucleus::createBasicBlock();
			endBB = Nucleus::createBasicBlock();

			Nucleus::createCondBr(cmp.value, bodyBB, endBB);
			Nucleus::setInsertBlock(bodyBB);

			return true;
		}

		void end()
		{
			Nucleus::createBr(testBB);
			Nucleus::setInsertBlock(endBB);
		}

	private:
		BasicBlock *testBB = nullptr;
		BasicBlock *endBB = nullptr;
		bool loopOnce = true;
	};

	class IfElseData
	{
	public:
		IfElseData(RValue<Bool> cmp) : iteration(0)
		{
			condition = cmp.value;

			beginBB = Nucleus::getInsertBlock();
			trueBB = Nucleus::createBasicBlock();
			falseBB = nullptr;
			endBB = Nucleus::createBasicBlock();

			Nucleus::setInsertBlock(trueBB);
		}

		~IfElseData()
		{
			Nucleus::createBr(endBB);

			Nucleus::setInsertBlock(beginBB);
			Nucleus::createCondBr(condition, trueBB, falseBB ? falseBB : endBB);

			Nucleus::setInsertBlock(endBB);
		}

		operator int()
		{
			return iteration;
		}

		IfElseData &operator++()
		{
			++iteration;

			return *this;
		}

		void elseClause()
		{
			Nucleus::createBr(endBB);

			falseBB = Nucleus::createBasicBlock();
			Nucleus::setInsertBlock(falseBB);
		}

	private:
		Value *condition;
		BasicBlock *beginBB;
		BasicBlock *trueBB;
		BasicBlock *falseBB;
		BasicBlock *endBB;
		int iteration;
	};

	#define For(init, cond, inc) \
	for(ForData for__ = true; for__; for__ = false) \
	for(init; for__.setup() && for__.test(cond); inc, for__.end())

	#define While(cond) For((void)0, cond, (void)0)

	#define Do                                            \
	{                                                     \
		BasicBlock *body__ = Nucleus::createBasicBlock(); \
		Nucleus::createBr(body__);                        \
		Nucleus::setInsertBlock(body__);

	#define Until(cond)                                     \
		BasicBlock *end__ = Nucleus::createBasicBlock();    \
		Nucleus::createCondBr((cond).value, end__, body__); \
		Nucleus::setInsertBlock(end__);                     \
	}

	enum {IF_BLOCK__, ELSE_CLAUSE__, ELSE_BLOCK__, IFELSE_NUM__};

	#define If(cond)                                                    \
	for(IfElseData ifElse__(cond); ifElse__ < IFELSE_NUM__; ++ifElse__) \
	if(ifElse__ == IF_BLOCK__)

	#define Else                       \
	else if(ifElse__ == ELSE_CLAUSE__) \
	{                                  \
		 ifElse__.elseClause();        \
	}                                  \
	else   // ELSE_BLOCK__
}

#endif   // sw_Reactor_hpp
