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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <cstdlib>

using namespace sw;

TEST(MathTest, SharedExponentSparse)
{
	for(uint64_t i = 0; i < 0x0000000100000000; i += 1000)
	{
		float f = bit_cast<float>(i);

		RGB9E5 ref(f, 0.0f, 0.0f, RGB9E5::ReferenceImplementation);
		RGB9E5 fast(f, 0.0f, 0.0f);

		EXPECT_EQ(ref, fast);
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

		RGB9E5 ref(r, g, b, RGB9E5::ReferenceImplementation);
		RGB9E5 fast(r, g, b);

		EXPECT_EQ(ref, fast);

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

		RGB9E5 ref(f, 0.0f, 0.0f, RGB9E5::ReferenceImplementation);
		RGB9E5 fast(f, 0.0f, 0.0f);

		EXPECT_EQ(ref, fast);
	}
}
