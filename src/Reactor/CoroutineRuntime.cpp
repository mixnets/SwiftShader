
#include "Coroutine.hpp"
#include "Debug.hpp"

#define ENABLE_MARL_COROUTINES 1
#define ENABLE_UCONTEXT_COROUTINES 1

#if ENABLE_MARL_COROUTINES
#	include "marl/scheduler.h"
#	include "marl/event.h"
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
		auto run = [&] {
			func();

			done.signal();        // coroutine is done.
			suspended.signal();   // resume any blocking await() call.
			terminated.signal();  // signal that the coroutine data is ready for freeing.
		};

		marl::schedule(marl::Task(run, marl::Task::Flags::SameThread));

		suspended.wait();  // block until the first yield or coroutine end
	}

	void Stop()
	{
		done.signal();      // signal that the coroutine should stop at next (or current) yield.
		resumed.signal();   // wake the coroutine if blocked on a yield.
		terminated.wait();  // wait for the coroutine to return.
		delete this;
	}

	bool Suspend()
	{
		suspended.signal();
		resumed.wait();
		return !done.test();
	}

	void Resume()
	{
		resumed.signal();
		suspended.wait();
	}

	bool IsDone() { return done.test(); }
	void SetPromisePtr(void *ptr) { promisePtr = ptr; }
	void *GetPromisePtr() { return promisePtr; }

private:
	const bool useInternalScheduler;
	marl::Event suspended;                                // the coroutine is suspended on a yield()
	marl::Event resumed;                                  // the caller is suspended on an await()
	marl::Event done{ marl::Event::Mode::Manual };        // the coroutine should stop at the next yield()
	marl::Event terminated{ marl::Event::Mode::Manual };  // the coroutine has finished.
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
				const std::function<void()> *target;
				struct
				{
					int a;
					int b;
				};
			};

			static void Main(int a, int b)
			{
				printf("Target::Main()\n");
				Args u;
				u.a = a;
				u.b = b;
				(*u.target)();
			}
		};

		auto res = getcontext(&coroutine);
		(void)res;
		ASSERT_MSG(res == 0, "getcontext() returned %d", int(res));

		coroutine.uc_stack.ss_sp = stack.data();
		coroutine.uc_stack.ss_size = stack.size();
		coroutine.uc_link = nullptr;

		Target::Args args;
		args.target = &func;
		makecontext(&coroutine, reinterpret_cast<void (*)()>(&Target::Main), 2, args.a, args.b);

		res = getcontext(&caller);
		(void)res;
		ASSERT_MSG(res == 0, "getcontext() returned %d", int(res));

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
		return done;
	}

	void Resume()
	{
		swapcontext(&caller, &coroutine);
	}

	bool IsDone() { return done; }
	void SetPromisePtr(void *ptr) { promisePtr = ptr; }
	void *GetPromisePtr() { return promisePtr; }

private:
	std::vector<uint8_t> stack;
	ucontext_t caller = {};
	ucontext_t coroutine = {};
	bool done = false;
	void *promisePtr = nullptr;
};

static const CoroutineRuntime UContextRuntime = bind<UContext>();
#endif  // ENABLE_UCONTEXT_COROUTINES

}  // anonymous namespace

#if ENABLE_MARL_COROUTINES
CoroutineRuntime const *const CoroutineRuntime::Marl = &MarlRuntime;
#else
CoroutineRuntime const *const CoroutineRuntime::Marl = nullptr;
#endif  // ENABLE_MARL_COROUTINES

#if ENABLE_UCONTEXT_COROUTINES
CoroutineRuntime const *const CoroutineRuntime::UContext = &UContextRuntime;
#else
CoroutineRuntime const *const CoroutineRuntime::UContext = nullptr;
#endif  // ENABLE_UCONTEXT_COROUTINES

CoroutineRuntime const *CoroutineRuntime::Get()
{
	std::unique_lock<std::mutex> lock(activeMutex);
	ASSERT_MSG(active != nullptr, "rr::CoroutineRuntime::Get() called without a runtime set");
	return active;
}

void CoroutineRuntime::Set(CoroutineRuntime const *runtime)
{
	std::unique_lock<std::mutex> lock(activeMutex);
	ASSERT_MSG(runtime != nullptr, "rr::CoroutineRuntime::Set() called with nullptr");
	active = runtime;
}

}  // namespace rr