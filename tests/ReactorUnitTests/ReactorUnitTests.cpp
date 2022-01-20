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

#include "Assert.hpp"
#include "Coroutine.hpp"
#include "Print.hpp"
#include "Reactor.hpp"

#include "gtest/gtest.h"

#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <thread>
#include <tuple>

using namespace rr;

static std::string testName()
{
	auto info = ::testing::UnitTest::GetInstance()->current_test_info();
	return std::string{ info->test_suite_name() } + "_" + info->name();
}

template<typename TestFuncType, typename RefFuncType, typename TestValueType>
struct IntrinsicTestParams
{
	std::function<TestFuncType> testFunc;   // Function we're testing (Reactor)
	std::function<RefFuncType> refFunc;     // Reference function to test against (C)
	std::vector<TestValueType> testValues;  // Values to input to functions
};

using IntrinsicTestParams_Float = IntrinsicTestParams<RValue<Float>(RValue<Float>), float(float), float>;
using IntrinsicTestParams_Float4 = IntrinsicTestParams<RValue<Float4>(RValue<Float4>), float(float), float>;
using IntrinsicTestParams_Float4_Float4 = IntrinsicTestParams<RValue<Float4>(RValue<Float4>, RValue<Float4>), float(float, float), std::pair<float, float>>;

// TODO(b/147818976): Each function has its own precision requirements for Vulkan, sometimes broken down
// by input range. These are currently validated by deqp, but we can improve our own tests as well.
// See https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#spirvenv-precision-operation
constexpr double INTRINSIC_PRECISION = 1e-4;

struct IntrinsicTest_Float : public testing::TestWithParam<IntrinsicTestParams_Float>
{
	void test()
	{
		FunctionT<float(float)> function;
		{
			Return(GetParam().testFunc((Float(function.Arg<0>()))));
		}

		auto routine = function(testName().c_str());

		for(auto &&v : GetParam().testValues)
		{
			SCOPED_TRACE(v);
			EXPECT_NEAR(routine(v), GetParam().refFunc(v), INTRINSIC_PRECISION);
		}
	}
};

using float4 = float[4];
using int4 = int[4];

// TODO: Move to Reactor.hpp
template<>
struct rr::CToReactor<int[4]>
{
	using type = Int4;
	static Int4 cast(float[4]);
};

// Value type wrapper around a <type>[4] (i.e. float4, int4)
template<typename T>
struct type4_value
{
	using E = typename std::remove_pointer_t<std::decay_t<T>>;

	type4_value() = default;
	explicit type4_value(E rep)
	    : v{ rep, rep, rep, rep }
	{}
	type4_value(E x, E y, E z, E w)
	    : v{ x, y, z, w }
	{}

	bool operator==(const type4_value &rhs) const
	{
		return std::equal(std::begin(v), std::end(v), rhs.v);
	}

	// For gtest printing
	friend std::ostream &operator<<(std::ostream &os, const type4_value &value)
	{
		return os << "[" << value.v[0] << ", " << value.v[1] << ", " << value.v[2] << ", " << value.v[3] << "]";
	}

	T v;
};

using float4_value = type4_value<float4>;
using int4_value = type4_value<int4>;

// Invoke a void(type4_value<T>*) routine on &v.v, returning wrapped result in v
template<typename RoutineType, typename T>
type4_value<T> invokeRoutine(RoutineType &routine, type4_value<T> v)
{
	routine(&v.v);
	return v;
}

// Invoke a void(type4_value<T>*, type4_value<T>*) routine on &v1.v, &v2.v returning wrapped result in v1
template<typename RoutineType, typename T>
type4_value<T> invokeRoutine(RoutineType &routine, type4_value<T> v1, type4_value<T> v2)
{
	routine(&v1.v, &v2.v);
	return v1;
}

struct IntrinsicTest_Float4 : public testing::TestWithParam<IntrinsicTestParams_Float4>
{
	void test()
	{
		FunctionT<void(float4 *)> function;
		{
			Pointer<Float4> a = function.Arg<0>();
			*a = GetParam().testFunc(*a);
			Return();
		}

		auto routine = function(testName().c_str());

		for(auto &&v : GetParam().testValues)
		{
			SCOPED_TRACE(v);
			float4_value result = invokeRoutine(routine, float4_value{ v });
			float4_value expected = float4_value{ GetParam().refFunc(v) };
			EXPECT_NEAR(result.v[0], expected.v[0], INTRINSIC_PRECISION);
			EXPECT_NEAR(result.v[1], expected.v[1], INTRINSIC_PRECISION);
			EXPECT_NEAR(result.v[2], expected.v[2], INTRINSIC_PRECISION);
			EXPECT_NEAR(result.v[3], expected.v[3], INTRINSIC_PRECISION);
		}
	}
};

struct IntrinsicTest_Float4_Float4 : public testing::TestWithParam<IntrinsicTestParams_Float4_Float4>
{
	void test()
	{
		FunctionT<void(float4 *, float4 *)> function;
		{
			Pointer<Float4> a = function.Arg<0>();
			Pointer<Float4> b = function.Arg<1>();
			*a = GetParam().testFunc(*a, *b);
			Return();
		}

		auto routine = function(testName().c_str());

		for(auto &&v : GetParam().testValues)
		{
			SCOPED_TRACE(v);
			float4_value result = invokeRoutine(routine, float4_value{ v.first }, float4_value{ v.second });
			float4_value expected = float4_value{ GetParam().refFunc(v.first, v.second) };
			EXPECT_NEAR(result.v[0], expected.v[0], INTRINSIC_PRECISION);
			EXPECT_NEAR(result.v[1], expected.v[1], INTRINSIC_PRECISION);
			EXPECT_NEAR(result.v[2], expected.v[2], INTRINSIC_PRECISION);
			EXPECT_NEAR(result.v[3], expected.v[3], INTRINSIC_PRECISION);
		}
	}
};

// clang-format off
INSTANTIATE_TEST_SUITE_P(IntrinsicTestParams_Float, IntrinsicTest_Float, testing::Values(
	IntrinsicTestParams_Float{ [](Float v) { return rr::Exp2(v); }, exp2f, {0.f, 1.f, 123.f} },
	IntrinsicTestParams_Float{ [](Float v) { return rr::Log2(v); }, log2f, {1.f, 123.f} },
	IntrinsicTestParams_Float{ [](Float v) { return rr::Sqrt(v); }, sqrtf, {0.f, 1.f, 123.f} }
));
// clang-format on

// TODO(b/149110874) Use coshf/sinhf when we've implemented SpirV versions at the SpirV level
float vulkan_sinhf(float a)
{
	return ((expf(a) - expf(-a)) / 2);
}
float vulkan_coshf(float a)
{
	return ((expf(a) + expf(-a)) / 2);
}

// clang-format off
constexpr float PI = 3.141592653589793f;
INSTANTIATE_TEST_SUITE_P(IntrinsicTestParams_Float4, IntrinsicTest_Float4, testing::Values(
	IntrinsicTestParams_Float4{ [](RValue<Float4> v) { return rr::Sin(v); },                    sinf,          {0.f, 1.f, PI, 123.f}  },
	IntrinsicTestParams_Float4{ [](RValue<Float4> v) { return rr::Cos(v); },                    cosf,          {0.f, 1.f, PI, 123.f}  }
	//IntrinsicTestParams_Float4{ [](RValue<Float4> v) { return rr::Tan(v); },                    tanf,          {0.f, 1.f, PI, 123.f}  },
	//IntrinsicTestParams_Float4{ [](RValue<Float4> v) { return rr::Asin(v, Precision::Full); },  asinf,         {0.f, 1.f, -1.f}  },
	//IntrinsicTestParams_Float4{ [](RValue<Float4> v) { return rr::Acos(v, Precision::Full); },  acosf,         {0.f, 1.f, -1.f}  },
	//IntrinsicTestParams_Float4{ [](RValue<Float4> v) { return rr::Atan(v); },                   atanf,         {0.f, 1.f, PI, 123.f}  },
	//IntrinsicTestParams_Float4{ [](RValue<Float4> v) { return rr::Sinh(v); },                   vulkan_sinhf,  {0.f, 1.f, PI}  },
	//IntrinsicTestParams_Float4{ [](RValue<Float4> v) { return rr::Cosh(v); },                   vulkan_coshf,  {0.f, 1.f, PI} },
	//IntrinsicTestParams_Float4{ [](RValue<Float4> v) { return rr::Tanh(v); },                   tanhf,         {0.f, 1.f, PI}  },
	//IntrinsicTestParams_Float4{ [](RValue<Float4> v) { return rr::Asinh(v); },                  asinhf,        {0.f, 1.f, PI, 123.f}  },
	//IntrinsicTestParams_Float4{ [](RValue<Float4> v) { return rr::Acosh(v); },                  acoshf,        {     1.f, PI, 123.f}  },
	//IntrinsicTestParams_Float4{ [](RValue<Float4> v) { return rr::Atanh(v); },                  atanhf,        {0.f, 0.9999f, -0.9999f}  },
	//IntrinsicTestParams_Float4{ [](RValue<Float4> v) { return rr::Exp(v); },                    expf,          {0.f, 1.f, PI}  },
	//IntrinsicTestParams_Float4{ [](RValue<Float4> v) { return rr::Log(v); },                    logf,          {1.f, PI, 123.f}  },
	//IntrinsicTestParams_Float4{ [](RValue<Float4> v) { return rr::Exp2(v); },                   exp2f,         {0.f, 1.f, PI, 123.f}  },
	//IntrinsicTestParams_Float4{ [](RValue<Float4> v) { return rr::Log2(v); },                   log2f,         {1.f, PI, 123.f}  },
	//IntrinsicTestParams_Float4{ [](RValue<Float4> v) { return rr::Sqrt(v); },                   sqrtf,         {0.f, 1.f, PI, 123.f}  }
));
// clang-format on

// clang-format off
INSTANTIATE_TEST_SUITE_P(IntrinsicTestParams_Float4_Float4, IntrinsicTest_Float4_Float4, testing::Values(
	IntrinsicTestParams_Float4_Float4{ [](RValue<Float4> v1, RValue<Float4> v2) { return Atan2(v1, v2); }, atan2f, { {0.f, 0.f}, {0.f, -1.f}, {-1.f, 0.f}, {123.f, 123.f} } },
	IntrinsicTestParams_Float4_Float4{ [](RValue<Float4> v1, RValue<Float4> v2) { return Pow(v1, v2); },   powf,   { {1.f, 0.f}, {1.f, -1.f}, {-1.f, 0.f} } }
));
// clang-format on

TEST_P(IntrinsicTest_Float4_Float4, Test)
{
	test();
}

// For gtest printing of pairs
namespace std {
template<typename T, typename U>
std::ostream &operator<<(std::ostream &os, const std::pair<T, U> &value)
{
	return os << "{ " << value.first << ", " << value.second << " }";
}
}  // namespace std

class StdOutCapture
{
public:
	~StdOutCapture()
	{
		stopIfCapturing();
	}

	void start()
	{
		stopIfCapturing();
		capturing = true;
		testing::internal::CaptureStdout();
	}

	std::string stop()
	{
		assert(capturing);
		capturing = false;
		return testing::internal::GetCapturedStdout();
	}

private:
	void stopIfCapturing()
	{
		if(capturing)
		{
			// This stops the capture
			testing::internal::GetCapturedStdout();
		}
	}

	bool capturing = false;
};

std::vector<std::string> split(const std::string &s)
{
	std::vector<std::string> result;
	std::istringstream iss(s);
	for(std::string line; std::getline(iss, line);)
	{
		result.push_back(line);
	}
	return result;
}
