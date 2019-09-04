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

#include "VkSemaphore.hpp"

#include "VkConfig.h"

#if SWIFTSHADER_EXTERNAL_SEMAPHORE_LINUX_MEMFD
#include "VkSemaphoreExternalLinux.hpp"
#elif SWIFTSHADER_EXTERNAL_SEMAPHORE_ZIRCON_EVENT
#include "VkSemaphoreExternalFuchsia.hpp"
#else
#include "VkSemaphoreExternalNone.hpp"
#endif

#include "marl/conditionvariable.h"
#include "marl/scheduler.h"

#include <functional>
#include <memory>
#include <mutex>
#include <utility>

namespace vk
{

// Helper class to manage a single background thread that can be used
// to wait on an external semaphore several times. This avoids spawning/joining
// a new thread on each Impl::wait() invokation. Usage is the following:
//
//  1) Create instance passing a pointer to the External instance.
//
//  2) Call doWaitInBackground() passing a completion callback that
//     will be called from the background thread when the semaphore
//     is signalled.
//
//     The method returns immediately, but should not be called again until
//     either the completion callback or doQuit() are called. An abort will
//     happen otherwise.
//
//     IMPORTANT: The completion callback should not call any other method
//     from this class or a deadlock may occur.
//
//  3) Call doQuit() to force-quit the thread. This can be called several
//     times safely. Note that the thread itself is only joined / destroyed
//     in this class' destructor though.
//
//  4) Destroying the instance will always force-quit the thread if it is
//     in the waiting thread. This avoids lockups if destruction happens before
//     the appropriate semaphore wait was performed.
//
class Semaphore::BackgroundThread
{
public:
	using CompletionCallback = std::function<void(void)>;

	explicit BackgroundThread(Semaphore::External* ext)
			: external(ext) {}

	~BackgroundThread()
	{
		std::unique_lock<std::mutex> lock(mutex);
		if (thread.get())
		{
			doQuit();
			thread->join();
		}
	}

	void doWaitInBackground(CompletionCallback&& completion_callback)
	{
		std::unique_lock<std::mutex> lock(mutex);
		switch (state)
		{
		case UNINITIALIZED:
			thread.reset(new std::thread([this]{this->mainLoop();}));
			state = WAITING_FOR_COMMAND;
			break;
		case WAITING_FOR_COMMAND:
			break;
		case QUITTING:
			ABORT("Thread has already quit!");
		default:
			ABORT("Already waiting on the semaphore!");
		}
		onCompletion = std::move(completion_callback);
		state = START_WAITING_FOR_SEMAPHORE;
		condition.notify_one();
	}

	void doQuit()
	{
		std::unique_lock<std::mutex> lock(mutex);
		if (state == WAITING_FOR_SEMAPHORE)
		{
			// Unblock the thread after clearing the callback.
			onCompletion = nullptr;
			external->signal();
		}
		if (state != QUITTING)
		{
			state = QUITTING;
			condition.notify_one();
		}
	}

private:
	void mainLoop()
	{
		std::unique_lock<std::mutex> lock(mutex);
		while (state != QUITTING)
		{
			condition.wait(lock, [this]{ return this->state != WAITING_FOR_COMMAND; });
			if (state == START_WAITING_FOR_SEMAPHORE)
			{
				state = WAITING_FOR_SEMAPHORE;
				lock.unlock();

				external->wait();

				lock.lock();
				if (state != QUITTING)
				{
					onCompletion();
					state = WAITING_FOR_COMMAND;
				}
			}
		}
	}

	enum State
	{
		UNINITIALIZED,
		WAITING_FOR_COMMAND,
		START_WAITING_FOR_SEMAPHORE,
		WAITING_FOR_SEMAPHORE,
		QUITTING,
	};

	Semaphore::External* const external;
	std::mutex mutex;
	std::condition_variable condition;
	State state = UNINITIALIZED;
	CompletionCallback onCompletion;
	std::unique_ptr<std::thread> thread;
};

// An implementation of VkSemaphore based on Marl primitives.
class Semaphore::Impl
{
public:
	// Create a new instance. The external instance will be allocated only
	// the |pCreateInfo->pNext| chain indicates it needs to be exported.
	Impl(const VkSemaphoreCreateInfo* pCreateInfo) {
		bool exportSemaphore = false;
		for (const auto* nextInfo = reinterpret_cast<const VkBaseInStructure*>(pCreateInfo->pNext);
			 nextInfo != nullptr; nextInfo = nextInfo->pNext)
		{
			if (nextInfo->sType == VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO)
			{
				const auto* exportInfo = reinterpret_cast<const VkExportSemaphoreCreateInfo *>(nextInfo);
				if (exportInfo->handleTypes != External::kExternalSemaphoreHandleType)
				{
					UNIMPLEMENTED("exportInfo->handleTypes");
				}
				exportSemaphore = true;
				break;
			}
		}

		if (exportSemaphore)
		{
			allocateExternalNoInit();
			external->init();
		}
	}

	~Impl() {
		deallocateExternal();
	}

	// Deallocate the External semaphore and its background thread, if any.
	void deallocateExternal()
	{
		if (external)
		{
			// NOTE: Destructor force-quits the thread if it's in the waiting state.
			backgroundThread->~BackgroundThread();
			backgroundThread = nullptr;

			external->~External();
			external = nullptr;
		}
	}

	// Allocate the external semaphore and background thread.
	// Note that this does not allocate the internal resource, which must be
	// performed by calling |external->init()|, or importing one using
	// a platform-specific |external->importXXX(...)| method.
	void allocateExternalNoInit()
	{
			external = new (externalStorage) External();
			backgroundThread = new (backgroundThreadStorage) BackgroundThread(external);
	}

	void wait()
	{
		if (external)
		{
			// Waiting for an external semaphore is a blocking operation
			// and should not be performed in the current thread when running
			// inside a fiber. Use a background thread and the marl
			// semaphore to wait for its completion.
			if (marl::Scheduler::get() != nullptr)
			{
				backgroundThread->doWaitInBackground(
						[this]{this->signalInternal();});
				waitInternal();
			}
			else
			{
				// Not inside a fiber, just wait directly then.
				external->wait();
			}
		}
		else
		{
			waitInternal();
		}
	}

	void signal()
	{
		if (external)
		{
			// Assumes that signalling an external semaphore is non-blocking,
			// so this can be performed directly either from a fiber or thread.
			external->signal();
		}
		else
		{
			signalInternal();
		}
	}

	// Wait on the marl condition variable only.
	void waitInternal()
	{
		std::unique_lock<std::mutex> lock(mutex);
		condition.wait(lock, [this]{ return this->signaled; });
		signaled = false;  // Vulkan requires resetting after waiting.
	}

	// Signal the marl condition variable only.
	void signalInternal()
        {
		std::unique_lock<std::mutex> lock(mutex);
		if (!signaled)
		{
			signaled = true;
			condition.notify_one();
		}
	}

private:
	// Necessary to make ::importXXX() and ::exportXXX() simpler.
	friend Semaphore;

	// Implementation of a non-external semaphore based on Marl.
	std::mutex mutex;
	marl::ConditionVariable condition;
	bool signaled = false;

	// Optional external semaphore data might be referenced and stored here.
	External* external = nullptr;
	Semaphore::BackgroundThread* backgroundThread = nullptr;

	alignas(External) char externalStorage[sizeof(External)];
	alignas(Semaphore::BackgroundThread) char
			backgroundThreadStorage[sizeof(Semaphore::BackgroundThread)];
};

Semaphore::Semaphore(const VkSemaphoreCreateInfo* pCreateInfo, void* mem)
{
	impl = new (mem) Impl(pCreateInfo);
}

void Semaphore::destroy(const VkAllocationCallbacks* pAllocator)
{
	impl->~Impl();
	vk::deallocate(impl, pAllocator);
}

size_t Semaphore::ComputeRequiredAllocationSize(const VkSemaphoreCreateInfo* pCreateInfo)
{
	return sizeof(Semaphore::Impl);
}

void Semaphore::wait()
{
	impl->wait();
}

void Semaphore::signal()
{
	impl->signal();
}

#if SWIFTSHADER_EXTERNAL_SEMAPHORE_LINUX_MEMFD
VkResult Semaphore::importFd(int fd)
{
	std::unique_lock<std::mutex> lock(impl->mutex);
	if (!impl->external)
	{
		impl->allocateExternalNoInit();
	}
	impl->external->importFd(fd);
	return VK_SUCCESS;
}

int Semaphore::exportFd() const
{
	std::unique_lock<std::mutex> lock(impl->mutex);
	if (!impl->external)
	{
		ABORT("Cannot export non-external semaphore");
	}
	return impl->external->exportFd();
}
#endif  // SWIFTSHADER_EXTERNAL_SEMAPHORE_LINUX_MEMFD

#if SWIFTSHADER_EXTERNAL_SEMAPHORE_ZIRCON_EVENT
VkResult Semaphore::importHandle(zx_handle_t handle)
{
	std::unique_lock<std::mutex> lock(impl->mutex);
	if (!impl->external)
	{
		impl->allocateExternalNoInit();
	}
	impl->external->importHandle(handle);
	return VK_SUCCESS;
}

zx_handle_t Semaphore::exportHandle() const
{
	std::unique_lock<std::mutex> lock(impl->mutex);
	if (!impl->external)
	{
		ABORT("Cannot export non-external semaphore");
	}
	return impl->external->exportHandle();
}
#endif  // SWIFTSHADER_EXTERNAL_SEMAPHORE_ZIRCON_EVENT

}  // namespace vk
