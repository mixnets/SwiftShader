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

#if defined(_WIN32)
#include "OSFiber_windows.hpp"
#elif defined(YARN_FIBERS_USE_UCONTEXT)
#include "OSFiber_ucontext.hpp"
#else
#include "OSFiber_asm.hpp"
#endif

#include "Fiber.hpp"

#include "Debug.hpp"
#include "Trace.hpp"

#include <functional>
#include <memory>

namespace yarn {

////////////////////////////////////////////////////////////////////////////////
// Fiber
////////////////////////////////////////////////////////////////////////////////
Fiber::Fiber(OSFiber* impl, uint32_t id) :
	impl(impl), id(id), worker(Scheduler::Worker::getCurrent())
{
	YARN_ASSERT(worker != nullptr, "No Scheduler::Worker bound");
}

Fiber::~Fiber()
{
	delete impl;
}

Fiber* Fiber::current()
{
	auto worker = Scheduler::Worker::getCurrent();
	return worker != nullptr ? worker->getCurrentFiber() : nullptr;
}

void Fiber::schedule()
{
	worker->enqueue(this);
}

void Fiber::yield()
{
	SCOPED_EVENT("YIELD");
	worker->yield(this);
}

void Fiber::switchTo(Fiber* to)
{
	if (to != this)
	{
		impl->switchTo(to->impl);
	}
}

Fiber* Fiber::create(uint32_t id, size_t stackSize, const std::function<void()>& func)
{
	return new Fiber(OSFiber::createFiber(stackSize, func), id);
}

Fiber* Fiber::createFromCurrentThread(uint32_t id, size_t stackSize)
{
	return new Fiber(OSFiber::createFiberFromCurrentThread(stackSize), id);
}

}  // namespace yarn
