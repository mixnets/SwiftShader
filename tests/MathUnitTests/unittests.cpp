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

#include "System/CPUID.hpp"
#include "System/Half.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstdlib>

using namespace sw;

// Returns the whole-number ULP error of `a` relative to `x`.
// Use the doouble-precision version below. This just illustrates the principle.
[[deprecated]] float ULP_32(float x, float a)
{
	// Flip the last mantissa bit to compute the 'unit in the last place' error.
	float x1 = bit_cast<float>(bit_cast<uint32_t>(x) ^ 0x00000001);
	float ulp = abs(x1 - x);

	return abs(a - x) / ulp;
}

double ULP_32(double x, double a)
{
	// binary64 has 52 mantissa bits, while binary32 has 23, so the ULP for the latter is 29 bits shifted.
	double x1 = bit_cast<double>(bit_cast<uint64_t>(x) ^ 0x0000000020000000ll);
	double ulp = abs(x1 - x);

	return abs(a - x) / ulp;
}

float ULP_16(float x, float a)
{
	// binary32 has 23 mantissa bits, while binary16 has 10, so the ULP for the latter is 13 bits shifted.
	double x1 = bit_cast<float>(bit_cast<uint32_t>(x) ^ 0x00002000);
	float ulp = abs(x1 - x);

	return abs(a - x) / ulp;
}

// 2^-4 precision
float Exp2(float x)
{
	int flat = bit_cast<int>(x);
	int i = (int)floor(x);
	float f = x - (float)i;
	float o = 1.0f + f;

	int io = bit_cast<int>(o);
	io += (i << 23);

	return bit_cast<float>(io);
}

float Exp2x(float x)
{
	int flat = bit_cast<int>(x);
	int i = (int)floor(x);
	float f = x - (float)i;
	float o = 1.0f + f;

	int io = bit_cast<int>(o);
	io += (i << 23);

	return bit_cast<float>(io);
}

inline float exp2_0(float x)
{
	int i = (int)((1 << 23) * x) + (127 << 23);

	return bit_cast<float>(i);
}

// ULP-16: 4.035
float exp2_3(float x)
{
	const float f = x - floor(x);

	// 1 + f − 2^f
	constexpr float A = 0.345f;
	x -= A * f - A * f * f;

	int i = (int)((1 << 23) * x + (127 << 23));

	return bit_cast<float>(i);
}

// ULP-32: 96
float exp2_5(float x)
{
	const float f = x - floor(x);

	// 1 + f − 2^f
	constexpr float A = 0.3070204f;
	constexpr float B = -0.241605f;
	constexpr float C = -0.0517451f;
	constexpr float D = -0.0136703f;

	x -= f * (A + f * (B + f * (C + f * D)));

	int i = (int)((1 << 23) * x + (127 << 23));

	return bit_cast<float>(i);
}

// ULP-32: 37.9367828
float exp2_6(float x)
{
	const float f = x - floor(x);

	// 1 + f − 2^f
	// constexpr float A = 0.30684479f;
	// constexpr float B = -0.240136f;
	// constexpr float C = -0.0558701f;
	// constexpr float D = -0.00894548f;
	// constexpr float E = -0.00189321f;

	constexpr float A = 3.06845249656632845792e-01f;
	constexpr float B = -2.40139721982230797126e-01f;
	constexpr float C = -5.58662282412822480682e-02f;
	constexpr float D = -8.94283890931273951763e-03f;
	constexpr float E = -1.89646052380707734290e-03f;

	x -= f * (A + f * (B + f * (C + f * (D + f * E))));

	int i = (int)((1 << 23) * x + (127 << 23));

	return bit_cast<float>(i);
}

// ULP-32: 37.8557243
float exp2_6fma(float x)
{
	const float f = x - floor(x);

	// 1 + f − 2^f
	// constexpr float A = 0.30684479f;
	// constexpr float B = -0.240136f;
	// constexpr float C = -0.0558701f;
	// constexpr float D = -0.00894548f;
	// constexpr float E = -0.00189321f;

	constexpr float A = -3.06845249656632845792e-01f;
	constexpr float B = 2.40139721982230797126e-01f;
	constexpr float C = 5.58662282412822480682e-02f;
	constexpr float D = 8.94283890931273951763e-03f;
	constexpr float E = 1.89646052380707734290e-03f;

	// x -= f * (A + f * (B + f * (C + f * (D + f * E))));
	x = fma(fma(fma(fma(fma(E, f, D), f, C), f, B), f, A), f, x);

	int i = (int)((1 << 23) * x + (127 << 23));

	return bit_cast<float>(i);
}

// ULP-32: 37.8557243
float exp2_6fma2(float x)
{
	const float f = x - floor(x);

	// 1 + f − 2^f
	// constexpr float A = 0.30684479f;
	// constexpr float B = -0.240136f;
	// constexpr float C = -0.0558701f;
	// constexpr float D = -0.00894548f;
	// constexpr float E = -0.00189321f;

	constexpr float A = -3.0684482709139982e-01f;
	constexpr float B = 2.40136023292754e-01f;
	constexpr float C = 5.58701110665769e-02f;
	constexpr float D = 8.94547921720901e-03f;
	constexpr float E = 1.89321351485994e-03f;

	// x -= f * (A + f * (B + f * (C + f * (D + f * E))));
	x = fma(fma(fma(fma(fma(E, f, D), f, C), f, B), f, A), f, x);

	int i = (int)((1 << 23) * x + (127 << 23));

	return bit_cast<float>(i);
}

// ULP-32: 2.29328680
double exp2_6d(double x)
{
	const double f = x - floor(x);

	// 1 + f − 2^f
	// constexpr float A = 0.30684479f;
	// constexpr float B = -0.240136f;
	// constexpr float C = -0.0558701f;
	// constexpr float D = -0.00894548f;
	// constexpr float E = -0.00189321f;

	constexpr double A = 3.06845249656632845792e-01;
	constexpr double B = -2.40139721982230797126e-01;
	constexpr double C = -5.58662282412822480682e-02;
	constexpr double D = -8.94283890931273951763e-03;
	constexpr double E = -1.89646052380707734290e-03;

	x -= f * (A + f * (B + f * (C + f * (D + f * E))));

	int64_t i = (int64_t)((1ll << 52) * x + (1023ll << 52));

	return bit_cast<double>(i);
}

// ULP-32: 3.36676240, worst x: -0.0677830279
float Exp2_old(float x)
{
	// This implementation is based on 2^(i + f) = 2^i * 2^f,
	// where i is the integer part of x and f is the fraction.

	// For 2^i we can put the integer part directly in the exponent of
	// the IEEE-754 floating-point number. Clamp to prevent overflow
	// past the representation of infinity.
	float x0 = x;
	// x0 = Min(x0, bit_cast<float>(int(0x43010000)));  // 129.00000e+0f
	// x0 = Max(x0, bit_cast<float>(int(0xC2FDFFFF)));  // -126.99999e+0f

	int i = (int)round(x0 - 0.5f);
	float ii = bit_cast<float>((i + int(127)) << 23);  // Add single-precision bias, and shift into exponent.

	// For the fractional part use a polynomial
	// which approximates 2^f in the 0 to 1 range.
	float f = x0 - float(i);
	float ff = bit_cast<float>(int(0x3AF61905));     // 1.8775767e-3f
	ff = ff * f + bit_cast<float>(int(0x3C134806));  // 8.9893397e-3f
	ff = ff * f + bit_cast<float>(int(0x3D64AA23));  // 5.5826318e-2f
	ff = ff * f + bit_cast<float>(int(0x3E75EAD4));  // 2.4015361e-1f
	ff = ff * f + bit_cast<float>(int(0x3F31727B));  // 6.9315308e-1f
	ff = ff * f + float(1.0f);

	return ii * ff;
}

// Returns the next floating-point number which is not treated equal to the input.
// Note that std::nextafter() does not skip representations flushed to zero.
static inline float inc(float x)
{
	int x1 = bit_cast<int>(x);

	while(bit_cast<float>(x1) == x)
	{
		// Since IEEE 754 uses ones' complement and integers are two's complement,
		// we need to explicitly hop from negative zero to positive zero.
		if(x1 == 0x80000000)  // -0.0f
		{
			// Note that while the comparison -0.0f == +0.0f returns true, this
			// function returns the next value which can be treated differently.
			return +0.0f;
		}

		// Negative ones' complement value are made less negative by subtracting 1
		// in two's complement representation.
		x1 += (x1 >= 0) ? 1 : -1;
	}

	return bit_cast<float>(x1);
}

TEST(MathTest, Exp2Exhaustive)
{
	sw::CPUID::setFlushToZero(true);

	float worst_ulp = 0;
	float worst_x = 0;
	float worst_val = 0;
	float worst_ref = 0;
	float worst_margin = 0;

	for(float x = -10; x <= 10; x = inc(x))
	{
		float val = (float)Exp2_old(x);

		double ref = exp2((double)x);
		float ulp = (float)ULP_32(ref, (double)val);

		float tolerance = (3 + 2 * abs(x));
		float margin = ulp / tolerance;

		if(margin > worst_margin)
		{
			worst_margin = margin;
			worst_ulp = ulp;
			worst_x = x;
			worst_val = val;
			worst_ref = ref;
		}
	}

	ASSERT_TRUE(worst_margin <= 1.0f);
}

TEST(MathTest, Exp2_Exhaustive)
{
	const float tolerance = powf(2.0f, -4.0f);

	for(float x = -2; x <= 2; x = nextafterf(x, +INFINITY))
	{
		float val = Exp2x(x);

		float ref = exp2(x);
		float diff = val - ref;
		if(abs(diff) > tolerance)
		{
			ASSERT_NEAR(val, ref, tolerance);
		}
	}
}

// Polynomal approximation of order 5 for sin(x * 2 * pi) in the range [-1/4, 1/4]
static float sin5(float x)
{
	// A * x^5 + B * x^3 + C * x
	// Exact at x = 0, 1/12, 1/6, 1/4, and their negatives, which correspond to x * 2 * pi = 0, pi/6, pi/3, pi/2
	const float A = (36288 - 20736 * sqrt(3)) / 5;
	const float B = 288 * sqrt(3) - 540;
	const float C = (47 - 9 * sqrt(3)) / 5;

	float x2 = x * x;

	return ((A * x2 + B) * x2 + C) * x;
}

TEST(MathTest, SinExhaustive)
{
	const float tolerance = powf(2.0f, -12.0f);  // Vulkan requires absolute error <= 2^−11 inside the range [−pi, pi]
	const float pi = 3.1415926535f;

	for(float x = -pi; x <= pi; x = nextafterf(x, +INFINITY))
	{
		// Range reduction and mirroring
		float x_2 = 0.25f - x * (0.5f / pi);
		float z = 0.25f - fabs(x_2 - round(x_2));

		float val = sin5(z);

		ASSERT_NEAR(val, sinf(x), tolerance);
	}
}

TEST(MathTest, CosExhaustive)
{
	const float tolerance = powf(2.0f, -12.0f);  // Vulkan requires absolute error <= 2^−11 inside the range [−pi, pi]
	const float pi = 3.1415926535f;

	for(float x = -pi; x <= pi; x = nextafterf(x, +INFINITY))
	{
		// Phase shift, range reduction, and mirroring
		float x_2 = x * (0.5f / pi);
		float z = 0.25f - fabs(x_2 - round(x_2));

		float val = sin5(z);

		ASSERT_NEAR(val, cosf(x), tolerance);
	}
}

TEST(MathTest, UnsignedFloat11_10)
{
	// Test the largest value which causes underflow to 0, and the smallest value
	// which produces a denormalized result.

	EXPECT_EQ(R11G11B10F::float32ToFloat11(bit_cast<float>(0x3500007F)), 0x0000);
	EXPECT_EQ(R11G11B10F::float32ToFloat11(bit_cast<float>(0x35000080)), 0x0001);

	EXPECT_EQ(R11G11B10F::float32ToFloat10(bit_cast<float>(0x3580003F)), 0x0000);
	EXPECT_EQ(R11G11B10F::float32ToFloat10(bit_cast<float>(0x35800040)), 0x0001);
}

// Clamps to the [0, hi] range. NaN input produces 0, hi must be non-NaN.
float clamp0hi(float x, float hi)
{
	// If x=NaN, x > 0 will compare false and we return 0.
	if(!(x > 0))
	{
		return 0;
	}

	// x is non-NaN at this point, so std::min() is safe for non-NaN hi.
	return std::min(x, hi);
}

unsigned int RGB9E5_reference(float r, float g, float b)
{
	// Vulkan 1.1.117 section 15.2.1 RGB to Shared Exponent Conversion

	// B is the exponent bias (15)
	constexpr int g_sharedexp_bias = 15;

	// N is the number of mantissa bits per component (9)
	constexpr int g_sharedexp_mantissabits = 9;

	// Emax is the maximum allowed biased exponent value (31)
	constexpr int g_sharedexp_maxexponent = 31;

	constexpr float g_sharedexp_max =
	    ((static_cast<float>(1 << g_sharedexp_mantissabits) - 1) /
	     static_cast<float>(1 << g_sharedexp_mantissabits)) *
	    static_cast<float>(1 << (g_sharedexp_maxexponent - g_sharedexp_bias));

	const float red_c = clamp0hi(r, g_sharedexp_max);
	const float green_c = clamp0hi(g, g_sharedexp_max);
	const float blue_c = clamp0hi(b, g_sharedexp_max);

	const float max_c = fmax(fmax(red_c, green_c), blue_c);
	const float exp_p = fmax(-g_sharedexp_bias - 1, floor(log2(max_c))) + 1 + g_sharedexp_bias;
	const int max_s = static_cast<int>(floor((max_c / exp2(exp_p - g_sharedexp_bias - g_sharedexp_mantissabits)) + 0.5f));
	const int exp_s = static_cast<int>((max_s < exp2(g_sharedexp_mantissabits)) ? exp_p : exp_p + 1);

	unsigned int R = static_cast<unsigned int>(floor((red_c / exp2(exp_s - g_sharedexp_bias - g_sharedexp_mantissabits)) + 0.5f));
	unsigned int G = static_cast<unsigned int>(floor((green_c / exp2(exp_s - g_sharedexp_bias - g_sharedexp_mantissabits)) + 0.5f));
	unsigned int B = static_cast<unsigned int>(floor((blue_c / exp2(exp_s - g_sharedexp_bias - g_sharedexp_mantissabits)) + 0.5f));
	unsigned int E = exp_s;

	return (E << 27) | (B << 18) | (G << 9) | R;
}

TEST(MathTest, SharedExponentSparse)
{
	for(uint64_t i = 0; i < 0x0000000100000000; i += 0x400)
	{
		float f = bit_cast<float>(i);

		unsigned int ref = RGB9E5_reference(f, 0.0f, 0.0f);
		unsigned int val = RGB9E5(f, 0.0f, 0.0f);

		EXPECT_EQ(ref, val);
	}
}

TEST(MathTest, SharedExponentRandom)
{
	srand(0);

	unsigned int x = 0;
	unsigned int y = 0;
	unsigned int z = 0;

	for(int i = 0; i < 10000000; i++)
	{
		float r = bit_cast<float>(x);
		float g = bit_cast<float>(y);
		float b = bit_cast<float>(z);

		unsigned int ref = RGB9E5_reference(r, g, b);
		unsigned int val = RGB9E5(r, g, b);

		EXPECT_EQ(ref, val);

		x += rand();
		y += rand();
		z += rand();
	}
}

TEST(MathTest, SharedExponentExhaustive)
{
	for(uint64_t i = 0; i < 0x0000000100000000; i += 1)
	{
		float f = bit_cast<float>(i);

		unsigned int ref = RGB9E5_reference(f, 0.0f, 0.0f);
		unsigned int val = RGB9E5(f, 0.0f, 0.0f);

		EXPECT_EQ(ref, val);
	}
}
