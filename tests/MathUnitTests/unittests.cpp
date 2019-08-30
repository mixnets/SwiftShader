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
#include "System/Hash.hpp"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <cstdlib>
#include <unordered_map>

using namespace sw;

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

TEST(MathTest, FastHashPOD)
{
	EXPECT_EQ(sw::FastHash::hash(static_cast<unsigned int>(99)), sw::FastHash::hash(static_cast<unsigned int>(99)));
	EXPECT_EQ(sw::FastHash::hash(static_cast<int>(99)), sw::FastHash::hash(static_cast<int>(99)));
	EXPECT_EQ(sw::FastHash::hash(static_cast<uint8_t>(99)), sw::FastHash::hash(static_cast<uint8_t>(99)));
	EXPECT_EQ(sw::FastHash::hash(static_cast<int8_t>(99)), sw::FastHash::hash(static_cast<int8_t>(99)));
	EXPECT_EQ(sw::FastHash::hash(static_cast<uint16_t>(99)), sw::FastHash::hash(static_cast<uint16_t>(99)));
	EXPECT_EQ(sw::FastHash::hash(static_cast<int16_t>(99)), sw::FastHash::hash(static_cast<int16_t>(99)));
	EXPECT_EQ(sw::FastHash::hash(static_cast<uint32_t>(99)), sw::FastHash::hash(static_cast<uint32_t>(99)));
	EXPECT_EQ(sw::FastHash::hash(static_cast<int32_t>(99)), sw::FastHash::hash(static_cast<int32_t>(99)));
	EXPECT_EQ(sw::FastHash::hash(static_cast<uint64_t>(99)), sw::FastHash::hash(static_cast<uint64_t>(99)));
	EXPECT_EQ(sw::FastHash::hash(static_cast<int64_t>(99)), sw::FastHash::hash(static_cast<int64_t>(99)));
	EXPECT_EQ(sw::FastHash::hash(99.0f), sw::FastHash::hash(99.0f));
	EXPECT_EQ(sw::FastHash::hash(99.0), sw::FastHash::hash(99.0));

	EXPECT_NE(sw::FastHash::hash(static_cast<unsigned int>(98)), sw::FastHash::hash(static_cast<unsigned int>(99)));
	EXPECT_NE(sw::FastHash::hash(static_cast<int>(98)), sw::FastHash::hash(static_cast<int>(99)));
	EXPECT_NE(sw::FastHash::hash(static_cast<uint8_t>(98)), sw::FastHash::hash(static_cast<uint8_t>(99)));
	EXPECT_NE(sw::FastHash::hash(static_cast<int8_t>(98)), sw::FastHash::hash(static_cast<int8_t>(99)));
	EXPECT_NE(sw::FastHash::hash(static_cast<uint16_t>(98)), sw::FastHash::hash(static_cast<uint16_t>(99)));
	EXPECT_NE(sw::FastHash::hash(static_cast<int16_t>(98)), sw::FastHash::hash(static_cast<int16_t>(99)));
	EXPECT_NE(sw::FastHash::hash(static_cast<uint32_t>(98)), sw::FastHash::hash(static_cast<uint32_t>(99)));
	EXPECT_NE(sw::FastHash::hash(static_cast<int32_t>(98)), sw::FastHash::hash(static_cast<int32_t>(99)));
	EXPECT_NE(sw::FastHash::hash(static_cast<uint64_t>(98)), sw::FastHash::hash(static_cast<uint64_t>(99)));
	EXPECT_NE(sw::FastHash::hash(static_cast<int64_t>(98)), sw::FastHash::hash(static_cast<int64_t>(99)));
	EXPECT_NE(sw::FastHash::hash(98.0f), sw::FastHash::hash(99.0f));
	EXPECT_NE(sw::FastHash::hash(98.0), sw::FastHash::hash(99.0));
}

TEST(MathTest, FastHashString)
{
	EXPECT_EQ(sw::FastHash::hash("abc"), sw::FastHash::hash("abc"));
	EXPECT_EQ(sw::FastHash::hash(std::string("abc")), sw::FastHash::hash(std::string("abc")));

	EXPECT_NE(sw::FastHash::hash("abc"), sw::FastHash::hash("def"));
	EXPECT_NE(sw::FastHash::hash(std::string("abc")), sw::FastHash::hash(std::string("def")));
}

struct Comparable
{
	static Comparable A;
	static Comparable B;

	Comparable() {}
	Comparable(int i, int8_t i8, uint32_t u32, int64_t i64, float f32, std::string s) :
		i(i), i8(i8), u32(u32), i64(i64), f32(f32), s(s) {}

	int i = 0;
	int8_t i8 = 0;
	uint32_t u32 = 0;
	int64_t i64 = 0;
	float f32 = 0;
	std::string s;

	SW_DECLARE_COMPARABLE(Comparable, i, i8, u32, i64, f32, s);
};

Comparable Comparable::A = { 10, 20, 30, 40, 50.0f, "60" };
Comparable Comparable::B = { 10, 20, 35, 40, 50.0f, "60" };

TEST(MathTest, ComparableOperators)
{
	EXPECT_TRUE(Comparable::A == Comparable::A);
	EXPECT_TRUE(Comparable::A != Comparable::B);
	EXPECT_TRUE(Comparable::A < Comparable::B);
	EXPECT_TRUE(Comparable::A <= Comparable::B);
	EXPECT_TRUE(Comparable::B > Comparable::A);
	EXPECT_TRUE(Comparable::B >= Comparable::A);
}

TEST(MathTest, FastHashComparable)
{
	EXPECT_EQ(sw::FastHash::hash(Comparable::A), sw::FastHash::hash(Comparable::A));
	EXPECT_NE(sw::FastHash::hash(Comparable::A), sw::FastHash::hash(Comparable::B));
}

TEST(MathTest, FastHashAsStdHash)
{
	std::unordered_map<Comparable, bool, sw::FastHash> map;
	map[Comparable::A] = true;
	map[Comparable::A] = true;
	EXPECT_EQ(map.size(), 1U);

	Comparable o = Comparable::A;
	o.s = "and now for something completely different";
	map[o] = true;
	EXPECT_EQ(map.size(), 2U);
}

TEST(MathTest, FastHashArrays)
{
	std::array<int, 5> a = {{ 1, 2, 3, 4, 5 }};
	std::array<int, 5> b = {{ 1, 2, 9, 4, 5 }};

	EXPECT_EQ(sw::FastHash::hash(a), sw::FastHash::hash(a));
	EXPECT_NE(sw::FastHash::hash(a), sw::FastHash::hash(b));
}