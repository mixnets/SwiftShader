// Copyright 2020 The SwiftShader Authors. All Rights Reserved.
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

#include "System/Hash.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace sw;

namespace {
enum class Enum
{
	A,
	B,
	C
};
}

// Check that sw::hash<> gives the same hash value for the same inputs.
TEST(Hash, Deterministic)
{
	ASSERT_EQ(sw::hash(std::string("abc")), sw::hash(std::string("abc")));
	ASSERT_EQ(sw::hash(true), sw::hash(true));
	ASSERT_EQ(sw::hash(static_cast<int>(123)), sw::hash(static_cast<int>(123)));
	ASSERT_EQ(sw::hash(static_cast<char>(123)), sw::hash(static_cast<char>(123)));
	ASSERT_EQ(sw::hash(static_cast<int8_t>(123)), sw::hash(static_cast<int8_t>(123)));
	ASSERT_EQ(sw::hash(static_cast<int16_t>(123)), sw::hash(static_cast<int16_t>(123)));
	ASSERT_EQ(sw::hash(static_cast<int32_t>(123)), sw::hash(static_cast<int32_t>(123)));
	ASSERT_EQ(sw::hash(static_cast<int64_t>(123)), sw::hash(static_cast<int64_t>(123)));
	ASSERT_EQ(sw::hash(static_cast<uint8_t>(123)), sw::hash(static_cast<uint8_t>(123)));
	ASSERT_EQ(sw::hash(static_cast<uint16_t>(123)), sw::hash(static_cast<uint16_t>(123)));
	ASSERT_EQ(sw::hash(static_cast<uint32_t>(123)), sw::hash(static_cast<uint32_t>(123)));
	ASSERT_EQ(sw::hash(static_cast<uint64_t>(123)), sw::hash(static_cast<uint64_t>(123)));

	ASSERT_EQ(sw::hash(1, 2, 3), sw::hash(1, 2, 3));
	ASSERT_EQ(sw::hash(false, true, false), sw::hash(false, true, false));
	ASSERT_EQ(sw::hash(Enum::A, Enum::B), sw::hash(Enum::A, Enum::B));
}

// Check that sw::hash<> gives the different results for different values.
TEST(Hash, DifferentInputs)
{
	ASSERT_NE(sw::hash(std::string("abc")), sw::hash(std::string("def")));
	ASSERT_NE(sw::hash(true), sw::hash(false));
	ASSERT_NE(sw::hash(static_cast<int>(123)), sw::hash(static_cast<int>(321)));
	ASSERT_NE(sw::hash(static_cast<char>(123)), sw::hash(static_cast<char>(321)));
	ASSERT_NE(sw::hash(static_cast<int8_t>(123)), sw::hash(static_cast<int8_t>(321)));
	ASSERT_NE(sw::hash(static_cast<int16_t>(123)), sw::hash(static_cast<int16_t>(321)));
	ASSERT_NE(sw::hash(static_cast<int32_t>(123)), sw::hash(static_cast<int32_t>(321)));
	ASSERT_NE(sw::hash(static_cast<int64_t>(123)), sw::hash(static_cast<int64_t>(321)));
	ASSERT_NE(sw::hash(static_cast<uint8_t>(123)), sw::hash(static_cast<uint8_t>(321)));
	ASSERT_NE(sw::hash(static_cast<uint16_t>(123)), sw::hash(static_cast<uint16_t>(321)));
	ASSERT_NE(sw::hash(static_cast<uint32_t>(123)), sw::hash(static_cast<uint32_t>(321)));
	ASSERT_NE(sw::hash(static_cast<uint64_t>(123)), sw::hash(static_cast<uint64_t>(321)));

	ASSERT_NE(sw::hash(1, 2, 3), sw::hash(6, 7, 8));
	ASSERT_NE(sw::hash(false, true, false), sw::hash(true, false, true));
	ASSERT_NE(sw::hash(Enum::A, Enum::B), sw::hash(Enum::B, Enum::C));
}

// Check that sw::hash<> gives the different results for arguments in a different order.
TEST(Hash, Order)
{
	ASSERT_NE(sw::hash(1, 2), sw::hash(2, 1));
	ASSERT_NE(sw::hash(1, 2, 3), sw::hash(3, 2, 1));
	ASSERT_NE(sw::hash(false, true), sw::hash(true, false));
	ASSERT_NE(sw::hash(Enum::A, Enum::B), sw::hash(Enum::B, Enum::A));
}
