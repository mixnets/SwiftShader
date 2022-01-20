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

#include "System/Half.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstdlib>

using namespace sw;

//TEST(MathTest, SinExhaustive)
//{
//	// original i =0x3c0031ce
//	// D = 0.999f, i =0x3f30e5bc
//
//	for(uint32_t i = 0x00000000; i <= 0x3F700000; i += 1)
//	{
//		float f = bit_cast<float>(i);
//		float z = f;
//
//		const float A = bit_cast<float>(0x3ce7989c);  // 0.0282710114332f; 0x3ce7989c
//		const float B = bit_cast<float>(0xbe4c585c);  // -0.19955581819f;0xbe4c585c
//		const float C = bit_cast<float>(0x3c4aadfd);  // 0.012370583689f; 0x3c4aadfd
//		const float D = 0.9999f;                      //bit_cast<float>(0x3F700000);  // 1.0f; 0x3F700000
//		const float E = 0.0f;
//
//		float ref = sinf(f);
//		float val = (((A * z + B) * z + C) * z + D) * z + E;
//
//		float diff = val - ref;
//		float err = abs(diff);
//		const float tolerance = powf(2.0f, -11.0f);
//
//		if(err <= tolerance)
//		{
//		}
//		else
//		{
//			EXPECT_TRUE(err <= tolerance);
//		}
//	}
//}
//
//TEST(MathTest, SinExhaustive)
//{
//	// original i =0x3c0031ce
//	// D = 0.9998f, i =0x3e52fbad ++
//	// D = 0.9997f, i=0x3e610c3a ++
//	// D = 0.9995f, i = 0x3e8383dc ++
//	// D = 0.9991f, i = 0x3f210979 --
//	// D = 0.9992f, i = 0x3f272510 --
//	// D = 0.9993f, i = 0x3f2d7195 --
//	// D = 0.9994f, i = 0x3e9273a9 ++
//	// D = 0.99935f, i = 0x3e9e1283 ++
//
//	// D = 0.9993f, i = 0x3e19fd9d, d = 0.000488296151
//	// D = 0.9992f, i = 0x3e1b697a, d = 0.000488296151
//	// D = 0.9991f, i = 0x3e1cd47f, d = 0.000488296151
//	// D = 0.9990f, i = 0x3e1e3fa1, d = 0.000488296151
//	// D = 0.9989f, i = 0x3e1faa7e, d = 0.000488296151
//	// D = 0.9985f, i = 0x3e254fd6, d = 0.000488296151
//	// D = 0.9980f, i = 0x3e2c510e, d = 0.000488296151
//	// D = 0.9950f, i = 0x3e547cf4, d = 0.000488296151
//	// D = 0.9940f, i = 0x3e6110a0, d = 0.000488296151
//	// D = 0.9935f, i = 0x3e6732fb, d = 0.000488296151
//	// D = 0.9934f, i = 0x3e6869ca, d = 0.000488296151
//	// D = 0.9934f, C = 0.9934f, i = 0x3e6872f0, d = 0.000488296151
//	// D = 0.9934f, C = 0.9800f, i = 0x3e6885cb, d = 0.000488296151
//	// D = 0.9934f, C = 0.9750f, i = 0x3e688c83, d = 0.000488296151
//	// D = 0.9934f, C = 0.9740f, i = 0x3e688e04, d = 0.000488296151
//	// D = 0.9934f, C = 0.9740f, B = 1.0010f, i = 0x3e688eee, d = 0.000488296151
//
//	for(uint32_t i = 0x30000000; i <= 0x3F800000; i += 1)
//	{
//		float f = bit_cast<float>(i);
//		float y = f * bit_cast<float>(0x3e22f983);  // 1/2pi 0.15915494309  0x3e22f983
//
//		y = y - roundf(y);
//		float z = y;
//
//		const float A = bit_cast<float>(0x3e35e52b) * 0.9900f;  //6.28318530718f * bit_cast<float>(0x3ce7989c);  // 0.0282710114332f; 0x3ce7989c
//		const float B = bit_cast<float>(0xbfa07e08) * 1.0011f;  //6.28318530718f * bit_cast<float>(0xbe4c585c);  // -0.19955581819f;0xbe4c585c
//		const float C = bit_cast<float>(0x3d9f2f29) * 0.9740f;  //6.28318530718f * bit_cast<float>(0x3c4aadfd);  // 0.012370583689f; 0x3c4aadfd
//		const float D = 6.28318530718f * 0.9934f;               //bit_cast<float>(0x3F700000);  // 1.0f; 0x3F700000
//
//		float ref = sinf(f);
//		float val = (((A * z + B) * z + C) * z + D) * z;
//
//		float diff = val - ref;
//		float err = abs(diff);
//		const float tolerance = powf(2.0f, -11.0f);
//
//		//const float pi = 3.1415926535f;
//		//float x = f;
//		//float r = (-16 * pi + 128 * sqrt(2) - 128) / powf(pi, 4) * powf(x, 4) + (20 * pi - 128 * sqrt(2) + 112) / powf(pi, 3) * powf(x, 3) + (32 * sqrt(2) - 8 * pi - 20) / powf(pi, 2) * powf(x, 2) + x;
//
//		if(err <= tolerance)
//		{
//		}
//		else
//		{
//			EXPECT_TRUE(err <= tolerance);
//		}
//	}
//}
//
//TEST(MathTest, SinExhaustive)
//{
//	// original i =0x3c0031ce
//	// D = 0.9998f, i =0x3e52fbad ++
//	// D = 0.9997f, i=0x3e610c3a ++
//	// D = 0.9995f, i = 0x3e8383dc ++
//	// D = 0.9991f, i = 0x3f210979 --
//	// D = 0.9992f, i = 0x3f272510 --
//	// D = 0.9993f, i = 0x3f2d7195 --
//	// D = 0.9994f, i = 0x3e9273a9 ++
//	// D = 0.99935f, i = 0x3e9e1283 ++
//
//	// D = 0.9993f, i = 0x3e19fd9d, d = 0.000488296151
//	// D = 0.9992f, i = 0x3e1b697a, d = 0.000488296151
//	// D = 0.9991f, i = 0x3e1cd47f, d = 0.000488296151
//	// D = 0.9990f, i = 0x3e1e3fa1, d = 0.000488296151
//	// D = 0.9989f, i = 0x3e1faa7e, d = 0.000488296151
//	// D = 0.9985f, i = 0x3e254fd6, d = 0.000488296151
//	// D = 0.9980f, i = 0x3e2c510e, d = 0.000488296151
//	// D = 0.9950f, i = 0x3e547cf4, d = 0.000488296151
//	// D = 0.9940f, i = 0x3e6110a0, d = 0.000488296151
//	// D = 0.9935f, i = 0x3e6732fb, d = 0.000488296151
//	// D = 0.9934f, i = 0x3e6869ca, d = 0.000488296151
//	// D = 0.9934f, C = 0.9934f, i = 0x3e6872f0, d = 0.000488296151
//	// D = 0.9934f, C = 0.9800f, i = 0x3e6885cb, d = 0.000488296151
//	// D = 0.9934f, C = 0.9750f, i = 0x3e688c83, d = 0.000488296151
//	// D = 0.9934f, C = 0.9740f, i = 0x3e688e04, d = 0.000488296151
//	// D = 0.9934f, C = 0.9740f, B = 1.0010f, i = 0x3e688eee, d = 0.000488296151
//
//	for(uint32_t i = 0x30000000; i <= 0x3F800000; i += 1)
//	{
//		float f = bit_cast<float>(i);
//		float y = f * bit_cast<float>(0x3e22f983);  // 1/2pi 0.15915494309  0x3e22f983
//
//		y = y - roundf(y);
//		float z = y;
//
//		const float A = bit_cast<float>(0x3e35e52b) * 0.9900f;  //6.28318530718f * bit_cast<float>(0x3ce7989c);  // 0.0282710114332f; 0x3ce7989c
//		const float B = bit_cast<float>(0xbfa07e08) * 1.0011f;  //6.28318530718f * bit_cast<float>(0xbe4c585c);  // -0.19955581819f;0xbe4c585c
//		const float C = bit_cast<float>(0x3d9f2f29) * 0.9740f;  //6.28318530718f * bit_cast<float>(0x3c4aadfd);  // 0.012370583689f; 0x3c4aadfd
//		const float D = 6.28318530718f * 0.9934f;               //bit_cast<float>(0x3F700000);  // 1.0f; 0x3F700000
//
//		float ref = sinf(f);
//		float val = (((A * z + B) * z + C) * z + D) * z;
//
//		float diff = val - ref;
//		float err = abs(diff);
//		const float tolerance = powf(2.0f, -11.0f);
//
//		//const float pi = 3.1415926535f;
//		//float x = f;
//		//float r = (-16 * pi + 128 * sqrt(2) - 128) / powf(pi, 4) * powf(x, 4) + (20 * pi - 128 * sqrt(2) + 112) / powf(pi, 3) * powf(x, 3) + (32 * sqrt(2) - 8 * pi - 20) / powf(pi, 2) * powf(x, 2) + x;
//
//		if(err <= tolerance)
//		{
//		}
//		else
//		{
//			EXPECT_TRUE(err <= tolerance);
//		}
//	}
//}

TEST(MathTest, CosEvenExhaustive)
{
	const float pi = 3.1415926535f;

	for(uint32_t i = 0x00000000; i <= 0x3F800000; i += 1)
	{
		float f = bit_cast<float>(i);
		float y = f * bit_cast<float>(0x3e22f983);  // 1/2pi 0.15915494309  0x3e22f983

		y = y - roundf(y);
		float z = y;

		const float A = powf(6.28318530718f, 6) * (4860 * sqrtf(3) - 8424) / (5 * powf(pi, 6));
		const float B = powf(6.28318530718f, 4) * (612 - 351 * sqrtf(3)) / powf(pi, 4);
		const float C = powf(6.28318530718f, 2) * (270 * sqrtf(3) - 517) / (10 * powf(pi, 2));
		const float D = 1.0f;

		float ref = cosf(f);
		float z2 = z * z;
		float val = ((A * z2 + B) * z2 + C) * z2 + D;

		float diff = val - ref;
		float err = abs(diff);
		const float tolerance = powf(2.0f, -17.0f);

		if(err <= tolerance)
		{
		}
		else
		{
			EXPECT_TRUE(err <= tolerance);
		}
	}
}

TEST(MathTest, SinExhaustive)
{
	const float pi = 3.1415926535f;

	// A = 1.00000f, B = 1.00000f, C = 1.00000f, D = 0.99985f, i = 0x3e4cb6cf +

	for(uint32_t i = 0x30000000; i <= 0x3F800000; i += 1)
	{
		float f = bit_cast<float>(i);
		float y = f * bit_cast<float>(0x3e22f983);  // 1/2pi 0.15915494309  0x3e22f983

		y = y - roundf(y);
		float z = y;

		const float A = 1.00000f * powf(6.28318530718f, 4) * (-16 * pi + 128 * sqrt(2) - 128) / powf(pi, 4);
		const float B = 1.00000f * powf(6.28318530718f, 3) * (20 * pi - 128 * sqrt(2) + 112) / powf(pi, 3);
		const float C = 1.00000f * powf(6.28318530718f, 2) * (32 * sqrt(2) - 8 * pi - 20) / powf(pi, 2);
		const float D = 0.99983f * powf(6.28318530718f, 1) * 1.0f;

		volatile float ff = bit_cast<float>(0.9999f);

		float ref = sinf(f);
		float val = (((A * z + B) * z + C) * z + D) * z;

		float diff = val - ref;
		float err = abs(diff);
		const float tolerance = powf(2.0f, -11.5f);

		//
		//float x = f;
		//float r = (-16 * pi + 128 * sqrt(2) - 128) / powf(pi, 4) * powf(x, 4) +
		//          (20 * pi - 128 * sqrt(2) + 112) / powf(pi, 3) * powf(x, 3) +
		//          (32 * sqrt(2) - 8 * pi - 20) / powf(pi, 2) * powf(x, 2) +
		//          x;

		if(err <= tolerance)
		{
		}
		else
		{
			EXPECT_TRUE(err <= tolerance);
		}
	}
}

TEST(MathTest, CosExhaustive)
{
	const float pi = 3.1415926535f;

	for(uint32_t i = 0x00000000; i <= 0x3F800000; i += 1)
	{
		float f = bit_cast<float>(i);
		float y = f * bit_cast<float>(0x3e22f983);  // 1/2pi 0.15915494309  0x3e22f983

		y = y - roundf(y);
		float z = y;

		const float A = powf(6.28318530718f, 4) * (-16 * pi + 128 * sqrt(2) - 128) / powf(pi, 4);
		const float B = powf(6.28318530718f, 3) * (12 * pi - 128 * sqrt(2) + 144) / powf(pi, 3);
		const float C = powf(6.28318530718f, 2) * (-2 * pi + 32 * sqrt(2) - 44) / powf(pi, 2);
		const float D = 0.0f;
		const float E = 1.0f;

		float ref = cosf(f);
		float val = (((A * z + B) * z + C) * z + D) * z + E;

		//float x = f;
		//float r = (-16 * pi + 128 * sqrt(2) - 128) / powf(pi, 4) * powf(x, 4) +
		//          (12 * pi - 128 * sqrt(2) + 144) / powf(pi, 3) * powf(x, 3) +
		//          (-2 * pi + 32 * sqrt(2) - 44) / powf(pi, 2) * powf(x, 2) +
		//          1;

		float diff = val - ref;
		float err = abs(diff);
		const float tolerance = powf(2.0f, -11.0f);

		if(err <= tolerance)
		{
		}
		else
		{
			EXPECT_TRUE(err <= tolerance);
		}
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
