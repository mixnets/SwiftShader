// SwiftShader Software Renderer
//
// Copyright(c) 2005-2012 TransGaming Inc.
//
// All rights reserved. No part of this software may be copied, distributed, transmitted,
// transcribed, stored in a retrieval system, translated into any human or computer
// language by any means, or disclosed to third parties without the explicit written
// agreement of TransGaming Inc. Without such an agreement, no rights or licenses, express
// or implied, including but not limited to any patent rights, are granted to you.
//

#ifndef sw_ShaderCore_hpp
#define sw_ShaderCore_hpp

#include "Debug.hpp"
#include "Shader.hpp"
#include "Reactor/Reactor.hpp"

namespace sw
{
	template<typename T> struct Traits {};
	template<> struct Traits<Short4> { typedef unsigned short type; };
	template<> struct Traits<Int4> { typedef int type; };
	template<> struct Traits<UInt4> { typedef unsigned int type; };
	template<> struct Traits<Float4> { typedef float type; };

	template<typename T>
	class VectorBase
	{
	public:
		typedef typename Traits<T>::type baseType;

		VectorBase();
		VectorBase(baseType x, baseType y, baseType z, baseType w);
		VectorBase(const VectorBase<T> &rhs);

		inline T &operator[](int i);
		inline VectorBase &operator=(const VectorBase<T> &rhs);

		T x;
		T y;
		T z;
		T w;
	};

	typedef VectorBase<Short4> Vector4s;
	typedef VectorBase<Int4> Vector4i;
	typedef VectorBase<UInt4> Vector4u;
	typedef VectorBase<Float4> Vector4f;

	Float4 exponential2(RValue<Float4> x, bool pp = false);
	Float4 logarithm2(RValue<Float4> x, bool abs, bool pp = false);
	Float4 exponential(RValue<Float4> x, bool pp = false);
	Float4 logarithm(RValue<Float4> x, bool abs, bool pp = false);
	Float4 power(RValue<Float4> x, RValue<Float4> y, bool pp = false);
	Float4 reciprocal(RValue<Float4> x, bool pp = false, bool finite = false);
	Float4 reciprocalSquareRoot(RValue<Float4> x, bool abs, bool pp = false);
	Float4 modulo(RValue<Float4> x, RValue<Float4> y);
	Float4 sine_pi(RValue<Float4> x, bool pp = false);     // limited to [-pi, pi] range
	Float4 cosine_pi(RValue<Float4> x, bool pp = false);   // limited to [-pi, pi] range
	Float4 sine(RValue<Float4> x, bool pp = false);
	Float4 cosine(RValue<Float4> x, bool pp = false);
	Float4 tangent(RValue<Float4> x, bool pp = false);
	Float4 arccos(RValue<Float4> x, bool pp = false);
	Float4 arcsin(RValue<Float4> x, bool pp = false);
	Float4 arctan(RValue<Float4> x, bool pp = false);
	Float4 arctan(RValue<Float4> y, RValue<Float4> x, bool pp = false);
	Float4 sineh(RValue<Float4> x, bool pp = false);
	Float4 cosineh(RValue<Float4> x, bool pp = false);
	Float4 tangenth(RValue<Float4> x, bool pp = false);
	Float4 arccosh(RValue<Float4> x, bool pp = false);  // Limited to x >= 1
	Float4 arcsinh(RValue<Float4> x, bool pp = false);
	Float4 arctanh(RValue<Float4> x, bool pp = false);  // Limited to ]-1, 1[ range
	Int4 floatBitsToInt(RValue<Float4> x);
	UInt4 floatBitsToUInt(RValue<Float4> x);
	Float4 intBitsToFloat(RValue<Int4> x);
	Float4 uintBitsToFloat(RValue<UInt4> x);

	Float4 dot2(const Vector4f &v0, const Vector4f &v1);
	Float4 dot3(const Vector4f &v0, const Vector4f &v1);
	Float4 dot4(const Vector4f &v0, const Vector4f &v1);

	void transpose4x4(Short4 &row0, Short4 &row1, Short4 &row2, Short4 &row3);
	void transpose4x4(Float4 &row0, Float4 &row1, Float4 &row2, Float4 &row3);
	void transpose4x3(Float4 &row0, Float4 &row1, Float4 &row2, Float4 &row3);
	void transpose4x2(Float4 &row0, Float4 &row1, Float4 &row2, Float4 &row3);
	void transpose4x1(Float4 &row0, Float4 &row1, Float4 &row2, Float4 &row3);
	void transpose2x4(Float4 &row0, Float4 &row1, Float4 &row2, Float4 &row3);
	void transpose2x4h(Float4 &row0, Float4 &row1, Float4 &row2, Float4 &row3);
	void transpose4xN(Float4 &row0, Float4 &row1, Float4 &row2, Float4 &row3, int N);

	template<typename T>
	class RegisterBase
	{
	public:
		RegisterBase(const Reference<T> &x, const Reference<T> &y, const Reference<T> &z, const Reference<T> &w) : x(x), y(y), z(z), w(w)
		{
		}

		Reference<T> &operator[](int i)
		{
			switch(i)
			{
			default:
			case 0: return x;
			case 1: return y;
			case 2: return z;
			case 3: return w;
			}
		}

		RegisterBase<T> &operator=(const RegisterBase<T> &rhs)
		{
			x = rhs.x;
			y = rhs.y;
			z = rhs.z;
			w = rhs.w;

			return *this;
		}

		RegisterBase<T> &operator=(const VectorBase<T> &rhs)
		{
			x = rhs.x;
			y = rhs.y;
			z = rhs.z;
			w = rhs.w;

			return *this;
		}

		operator VectorBase<T>()
		{
			VectorBase<T> v;

			v.x = x;
			v.y = y;
			v.z = z;
			v.w = w;

			return v;
		}

		Reference<T> x;
		Reference<T> y;
		Reference<T> z;
		Reference<T> w;
	};

	typedef RegisterBase<Int4> RegisterI;
	typedef RegisterBase<UInt4> RegisterU;
	typedef RegisterBase<Float4> Register;

	template<int S, typename T>
	class RegisterArrayBase
	{
	public:
		RegisterArrayBase(bool dynamic = false) : dynamic(dynamic)
		{
			if(dynamic)
			{
				x = new Array<T>(S);
				y = new Array<T>(S);
				z = new Array<T>(S);
				w = new Array<T>(S);
			}
			else
			{
				x = new Array<T>[S];
				y = new Array<T>[S];
				z = new Array<T>[S];
				w = new Array<T>[S];
			}
		}

		~RegisterArrayBase()
		{
			delete[] x;
			delete[] y;
			delete[] z;
			delete[] w;
		}

		RegisterBase<T> operator[](int i)
		{
			if(dynamic)
			{
				return RegisterBase<T>(x[0][i], y[0][i], z[0][i], w[0][i]);
			}
			else
			{
				return RegisterBase<T>(x[i][0], y[i][0], z[i][0], w[i][0]);
			}
		}

		RegisterBase<T> operator[](RValue<Int> i)
		{
			ASSERT(dynamic);

			return RegisterBase<T>(x[0][i], y[0][i], z[0][i], w[0][i]);
		}

	private:
		const bool dynamic;
		Array<T> *x;
		Array<T> *y;
		Array<T> *z;
		Array<T> *w;
	};

	template <int S> using RegisterArrayI = RegisterArrayBase<S, Int4>;
	template <int S> using RegisterArrayU = RegisterArrayBase<S, UInt4>;
	template <int S> using RegisterArray = RegisterArrayBase<S, Float4>;

	class ShaderCore
	{
		typedef Shader::Control Control;

	public:
		void mov(Vector4f &dst, const Vector4f &src, bool floorToInteger = false);
		void f2b(Vector4f &dst, const Vector4f &src);
		void b2f(Vector4f &dst, const Vector4f &src);
		void add(Vector4f &dst, const Vector4f &src0, const Vector4f &src1);
		void sub(Vector4f &dst, const Vector4f &src0, const Vector4f &src1);
		void mad(Vector4f &dst, const Vector4f &src0, const Vector4f &src1, const Vector4f &src2);
		void mul(Vector4f &dst, const Vector4f &src0, const Vector4f &src1);
		void rcpx(Vector4f &dst, const Vector4f &src, bool pp = false);
		void div(Vector4f &dst, const Vector4f &src0, const Vector4f &src1);
		void mod(Vector4f &dst, const Vector4f &src0, const Vector4f &src1);
		void rsqx(Vector4f &dst, const Vector4f &src, bool pp = false);
		void sqrt(Vector4f &dst, const Vector4f &src, bool pp = false);
		void rsq(Vector4f &dst, const Vector4f &src, bool pp = false);
		void len2(Float4 &dst, const Vector4f &src, bool pp = false);
		void len3(Float4 &dst, const Vector4f &src, bool pp = false);
		void len4(Float4 &dst, const Vector4f &src, bool pp = false);
		void dist1(Float4 &dst, const Vector4f &src0, const Vector4f &src1, bool pp = false);
		void dist2(Float4 &dst, const Vector4f &src0, const Vector4f &src1, bool pp = false);
		void dist3(Float4 &dst, const Vector4f &src0, const Vector4f &src1, bool pp = false);
		void dist4(Float4 &dst, const Vector4f &src0, const Vector4f &src1, bool pp = false);
		void dp1(Vector4f &dst, const Vector4f &src0, const Vector4f &src1);
		void dp2(Vector4f &dst, const Vector4f &src0, const Vector4f &src1);
		void dp2add(Vector4f &dst, const Vector4f &src0, const Vector4f &src1, const Vector4f &src2);
		void dp3(Vector4f &dst, const Vector4f &src0, const Vector4f &src1);
		void dp4(Vector4f &dst, const Vector4f &src0, const Vector4f &src1);
		void min(Vector4f &dst, const Vector4f &src0, const Vector4f &src1);
		void max(Vector4f &dst, const Vector4f &src0, const Vector4f &src1);
		void slt(Vector4f &dst, const Vector4f &src0, const Vector4f &src1);
		void step(Vector4f &dst, const Vector4f &src0, const Vector4f &src1);
		void exp2x(Vector4f &dst, const Vector4f &src, bool pp = false);
		void exp2(Vector4f &dst, const Vector4f &src, bool pp = false);
		void exp(Vector4f &dst, const Vector4f &src, bool pp = false);
		void log2x(Vector4f &dst, const Vector4f &src, bool pp = false);
		void log2(Vector4f &dst, const Vector4f &src, bool pp = false);
		void log(Vector4f &dst, const Vector4f &src, bool pp = false);
		void lit(Vector4f &dst, const Vector4f &src);
		void att(Vector4f &dst, const Vector4f &src0, const Vector4f &src1);
		void lrp(Vector4f &dst, const Vector4f &src0, const Vector4f &src1, const Vector4f &src2);
		void smooth(Vector4f &dst, const Vector4f &src0, const Vector4f &src1, const Vector4f &src2);
		void floatBitsToInt(Vector4i &dst, const Vector4f &src);
		void floatBitsToUInt(Vector4u &dst, const Vector4f &src);
		void intBitsToFloat(Vector4f &dst, const Vector4i &src);
		void uintBitsToFloat(Vector4f &dst, const Vector4u &src);
		void frc(Vector4f &dst, const Vector4f &src);
		void trunc(Vector4f &dst, const Vector4f &src);
		void floor(Vector4f &dst, const Vector4f &src);
		void round(Vector4f &dst, const Vector4f &src);
		void roundEven(Vector4f &dst, const Vector4f &src);
		void ceil(Vector4f &dst, const Vector4f &src);
		void powx(Vector4f &dst, const Vector4f &src0, const Vector4f &src1, bool pp = false);
		void pow(Vector4f &dst, const Vector4f &src0, const Vector4f &src1, bool pp = false);
		void crs(Vector4f &dst, const Vector4f &src0, const Vector4f &src1);
		void forward1(Vector4f &dst, const Vector4f &src0, const Vector4f &src1, const Vector4f &src2);
		void forward2(Vector4f &dst, const Vector4f &src0, const Vector4f &src1, const Vector4f &src2);
		void forward3(Vector4f &dst, const Vector4f &src0, const Vector4f &src1, const Vector4f &src2);
		void forward4(Vector4f &dst, const Vector4f &src0, const Vector4f &src1, const Vector4f &src2);
		void reflect1(Vector4f &dst, const Vector4f &src0, const Vector4f &src1);
		void reflect2(Vector4f &dst, const Vector4f &src0, const Vector4f &src1);
		void reflect3(Vector4f &dst, const Vector4f &src0, const Vector4f &src1);
		void reflect4(Vector4f &dst, const Vector4f &src0, const Vector4f &src1);
		void refract1(Vector4f &dst, const Vector4f &src0, const Vector4f &src1, const Float4 &src2);
		void refract2(Vector4f &dst, const Vector4f &src0, const Vector4f &src1, const Float4 &src2);
		void refract3(Vector4f &dst, const Vector4f &src0, const Vector4f &src1, const Float4 &src2);
		void refract4(Vector4f &dst, const Vector4f &src0, const Vector4f &src1, const Float4 &src2);
		void sgn(Vector4f &dst, const Vector4f &src);
		void abs(Vector4f &dst, const Vector4f &src);
		void nrm2(Vector4f &dst, const Vector4f &src, bool pp = false);
		void nrm3(Vector4f &dst, const Vector4f &src, bool pp = false);
		void nrm4(Vector4f &dst, const Vector4f &src, bool pp = false);
		void sincos(Vector4f &dst, const Vector4f &src, bool pp = false);
		void cos(Vector4f &dst, const Vector4f &src, bool pp = false);
		void sin(Vector4f &dst, const Vector4f &src, bool pp = false);
		void tan(Vector4f &dst, const Vector4f &src, bool pp = false);
		void acos(Vector4f &dst, const Vector4f &src, bool pp = false);
		void asin(Vector4f &dst, const Vector4f &src, bool pp = false);
		void atan(Vector4f &dst, const Vector4f &src, bool pp = false);
		void atan2(Vector4f &dst, const Vector4f &src0, const Vector4f &src1, bool pp = false);
		void cosh(Vector4f &dst, const Vector4f &src, bool pp = false);
		void sinh(Vector4f &dst, const Vector4f &src, bool pp = false);
		void tanh(Vector4f &dst, const Vector4f &src, bool pp = false);
		void acosh(Vector4f &dst, const Vector4f &src, bool pp = false);
		void asinh(Vector4f &dst, const Vector4f &src, bool pp = false);
		void atanh(Vector4f &dst, const Vector4f &src, bool pp = false);
		void expp(Vector4f &dst, const Vector4f &src, unsigned short version);
		void logp(Vector4f &dst, const Vector4f &src, unsigned short version);
		void cmp0(Vector4f &dst, const Vector4f &src0, const Vector4f &src1, const Vector4f &src2);
		void cmp(Vector4f &dst, const Vector4f &src0, const Vector4f &src1, Control control);
		void icmp(Vector4f &dst, const Vector4f &src0, const Vector4f &src1, Control control);
		void select(Vector4f &dst, const Vector4f &src0, const Vector4f &src1, const Vector4f &src2);
		void extract(Float4 &dst, const Vector4f &src0, const Float4 &src1);
		void insert(Vector4f &dst, const Vector4f &src, const Float4 &element, const Float4 &index);
		void all(Float4 &dst, const Vector4f &src);
		void any(Float4 &dst, const Vector4f &src);
		void not(Vector4f &dst, const Vector4f &src);
		void or(Float4 &dst, const Float4 &src0, const Float4 &src1);
		void xor(Float4 &dst, const Float4 &src0, const Float4 &src1);
		void and(Float4 &dst, const Float4 &src0, const Float4 &src1);

	private:
		void sgn(Float4 &dst, const Float4 &src);
		void cmp0(Float4 &dst, const Float4 &src0, const Float4 &src1, const Float4 &src2);
		void select(Float4 &dst, RValue<Int4> src0, const Float4 &src1, const Float4 &src2);
	};
}

#endif   // sw_ShaderCore_hpp
