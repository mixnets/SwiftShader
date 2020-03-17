
#if !defined(ENABLE_MARL_COROUTINES)
#	define ENABLE_MARL_COROUTINES 1
#endif

#if !defined(ENABLE_UCONTEXT_COROUTINES) && (defined(__APPLE__) || defined(__linux__))
#	define ENABLE_UCONTEXT_COROUTINES 1
#endif

#if !defined(ENABLE_WIN32_COROUTINES) && (defined(_WIN32))
#	define ENABLE_WIN32_COROUTINES 1
#endif

#if ENABLE_UCONTEXT_COROUTINES
#	if !defined(_XOPEN_SOURCE)
// This must come before other #includes, otherwise we'll end up with ucontext_t
// definition mismatches, leading to memory corruption hilarity.
#		define _XOPEN_SOURCE
#	endif  //  !defined(_XOPEN_SOURCE)
#	include <ucontext.h>
#	if defined(__clang__)
#		pragma clang diagnostic push
#		pragma clang diagnostic ignored "-Wdeprecated-declarations"
#	endif  // defined(__clang__)
#endif

#include "Coroutine.hpp"
#include "Debug.hpp"

#if ENABLE_MARL_COROUTINES
#	include "marl/scheduler.h"
#	include "marl/event.h"
#endif

#if ENABLE_WIN32_COROUTINES
#	ifndef WIN32_LEAN_AND_MEAN
#		define WIN32_LEAN_AND_MEAN
#	endif  // !WIN32_LEAN_AND_MEAN
#	include <Windows.h>
#endif

#include <mutex>

namespace rr {
namespace {

////////////////////////////////////////////////////////////////////////////////
// Common
////////////////////////////////////////////////////////////////////////////////
template<typename T>
CoroutineRuntime bind()
{
	struct Binding
	{
		static Nucleus::CoroutineHandle Create() { return new T(); }
		static void Start(Nucleus::CoroutineHandle handle, const std::function<void()> &func) { reinterpret_cast<T *>(handle)->Start(func); }
		static void Stop(Nucleus::CoroutineHandle handle) { reinterpret_cast<T *>(handle)->Stop(); }
		static bool Suspend(Nucleus::CoroutineHandle handle) { return reinterpret_cast<T *>(handle)->Suspend(); }
		static void Resume(Nucleus::CoroutineHandle handle) { reinterpret_cast<T *>(handle)->Resume(); }
		static bool IsDone(Nucleus::CoroutineHandle handle) { return reinterpret_cast<T *>(handle)->IsDone(); }
		static void SetPromisePtr(Nucleus::CoroutineHandle handle, void *promisePtr) { reinterpret_cast<T *>(handle)->SetPromisePtr(promisePtr); }
		static void *GetPromisePtr(Nucleus::CoroutineHandle handle) { return reinterpret_cast<T *>(handle)->GetPromisePtr(); }
	};

	CoroutineRuntime out;
	out.Create = &Binding::Create;
	out.Start = &Binding::Start;
	out.Stop = &Binding::Stop;
	out.Suspend = &Binding::Suspend;
	out.Resume = &Binding::Resume;
	out.IsDone = &Binding::IsDone;
	out.SetPromisePtr = &Binding::SetPromisePtr;
	out.GetPromisePtr = &Binding::GetPromisePtr;
	return out;
}

CoroutineRuntime const *active = nullptr;  // guarded by activeMutex
std::mutex activeMutex;

////////////////////////////////////////////////////////////////////////////////
// Marl
////////////////////////////////////////////////////////////////////////////////
#if ENABLE_MARL_COROUTINES
struct Marl
{
	marl::Scheduler &getOrCreateScheduler()
	{
		static auto scheduler = [] {
			auto s = std::make_unique<marl::Scheduler>();
			s->setWorkerThreadCount(8);
			return s;
		}();
		return *scheduler;
	}

	Marl()
	    : useInternalScheduler(marl::Scheduler::get() == nullptr)
	{
		if(useInternalScheduler)
		{
			getOrCreateScheduler().bind();
		}
	}

	~Marl()
	{
		if(useInternalScheduler)
		{
			getOrCreateScheduler().unbind();
		}
	}

	void Start(const std::function<void()> &func)
	{
		auto run = [=] {
			ASSERT(!routineFiber);
			routineFiber = marl::Scheduler::Fiber::current();

			func();

			std::unique_lock<std::mutex> lock(mutex);
			ASSERT(inRoutine);
			done = true;        // coroutine is done.
			terminated = true;  // signal that the coroutine data is ready for freeing.
			inRoutine = false;
			mainFiber->notify();
		};

		ASSERT(!mainFiber);
		mainFiber = marl::Scheduler::Fiber::current();

		// block until the first yield or coroutine end
		std::unique_lock<std::mutex> lock(mutex);
		ASSERT(!inRoutine);
		inRoutine = true;
		marl::schedule(marl::Task(run, marl::Task::Flags::SameThread));
		mainFiber->wait(lock, [this] { return !inRoutine; });
	}

	void Stop()
	{
		ASSERT(marl::Scheduler::Fiber::current() == mainFiber);

		{
			std::unique_lock<std::mutex> lock(mutex);
			ASSERT(!inRoutine);
			done = true;
			inRoutine = true;
			routineFiber->notify();
			mainFiber->wait(lock, [this] { return terminated; });
		}
		delete this;
	}

	bool Suspend()
	{
		ASSERT(marl::Scheduler::Fiber::current() == routineFiber);

		std::unique_lock<std::mutex> lock(mutex);
		ASSERT(inRoutine);
		inRoutine = false;
		mainFiber->notify();
		routineFiber->wait(lock, [this] { return inRoutine; });
		return !done;
	}

	void Resume()
	{
		ASSERT(marl::Scheduler::Fiber::current() == mainFiber);

		std::unique_lock<std::mutex> lock(mutex);
		ASSERT(!inRoutine);
		inRoutine = true;
		routineFiber->notify();
		mainFiber->wait(lock, [this] { return !inRoutine; });
	}

	bool IsDone()
	{
		std::unique_lock<std::mutex> lock(mutex);
		return done;
	}
	void SetPromisePtr(void *ptr) { promisePtr = ptr; }
	void *GetPromisePtr() { return promisePtr; }

private:
	const bool useInternalScheduler;
	bool done = false;        // the coroutine should stop at the next yield()
	bool terminated = false;  // the coroutine has finished.
	bool inRoutine = false;   // is the coroutine currently executing?
	marl::Scheduler::Fiber *mainFiber = nullptr;
	marl::Scheduler::Fiber *routineFiber = nullptr;
	std::mutex mutex;
	void *promisePtr = nullptr;
};
static const CoroutineRuntime MarlRuntime = bind<Marl>();
#endif  // ENABLE_MARL_COROUTINES

////////////////////////////////////////////////////////////////////////////////
// ucontext
////////////////////////////////////////////////////////////////////////////////
#if ENABLE_UCONTEXT_COROUTINES
struct UContext
{
	static constexpr const size_t StackSize = 1 << 20;

	UContext()
	    : stack(StackSize)
	{
	}

	~UContext()
	{
	}

	void Start(const std::function<void()> &func)
	{
		struct Target
		{
			union Args
			{
				UContext *self;
				struct
				{
					int a;
					int b;
				};
			};

			static void Main(int a, int b)
			{
				Args u;
				u.a = a;
				u.b = b;
				std::function<void()> target;
				std::swap(target, u.self->target);
				target();
				u.self->done = true;
			}
		};

		auto res = getcontext(&coroutine);
		(void)res;
		ASSERT_MSG(res == 0, "getcontext() returned %d", int(res));

		coroutine.uc_stack.ss_sp = stack.data();
		coroutine.uc_stack.ss_size = stack.size();
		coroutine.uc_link = &caller;

		ASSERT_MSG((reinterpret_cast<uintptr_t>(coroutine.uc_stack.ss_sp) & 15) == 0, "Stack unaligned");

		target = func;

		Target::Args args;
		args.self = this;
		makecontext(&coroutine, reinterpret_cast<void (*)()>(Target::Main), 2, args.a, args.b);

		Resume();  // caller -> coroutine
	}

	void Stop()
	{
		done = true;
		Resume();
		delete this;
	}

	bool Suspend()
	{
		swapcontext(&coroutine, &caller);
		return !done;
	}

	void Resume()
	{
		if(!done)
		{
			swapcontext(&caller, &coroutine);
		}
	}

	bool IsDone() { return done; }
	void SetPromisePtr(void *ptr) { promisePtr = ptr; }
	void *GetPromisePtr() { return promisePtr; }

private:
	std::vector<uint8_t> stack;
	ucontext_t caller = {};
	ucontext_t coroutine = {};
	std::function<void()> target;
	bool done = false;
	void *promisePtr = nullptr;
};

static const CoroutineRuntime UContextRuntime = bind<UContext>();
#endif  // ENABLE_UCONTEXT_COROUTINES

////////////////////////////////////////////////////////////////////////////////
// Win32
////////////////////////////////////////////////////////////////////////////////
#if ENABLE_WIN32_COROUTINES
struct Win32
{
	static constexpr const size_t StackSize = 1 << 20;

	Win32()
	{
		mainFiber = ::ConvertThreadToFiber(nullptr);

		if(mainFiber)
		{
			convertedFiber = true;
		}
		else
		{
			// We're probably already on a fiber, so just grab it and remember that we didn't
			// convert it, so not to convert back to thread.
			mainFiber = GetCurrentFiber();
			convertedFiber = false;
		}
		ASSERT(mainFiber);
	}

	~Win32()
	{
		::DeleteFiber(routineFiber);
		if(convertedFiber)
		{
			::ConvertFiberToThread();
			mainFiber = nullptr;
		}
	}

	void Start(const std::function<void()> &func)
	{
		struct Invoker
		{
			std::function<void()> func;

			static VOID __stdcall fiberEntry(LPVOID lpParameter)
			{
				auto self = reinterpret_cast<Win32 *>(lpParameter);
				self->target();
				self->done = true;
				::SwitchToFiber(self->mainFiber);
			}
		};

		ASSERT(!routineFiber);
		routineFiber = ::CreateFiber(StackSize, &Invoker::fiberEntry, this);
		ASSERT(routineFiber);
		target = func;
		::SwitchToFiber(routineFiber);
	}

	void Stop()
	{
		done = true;
		Resume();
		delete this;
	}

	bool Suspend()
	{
		ASSERT(mainFiber);
		::SwitchToFiber(mainFiber);
		return !done;
	}

	void Resume()
	{
		ASSERT(routineFiber);
		if(!done)
		{
			::SwitchToFiber(routineFiber);
		}
	}

	bool IsDone() { return done; }
	void SetPromisePtr(void *ptr) { promisePtr = ptr; }
	void *GetPromisePtr() { return promisePtr; }

private:
	using FiberHandle = void *;
	std::function<void()> target;
	FiberHandle mainFiber = nullptr;
	FiberHandle routineFiber = nullptr;
	bool convertedFiber = false;
	bool done = false;
	void *promisePtr = nullptr;
};

static const CoroutineRuntime Win32Runtime = bind<Win32>();
#endif  // ENABLE_WIN32_COROUTINES

}  // anonymous namespace

#if ENABLE_MARL_COROUTINES
CoroutineRuntime const *const CoroutineRuntime::Marl = &MarlRuntime;
#else   // ENABLE_MARL_COROUTINES
CoroutineRuntime const *const CoroutineRuntime::Marl = nullptr;
#endif  // ENABLE_MARL_COROUTINES

#if ENABLE_UCONTEXT_COROUTINES
CoroutineRuntime const *const CoroutineRuntime::UContext = &UContextRuntime;
#else   // ENABLE_UCONTEXT_COROUTINES
CoroutineRuntime const *const CoroutineRuntime::UContext = nullptr;
#endif  // ENABLE_UCONTEXT_COROUTINES

#if ENABLE_WIN32_COROUTINES
CoroutineRuntime const *const CoroutineRuntime::Win32 = &Win32Runtime;
#else   // ENABLE_WIN32_COROUTINES
CoroutineRuntime const *const CoroutineRuntime::Win32 = nullptr;
#endif  // ENABLE_WIN32_COROUTINES

CoroutineRuntime const *CoroutineRuntime::Get()
{
	std::unique_lock<std::mutex> lock(activeMutex);
	return active;
}

void CoroutineRuntime::Set(CoroutineRuntime const *runtime)
{
	std::unique_lock<std::mutex> lock(activeMutex);
	active = runtime;
}

}  // namespace rr