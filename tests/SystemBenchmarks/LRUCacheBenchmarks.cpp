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

#include "System/LRUCache.hpp"

#include "benchmark/benchmark.h"

#include <array>

namespace {

// https://en.wikipedia.org/wiki/Xorshift
class FastRnd
{
public:
	inline size_t operator()()
	{
		x ^= x << 13;
		x ^= x >> 7;
		x ^= x << 17;
		return x;
	}

private:
	size_t x = 3243298;
};

struct ComplexKey
{
	std::array<size_t, 8> words;
};

bool operator==(const ComplexKey &a, const ComplexKey &b)
{
	for(size_t w = 0; w < a.words.size(); w++)
	{
		if(a.words[w] != b.words[w]) { return false; }
	}
	return true;
}

struct ComplexKeyHash
{
	size_t operator()(const ComplexKey &key) const
	{
		size_t hash = 12227;
		for(size_t w = 0; w < key.words.size(); w++)
		{
			hash = hash * 11801 + key.words[w];
		}
		return hash;
	}
};

}  // namespace

class LRUCacheBenchmark : public benchmark::Fixture
{
public:
	void SetUp(const ::benchmark::State &state)
	{
		size = state.range(0);
	}

	void TearDown(const ::benchmark::State &state) {}

	size_t size;
};

BENCHMARK_DEFINE_F(LRUCacheBenchmark, AddInt)
(benchmark::State &state)
{
	sw::LRUCache<size_t, size_t> cache(size);
	FastRnd rnd;

	int i = 0;
	for(auto _ : state)
	{
		cache.add(rnd() % size, i);
		i++;
	}
}
BENCHMARK_REGISTER_F(LRUCacheBenchmark, AddInt)->RangeMultiplier(8)->Range(1, 0x100000)->ArgName("cache-size");

BENCHMARK_DEFINE_F(LRUCacheBenchmark, GetIntCacheHit)
(benchmark::State &state)
{
	sw::LRUCache<size_t, size_t> cache(size);
	FastRnd rnd;

	for(size_t i = 0; i < size; i++)
	{
		cache.add(i, i);
	}

	for(auto _ : state)
	{
		cache.lookup(rnd() % size);
	}
}
BENCHMARK_REGISTER_F(LRUCacheBenchmark, GetIntCacheHit)->RangeMultiplier(8)->Range(1, 0x100000)->ArgName("cache-size");

BENCHMARK_DEFINE_F(LRUCacheBenchmark, GetIntCacheMiss)
(benchmark::State &state)
{
	sw::LRUCache<size_t, size_t> cache(size);
	FastRnd rnd;
	for(size_t i = 0; i < size; i++)
	{
		cache.add(size + i, i);
	}

	for(auto _ : state)
	{
		cache.lookup(rnd() % size);
	}
}
BENCHMARK_REGISTER_F(LRUCacheBenchmark, GetIntCacheMiss)->RangeMultiplier(8)->Range(1, 0x100000)->ArgName("cache-size");

BENCHMARK_DEFINE_F(LRUCacheBenchmark, AddRandomComplexKey)
(benchmark::State &state)
{
	sw::LRUCache<ComplexKey, size_t, ComplexKeyHash> cache(size);
	FastRnd rnd;

	int i = 0;
	for(auto _ : state)
	{
		ComplexKey key;
		for(size_t w = 0; w < key.words.size(); w++)
		{
			key.words[w] = rnd() & 1;
		}

		cache.add(key, i);
		i++;
	}
}
BENCHMARK_REGISTER_F(LRUCacheBenchmark, AddRandomComplexKey)->RangeMultiplier(8)->Range(1, 0x100000)->ArgName("cache-size");

BENCHMARK_DEFINE_F(LRUCacheBenchmark, GetComplexKeyCacheHit)
(benchmark::State &state)
{
	sw::LRUCache<ComplexKey, size_t, ComplexKeyHash> cache(size);
	FastRnd rnd;

	for(size_t i = 0; i < size; i++)
	{
		ComplexKey key;
		for(size_t w = 0; w < key.words.size(); w++)
		{
			key.words[w] = (1ull << w);
		}
		cache.add(key, i);
	}

	for(auto _ : state)
	{
		auto i = rnd() & size;

		ComplexKey key;
		for(size_t w = 0; w < key.words.size(); w++)
		{
			key.words[w] = i & (1ull << w);
		}
		cache.lookup(key);
	}
}
BENCHMARK_REGISTER_F(LRUCacheBenchmark, GetComplexKeyCacheHit)->RangeMultiplier(8)->Range(1, 0x100000)->ArgName("cache-size");

BENCHMARK_DEFINE_F(LRUCacheBenchmark, GetComplexKeyCacheMiss)
(benchmark::State &state)
{
	sw::LRUCache<ComplexKey, size_t, ComplexKeyHash> cache(size);
	FastRnd rnd;

	for(size_t i = 0; i < size; i++)
	{
		ComplexKey key;
		for(size_t w = 0; w < key.words.size(); w++)
		{
			key.words[w] = 8 + (1ull << w);
		}
		cache.add(key, i);
	}

	for(auto _ : state)
	{
		auto i = rnd() & size;

		ComplexKey key;
		for(size_t w = 0; w < key.words.size(); w++)
		{
			key.words[w] = i & (1ull << w);
		}
		cache.lookup(key);
	}
}
BENCHMARK_REGISTER_F(LRUCacheBenchmark, GetComplexKeyCacheMiss)->RangeMultiplier(8)->Range(1, 0x100000)->ArgName("cache-size");

#include "Reactor.hpp"
#include "ShaderCore.hpp"
using namespace rr;

// Macro that creates a lambda wrapper around the input overloaded function,
// creating a non-overload based on the args. This is useful for passing
// overloaded functions as template arguments.
// See https://stackoverflow.com/questions/25871381/c-overloaded-function-as-template-argument
#define LIFT(fname)                                          \
	[](auto &&... args) -> decltype(auto) {                  \
		return fname(std::forward<decltype(args)>(args)...); \
	}

template<typename Func, class... Args>
static void Transcedental1(benchmark::State &state, Func func, Args &&... args)
{
	FunctionT<float(float)> function;
	{
		Float a = function.Arg<0>();
		Float4 v{ a };
		Float4 r = func(v, args...);
		Return(Float(r.x));
	}

	auto routine = function("one");

	for(auto _ : state)
	{
		routine(1.f);
	}
}

template<typename Func, class... Args>
static void Transcedental2(benchmark::State &state, Func func, Args &&... args)
{
	FunctionT<float(float, float)> function;
	{
		Float a = function.Arg<0>();
		Float b = function.Arg<1>();
		Float4 v1{ a };
		Float4 v2{ b };
		Float4 r = func(v1, v2, args...);
		Return(Float(r.x));
	}

	auto routine = function("one");

	for(auto _ : state)
	{
		//routine(1.f, 1.f);
		routine(0.456f, 0.789f);
	}
}

BENCHMARK_CAPTURE(Transcedental1, rr_Sin, rr::Sin);
BENCHMARK_CAPTURE(Transcedental1, sw_sine_pp_true, sw::sine, true);
BENCHMARK_CAPTURE(Transcedental1, sw_sine_pp_false, sw::sine, false);

BENCHMARK_CAPTURE(Transcedental1, rr_Cos, rr::Cos);
BENCHMARK_CAPTURE(Transcedental1, sw_cosine_pp_true, sw::cosine, true);
BENCHMARK_CAPTURE(Transcedental1, sw_cosine_pp_false, sw::cosine, false);

BENCHMARK_CAPTURE(Transcedental1, rr_Tan, rr::Tan);
BENCHMARK_CAPTURE(Transcedental1, sw_tangent_pp_true, sw::tangent, true);
BENCHMARK_CAPTURE(Transcedental1, sw_tangent_pp_false, sw::tangent, false);

BENCHMARK_CAPTURE(Transcedental1, rr_Asin_fullp, rr::Asin, Precision::Full);
BENCHMARK_CAPTURE(Transcedental1, rr_Asin_relaxedp, rr::Asin, Precision::Relaxed);
BENCHMARK_CAPTURE(Transcedental1, sw_arcsin_pp_true, sw::arcsin, true);
BENCHMARK_CAPTURE(Transcedental1, sw_arcsin_pp_false, sw::arcsin, false);

BENCHMARK_CAPTURE(Transcedental1, rr_Acos_fullp, rr::Acos, Precision::Full);
BENCHMARK_CAPTURE(Transcedental1, rr_Acos_relaxedp, rr::Acos, Precision::Relaxed);
BENCHMARK_CAPTURE(Transcedental1, sw_arccos_pp_true, sw::arccos, true);
BENCHMARK_CAPTURE(Transcedental1, sw_arccos_pp_false, sw::arccos, false);

BENCHMARK_CAPTURE(Transcedental1, rr_Atan, rr::Atan);
BENCHMARK_CAPTURE(Transcedental1, sw_arctan_pp_true, LIFT(sw::arctan), true);
BENCHMARK_CAPTURE(Transcedental1, sw_arctan_pp_false, LIFT(sw::arctan), false);

BENCHMARK_CAPTURE(Transcedental1, rr_Sinh, rr::Sinh);
BENCHMARK_CAPTURE(Transcedental1, sw_sineh_pp_true, sw::sineh, true);
BENCHMARK_CAPTURE(Transcedental1, sw_sineh_pp_false, sw::sineh, false);

BENCHMARK_CAPTURE(Transcedental1, rr_Cosh, rr::Cosh);
BENCHMARK_CAPTURE(Transcedental1, sw_cosineh_pp_true, sw::cosineh, true);
BENCHMARK_CAPTURE(Transcedental1, sw_cosineh_pp_false, sw::cosineh, false);

BENCHMARK_CAPTURE(Transcedental1, rr_Tanh, rr::Tanh);
BENCHMARK_CAPTURE(Transcedental1, sw_tangenth_pp_true, sw::tangenth, true);
BENCHMARK_CAPTURE(Transcedental1, sw_tangenth_pp_false, sw::tangenth, false);

BENCHMARK_CAPTURE(Transcedental1, rr_Asinh, rr::Asinh);
BENCHMARK_CAPTURE(Transcedental1, sw_arcsinh_pp_true, sw::arcsinh, true);
BENCHMARK_CAPTURE(Transcedental1, sw_arcsinh_pp_false, sw::arcsinh, false);

BENCHMARK_CAPTURE(Transcedental1, rr_Acosh, rr::Acosh);
BENCHMARK_CAPTURE(Transcedental1, sw_arccosh_pp_true, sw::arccosh, true);
BENCHMARK_CAPTURE(Transcedental1, sw_arccosh_pp_false, sw::arccosh, false);

BENCHMARK_CAPTURE(Transcedental1, rr_Atanh, rr::Atanh);
BENCHMARK_CAPTURE(Transcedental1, sw_arctanh_pp_true, sw::arctanh, true);
BENCHMARK_CAPTURE(Transcedental1, sw_arctanh_pp_false, sw::arctanh, false);

BENCHMARK_CAPTURE(Transcedental2, rr_Atan2, rr::Atan2);
BENCHMARK_CAPTURE(Transcedental2, sw_arctan_pp_true, LIFT(sw::arctan), true);
BENCHMARK_CAPTURE(Transcedental2, sw_arctan_pp_false, LIFT(sw::arctan), false);

BENCHMARK_CAPTURE(Transcedental2, rr_Pow, rr::Pow);
BENCHMARK_CAPTURE(Transcedental2, sw_power_pp_true, sw::power, true);
BENCHMARK_CAPTURE(Transcedental2, sw_power_pp_false, sw::power, false);

BENCHMARK_CAPTURE(Transcedental1, rr_Exp, rr::Exp);
BENCHMARK_CAPTURE(Transcedental1, sw_exponential_pp_true, sw::exponential, true);
BENCHMARK_CAPTURE(Transcedental1, sw_exponential_pp_false, sw::exponential, false);

BENCHMARK_CAPTURE(Transcedental1, rr_Log, rr::Log);
BENCHMARK_CAPTURE(Transcedental1, sw_logarithm_pp_true, sw::logarithm, true);
BENCHMARK_CAPTURE(Transcedental1, sw_logarithm_pp_false, sw::logarithm, false);

BENCHMARK_CAPTURE(Transcedental1, rr_Exp2, LIFT(rr::Exp2));
BENCHMARK_CAPTURE(Transcedental1, sw_exponential2_pp_true, sw::exponential2, true);
BENCHMARK_CAPTURE(Transcedental1, sw_exponential2_pp_false, sw::exponential2, false);

BENCHMARK_CAPTURE(Transcedental1, rr_Log2, LIFT(rr::Log2));
BENCHMARK_CAPTURE(Transcedental1, sw_logarithm2_pp_true, sw::logarithm2, true);
BENCHMARK_CAPTURE(Transcedental1, sw_logarithm2_pp_false, sw::logarithm2, false);

BENCHMARK_CAPTURE(Transcedental1, rr_Rcp_pp_exactAtPow2_true, LIFT(rr::Rcp_pp), true);
BENCHMARK_CAPTURE(Transcedental1, rr_Rcp_pp_exactAtPow2_false, LIFT(rr::Rcp_pp), false);
BENCHMARK_CAPTURE(Transcedental1, sw_reciprocal_exactAtPow2_true_pp_true, sw::reciprocal, true, false, true);
BENCHMARK_CAPTURE(Transcedental1, sw_reciprocal_exactAtPow2_true_pp_false, sw::reciprocal, false, false, true);
BENCHMARK_CAPTURE(Transcedental1, sw_reciprocal_exactAtPow2_false_pp_true, sw::reciprocal, true, false, false);
BENCHMARK_CAPTURE(Transcedental1, sw_reciprocal_exactAtPow2_false_pp_false, sw::reciprocal, false, false, false);

BENCHMARK_CAPTURE(Transcedental1, rr_RcpSqrt_pp, LIFT(rr::RcpSqrt_pp));
BENCHMARK_CAPTURE(Transcedental1, sw_reciprocalSquareRoot_pp_true, sw::reciprocalSquareRoot, false, true);
BENCHMARK_CAPTURE(Transcedental1, sw_reciprocalSquareRoot_pp_false, sw::reciprocalSquareRoot, false, false);

//static void Transcedental_Test(benchmark::State &state)
//{
//	FunctionT<float(float, float)> function;
//	{
//		Float a = function.Arg<0>();
//		Float b = function.Arg<1>();
//		Float4 v1{ a };
//		Float4 v2{ b };
//		Float4 r = rr::Pow(v1, v2);
//		Return(Float(r.x));
//	}
//
//	auto routine = function("one");
//
//	for(auto _ : state)
//	{
//		routine(0.5f, 0.123f);
//	}
//}
//
//BENCHMARK_CAPTURE(Transcedental_Test, Test);
