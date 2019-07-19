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

#if !defined(_WIN32) && !defined(_XOPEN_SOURCE)
// This must come before other #includes, otherwise we'll end up with ucontext_t
// definition mismatches, leading to memory corruption hilarity.
#define _XOPEN_SOURCE
#endif //  !defined(_WIN32) && !defined(_XOPEN_SOURCE)

#include "Fiber.hpp"
#include "Scheduler.hpp"

#include <assert.h>

#include <atomic>
#include <functional>
#include <memory>

#if defined(_WIN32)
#	include <Windows.h>
#else
#	include <ucontext.h>
#endif

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif // defined(__clang__)

/*
TODO: In theory, we should be using annotations to tell sanitizers that we're
switching stacks. However, so far we've seen no false-positives.

#ifdef(_asan_enabled_)
extern "C"
{
	extern void __sanitizer_start_switch_fiber(void **fake_stack_save, const void *bottom, size_t size);
	extern void __sanitizer_finish_switch_fiber(void *fake_stack_save, const void **bottom_old, size_t *size_old);
}
#endif // _asan_enabled_
*/

namespace
{

std::atomic<uint32_t> nextFiberID = { 0 };

} // anonymous namespace

namespace yarn {

////////////////////////////////////////////////////////////////////////////////
// OSFiber
////////////////////////////////////////////////////////////////////////////////
class OSFiber
{
public:
	~OSFiber();
	static OSFiber* createFiberFromCurrentThread(size_t stackSize);
	static OSFiber* createFiber(size_t stackSize, const std::function<void()>& func);
	void switchTo(OSFiber*);

private:
	OSFiber();

#if defined(_WIN32)
	static void WINAPI run(void* self);
	LPVOID fiber = nullptr;
	bool isFiberFromThread = false;
	std::function<void()> target;
#else
	size_t stackSize = 0;
	std::unique_ptr<uint8_t[]> stack;
	ucontext_t context;
	std::function<void()> target;
#endif
};

#if defined(_WIN32)

OSFiber::OSFiber() = default;

OSFiber::~OSFiber()
{
	if (fiber != nullptr)
	{
		if (isFiberFromThread)
		{
			ConvertFiberToThread();
		}
		else
		{
			DeleteFiber(fiber);
		}
	}
}

OSFiber* OSFiber::createFiberFromCurrentThread(size_t stackSize)
{
	auto out = new OSFiber();
	out->fiber = ConvertThreadToFiber(nullptr);
	out->isFiberFromThread = true;
	return out;
}

OSFiber* OSFiber::createFiber(size_t stackSize, const std::function<void()>& func)
{
	auto out = new OSFiber();
	out->fiber = CreateFiber(stackSize, &OSFiber::run, out);
	out->target = func;
	return out;
}

void OSFiber::switchTo(OSFiber* fiber)
{
	SwitchToFiber(fiber->fiber);
}

void WINAPI OSFiber::run(void* self)
{
	std::function<void()> func;
	std::swap(func, reinterpret_cast<OSFiber*>(self)->target);
	func();
}

#else

OSFiber::OSFiber() = default;

OSFiber::~OSFiber() = default;

OSFiber* OSFiber::createFiberFromCurrentThread(size_t stackSize)
{
	auto out = new OSFiber();
	out->context = {};
	out->stackSize = stackSize;
	out->stack = std::unique_ptr<uint8_t[]>(new uint8_t[stackSize]);
	getcontext(&out->context);
	return out;
}

OSFiber* OSFiber::createFiber(size_t stackSize, const std::function<void()>& func)
{
	union Args
	{
		OSFiber* self;
		struct { int a; int b; };
	};

	struct Target
	{
		static void Main(int a, int b)
		{
			Args u;
			u.a = a; u.b = b;
			std::function<void()> func;
			std::swap(func, u.self->target);
			func();
		}
	};

	auto out = new OSFiber();
	out->context = {};
	out->stackSize = stackSize;
	out->stack = std::unique_ptr<uint8_t[]>(new uint8_t[stackSize]);
	out->target = func;

	auto alignmentOffset = 15 - (reinterpret_cast<uintptr_t>(out->stack.get() + 15) & 15);
	auto res = getcontext(&out->context);
	assert(res == 0); (void)res;
	out->context.uc_stack.ss_sp = out->stack.get() + alignmentOffset;
	out->context.uc_stack.ss_size = stackSize - alignmentOffset;
	out->context.uc_link = nullptr;

	Args args;
	args.self = out;
	makecontext(&out->context, reinterpret_cast<void(*)()>(&Target::Main), 2, args.a, args.b);

	return out;
}

void OSFiber::switchTo(OSFiber* fiber)
{
	auto res = swapcontext(&context, &fiber->context);
	assert(res == 0); (void)res;
}

#endif

////////////////////////////////////////////////////////////////////////////////
// Fiber
////////////////////////////////////////////////////////////////////////////////
Fiber::Fiber(OSFiber* impl) :
	impl(impl), id(nextFiberID++), worker(Scheduler::Worker::getCurrent())
{
	assert(worker != nullptr);
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
	assert(worker == Scheduler::Worker::getCurrent());
	worker->yield(this);
}

void Fiber::switchTo(Fiber* to)
{
	impl->switchTo(to->impl);
}

Fiber* Fiber::create(size_t stackSize, const std::function<void()>& func)
{
	return new Fiber(OSFiber::createFiber(stackSize, func));
}

Fiber* Fiber::createFromCurrentThread(size_t stackSize)
{
	return new Fiber(OSFiber::createFiberFromCurrentThread(stackSize));
}

}  // namespace yarn

#if defined(__clang__)
#pragma clang diagnostic pop
#endif // defined(__clang__)
