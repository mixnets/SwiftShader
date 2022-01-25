// Copyright 2022 The SwiftShader Authors. All Rights Reserved.
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

#include "Scheduler.hpp"

#include "CPUID.hpp"
#include "Debug.hpp"
#include "SwiftConfig.hpp"

namespace {
marl::Thread::Core getCoreFromIndex(uint8_t coreIndex)
{
	marl::Thread::Core core = {};
#if defined(_WIN32)
	// We only support one processor group on Windows
	// when an explicit affinity mask is used.
	core.windows.group = 0;
	core.windows.index = coreIndex;
#else
	core.pthread.index = coreIndex;
#endif
	return core;
}

marl::Thread::Affinity getAffinityFromMask(uint64_t affinityMask)
{
	if(affinityMask == std::numeric_limits<uint64_t>::max())
	{
		return marl::Thread::Affinity::all();
	}

	ASSERT(affinityMask != 0);
	marl::containers::vector<marl::Thread::Core, 32> cores;
	uint8_t coreIndex = 0;
	while(affinityMask)
	{
		if(affinityMask & 1)
		{
			cores.push_back(getCoreFromIndex(coreIndex));
		}
		++coreIndex;
		affinityMask >>= 1;
	}

	return marl::Thread::Affinity(cores, marl::Allocator::Default);
}

std::shared_ptr<marl::Thread::Affinity::Policy> getAffinityPolicy(marl::Thread::Affinity &&affinity, sw::Configuration::AffinityPolicy affinityPolicy)
{
	switch(affinityPolicy)
	{
	case sw::Configuration::AffinityPolicy::AnyOf:
		return marl::Thread::Affinity::Policy::anyOf(std::move(affinity));
	case sw::Configuration::AffinityPolicy::OneOf:
		return marl::Thread::Affinity::Policy::oneOf(std::move(affinity));
	default:
		UNREACHABLE("unknown affinity policy");
	}
	return nullptr;
}
}  // namespace

namespace sw {

Scheduler::Scheduler(const sw::Configuration &config)
{
	uint32_t threadCount = (config.threadCount == 0) ? std::min<size_t>(marl::Thread::numLogicalCPUs(), 16)
	                                                 : config.threadCount;
	auto affinity = getAffinityFromMask(config.affinityMask);
	auto affinityPolicy = getAffinityPolicy(std::move(affinity), config.affinityPolicy);

	marl::Scheduler::Config cfg;
	cfg.setWorkerThreadCount(threadCount);
	cfg.setWorkerThreadAffinityPolicy(affinityPolicy);
	cfg.setWorkerThreadInitializer([](int) {
		sw::CPUID::setFlushToZero(true);
		sw::CPUID::setDenormalsAreZero(true);
	});
	scheduler = std::make_unique<marl::Scheduler>(cfg);
}

marl::Scheduler *Scheduler::get()
{
	return scheduler.get();
}

}  // namespace sw