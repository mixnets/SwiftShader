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

#ifndef VK_SEMAPHORE_IMPL_POSIX_H_
#define VK_SEMAPHORE_IMPL_POSIX_H_

#include "VkConfig.h"
#include "VkDebug.hpp"

// An implementation of vk::Semaphore for Linux, Android and OS X.

#include "System/Linux/MemFd.hpp"
#include "System/Memory.hpp"

#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <sys/mman.h>

namespace linux {

class PosixSemaphore {
public:
	PosixSemaphore(bool process_shared = false)
	{
		if (process_shared)
		{
			pthread_mutexattr_t mattr;
			pthread_mutexattr_init(&mattr);
			pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
			pthread_mutex_init(&mutex, &mattr);
			pthread_mutexattr_destroy(&mattr);

			pthread_condattr_t cattr;
			pthread_condattr_init(&cattr);
			pthread_condattr_setpshared(&cattr, PTHREAD_PROCESS_SHARED);
			pthread_cond_init(&cond, &cattr);
			pthread_condattr_destroy(&cattr);
		}
	}

	~PosixSemaphore()
	{
		pthread_cond_destroy(&cond);
		pthread_mutex_destroy(&mutex);
	}

	void wait()
	{
		pthread_mutex_lock(&mutex);
		while (!signaled)
		{
			pthread_cond_wait(&cond, &mutex);
		}
		// From Vulkan 1.1.119 spec, section 6.4.2:
		// Unlike fences or events, the act of waiting for a semaphore also
		// unsignals that semaphore.
		signaled = false;
		pthread_mutex_unlock(&mutex);
	}

	void signal()
	{
		pthread_mutex_lock(&mutex);
		signaled = true;
		pthread_cond_broadcast(&cond);
		pthread_mutex_unlock(&mutex);
	}

private:
	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
	bool signaled = false;
};

}  // namespace linux

namespace vk
{

// NOTE: For now the implementation only stores a single semaphore.
// but external semaphores will requires the PosixSemaphore to be stored in
// a shared memory region page instead.
class Semaphore::Impl {
public:
	Impl(const VkSemaphoreCreateInfo* pCreateInfo)
	{
		bool exportSemaphore = false;

		const auto* createInfo = reinterpret_cast<const VkBaseInStructure*>(pCreateInfo->pNext);
		while(createInfo)
		{
			switch(createInfo->sType)
			{
			case VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO:
				{
					const auto* exportInfo = reinterpret_cast<const VkExportSemaphoreCreateInfo *>(createInfo);
						if (exportInfo->handleTypes != VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT)
						{
							UNIMPLEMENTED("exportInfo->handleTypes");
						}
						exportSemaphore = true;
				}
				break;
			default:
				UNIMPLEMENTED("createInfo->sType");
				break;
			}

			createInfo = createInfo->pNext;
		}

		void* addr = nullptr;
		if (exportSemaphore)
		{
			// To be exportable, the PosixSemaphore must be stored in a shared
			// memory region.
			const size_t size = sw::memoryPageSize();
			static int counter = 0;
			char name[40];
			snprintf(name, sizeof(name), "SwiftShader.Semaphore.%d", ++counter);
			if (!memfd.allocate(name, size))
			{
				ABORT("memfd.allocate() returned %s", strerror(errno));
			}
			addr = mapSharedRegion(memfd.fd, size);
		}
		initMappedSemaphore(addr);
	}

	~Impl()
	{
		close();
	}

	void close()
	{
		if (semaphore)
		{
			semaphore->~PosixSemaphore();
			semaphore = nullptr;
		}

		if (memfd.isValid())
		{
			::munmap(semaphore, sw::memoryPageSize());
			memfd.close();
		}
	}

	void initMappedSemaphore(void* addr)
	{
		semaphore = reinterpret_cast<linux::PosixSemaphore *>(addr ? addr : storage);
		new (semaphore) linux::PosixSemaphore(semaphore);
	}

	void* mapSharedRegion(int fd, size_t size)
	{
		void* addr = ::mmap(nullptr, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
		if (addr == MAP_FAILED)
		{
			ABORT("mmap() failed: %s", strerror(errno));
		}
		return addr;
	}

	int exportFd()
	{
		if (!memfd.isValid())
		{
			ABORT("Cannot export non-external semaphore!");
		}
		return memfd.exportFd();
	}

	void importFd(int fd)
	{
		close();
		memfd.importFd(fd);
		void* addr = mapSharedRegion(memfd.fd, sw::memoryPageSize());
		initMappedSemaphore(addr);
	}

	void wait()
	{
		semaphore->wait();
	}

	void wait(const VkPipelineStageFlags& flags)
	{
		semaphore->wait();
	}

	void signal()
	{
		semaphore->signal();
	}
	
private:
	linux::MemFd memfd;
	linux::PosixSemaphore*  semaphore = nullptr;
	alignas (linux::PosixSemaphore) char storage[sizeof(linux::PosixSemaphore)];
};

int Semaphore::exportFd() const
{
	return impl->exportFd();
}

VkResult Semaphore::importFd(int fd)
{
	impl->importFd(fd);

	return VK_SUCCESS;
}

}  // namespace vk

#endif  // VK_SEMAPHORE_IMPL_POSIX_H_
