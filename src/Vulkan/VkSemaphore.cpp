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

#include "Yarn/ConditionVariable.hpp"
#include "Yarn/Scheduler.hpp"

#include <mutex>

namespace vk
{

class ExternalSemaphore;

// An implementation of VkSemaphore based on Yarn primitives.
class Semaphore::Impl
{
public:
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
			external = new (storage) External();
		}
	}

	~Impl() {
		if (external)
		{
			external->~External();
		}
	}

	void wait()
	{
		if (external)
		{
			// Waiting for an external semaphore is a blocking operation
			// and should not be performed in the current thread when running
			// inside a Yarn fiber. Use a background thread and the yarn
			// semaphore to wait for its completion.
			//
			// TODO(digit): Reuse a single std::thread instance instead of
			// creating / destroying one on every wait().
			if (yarn::Scheduler::get() != nullptr)
			{
				auto thread = std::thread([this]{
					external->wait();
					this->signalInternal();
				});
				waitInternal();
				thread.join();
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

	void waitInternal()
	{
		std::unique_lock<std::mutex> lock(mutex);
		condition.wait(lock, [this]{ return this->signaled; });
		signaled = false;  // Vulkan requires resetting after waiting.
	}

	void signalInternal()
        {
		std::unique_lock<std::mutex> lock(mutex);
		if (!signaled)
		{
			signaled = true;
			condition.notify_one();
		}
	}

	void signal()
        {
		if (external)
		{
			// Assumes that signalling an external semaphore is non-blocking.
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

	// Implementation of a non-external semaphore based on Yarn.
	std::mutex mutex;
	yarn::ConditionVariable condition;
	bool signaled = false;

	// Optional external semaphore might be referenced and stored here.
	External* external = nullptr;
	alignas(External) char storage[sizeof(External)];
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
	if (impl->external)
	{
		impl->external->~External();
	}
	impl->external = new (impl->storage) External(fd);
	return VK_SUCCESS;
}

int Semaphore::exportFd()
{
		if (!impl->external)
		{
			ABORT("Cannot export non-external semaphore");
		}
		return impl->external->exportFd();
}
#endif  // SWIFTSHADER_EXTERNAL_SEMAPHORE_LINUX_MEMFD

}  // namespace vk
