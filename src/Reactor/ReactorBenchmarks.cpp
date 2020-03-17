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

#include "Coroutine.hpp"
#include "Reactor.hpp"

#include "benchmark/benchmark.h"

BENCHMARK_MAIN();

namespace {
static rr::CoroutineRuntime const *const runtimes[] = {
	rr::CoroutineRuntime::Marl,
	rr::CoroutineRuntime::UContext,
};
static constexpr size_t numRuntimes = sizeof(runtimes) / sizeof(runtimes[0]);
}  // namespace

class Coroutines : public benchmark::Fixture
{
public:
	void SetUp(const ::benchmark::State &state)
	{
		rr::CoroutineRuntime::Set(runtime(state));
	}

	void TearDown(const ::benchmark::State &state) {}

	static rr::CoroutineRuntime const *runtime(const ::benchmark::State &state)
	{
		return runtimes[state.range(0)];
	}

	static size_t iterations(const ::benchmark::State &state)
	{
		return static_cast<size_t>(state.range(1));
	}

	static void args(benchmark::internal::Benchmark *b)
	{
		b->ArgNames({ "runtime", "iterations" });
		for(unsigned int runtime = 0; runtime < numRuntimes; ++runtime)
		{
			for(unsigned int iterations = 1U; iterations <= 0x1000000; iterations *= 8)
			{
				b->Args({ runtime, iterations });
			}
		}
	}
};

BENCHMARK_DEFINE_F(Coroutines, Fibonacci)
(benchmark::State &state)
{
	using namespace rr;

	if(!Caps.CoroutinesSupported)
	{
		state.SkipWithError("Coroutines are not supported");
		return;
	}

	Coroutine<int()> function;
	{
		Yield(Int(0));
		Yield(Int(1));
		Int current = 1;
		Int next = 1;
		While(true)
		{
			Yield(next);
			auto tmp = current + next;
			current = next;
			next = tmp;
		}
	}

	auto coroutine = function();

	int out = 0;
	for(auto _ : state)
	{
		for(int64_t i = 0, c = iterations(state); i < c; i++)
		{
			coroutine->await(out);
		}
	}
}

BENCHMARK_REGISTER_F(Coroutines, Fibonacci)->Apply(Coroutines::args);
