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
#else
#include "VkSemaphoreExternalNone.hpp"
#endif

#include "marl/blockingcall.h"
#include "marl/conditionvariable.h"

#include <functional>
#include <memory>
#include <mutex>
#include <utility>

namespace vk
{

// An implementation of VkSemaphore based on Marl primitives.
class Semaphore::Impl
{
public:
	// Create a new instance. The external instance will be allocated only
	// the pCreateInfo->pNext chain indicates it needs to be exported.
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

	// Deallocate the External semaphore if any.
	void deallocateExternal()
	{
		if (external)
		{
			external->~External();
			external = nullptr;
		}
	}

	// Allocate the external semaphore.
	// Note that this does not allocate the internal resource, which must be
	// performed by calling external->init(), or importing one using
	// a platform-specific external->importXXX(...) method.
	void allocateExternalNoInit()
	{
		external = new (externalStorage) External();
	}

	void wait()
	{
		if (external)
		{
			if (!external->tryWait())
			{
				// Dispatch the external wait to a background thread, then let it
				// signal the internal semaphore to wait for it on the main
				// fiber.
				//
				// NOTE: This currently creates a new std::thread on each call,
				// and benchmarking shows that this is about 3 to 4 times slower
				// than actually using a dedicated background thread loop to
				// do each wait (e.g. on Linux, 24us vs 8us on Xeon E5-2690 @
				// 2.6 GHz).
				//
				// However, it is assumed here that this difference is negligible
				// compared with the actual semaphore wait() time that will occur.

				// NOTE: The following does not seem to compile and generate
				// template expansion errors at the moment, so replace it with
				// an explicit std::thread() creation + signaling the internal
				// semaphore instead (which is functionally equivalent):
				//
				// 		marl::blocking_call([this](){
				// 			external->wait();
				// 		});
				//
				auto thread = std::thread([this](){
					external->wait();
					signalInternal();
				});
				waitInternal();
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
			// so it can be performed directly either from a fiber or thread.
			external->signal();
		}
		else
		{
			signalInternal();
		}
	}

private:
	// Necessary to make ::importXXX() and ::exportXXX() simpler.
	friend Semaphore;

	void waitInternal()
	{
		// Wait on the marl condition variable only.
		std::unique_lock<std::mutex> lock(mutex);
		condition.wait(lock, [this]{ return this->signaled; });
		signaled = false;  // Vulkan requires resetting after waiting.
	}

	void signalInternal()
	{
		// Signal the marl condition variable only.
		std::unique_lock<std::mutex> lock(mutex);
		if (!signaled)
		{
			signaled = true;
			condition.notify_one();
		}
	}

	// Implementation of a non-external semaphore based on Marl.
	std::mutex mutex;
	marl::ConditionVariable condition;
	bool signaled = false;

	// Optional external semaphore data might be referenced and stored here.
	External* external = nullptr;

	alignas(External) char externalStorage[sizeof(External)];
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

VkResult Semaphore::exportFd(int* pFd) const
{
	std::unique_lock<std::mutex> lock(impl->mutex);
	if (!impl->external)
	{
		TRACE("Cannot export non-external semaphore");
		return VK_ERROR_INVALID_EXTERNAL_HANDLE;
	}
	return impl->external->exportFd(pFd);
}
#endif  // SWIFTSHADER_EXTERNAL_SEMAPHORE_LINUX_MEMFD

}  // namespace vk
