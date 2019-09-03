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

#ifndef VK_SEMAPHORE_EXTERNAL_LINUX_H_
#define VK_SEMAPHORE_EXTERNAL_LINUX_H_

#include "System/Linux/MemFd.hpp"
#include "System/Memory.hpp"

#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <sys/mman.h>

namespace linux
{

// A process-shared semaphore implementation that can be stored in
// a process-shared memory region. It also includes a reference count to
// ensure it is only destroyed when the last reference to it is dropped.
class SharedSemaphore {
public:
	SharedSemaphore()
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

	~SharedSemaphore()
	{
		pthread_cond_destroy(&cond);
		pthread_mutex_destroy(&mutex);
	}

	// Increment reference count.
	void addRef()
	{
		pthread_mutex_lock(&mutex);
		ref_count++;
		pthread_mutex_unlock(&mutex);
	}

	// Decrement reference count and returns true iff it reaches 0.
	bool deref()
	{
		pthread_mutex_lock(&mutex);
		bool result = (--ref_count == 0);
		pthread_mutex_unlock(&mutex);
		return result;
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
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	int ref_count = 1;
	bool signaled = false;
};

}  // namespace linux

namespace vk
{

// An external semaphore implementation for Linux, that stores a SharedSemaphore
// instance in a memfd-backed shared memory region, which can be imported/exported
// as a single file descriptor, as required by Vulkan.
class Semaphore::External {
public:
	// The type of external semaphore handle types supported by this implementation.
	static const VkExternalSemaphoreHandleTypeFlags kExternalSemaphoreHandleType = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT;

	explicit External(int fd = -1)
	{
		// Allocate or import the region's file descriptor.
		const size_t size = sw::memoryPageSize();
		if (fd < 0)
		{
			// To be exportable, the PosixSemaphore must be stored in a shared
			// memory region.
			static int counter = 0;
			char name[40];
			snprintf(name, sizeof(name), "SwiftShader.Semaphore.%d", ++counter);
			if (!memfd.allocate(name, size))
			{
				ABORT("memfd.allocate() returned %s", strerror(errno));
			}
		}
		else
		{
			memfd.importFd(fd);
		}

		// Map the region into memory and point the semaphore to it.
		void* addr = ::mmap(nullptr, size, PROT_READ|PROT_WRITE, MAP_SHARED, memfd.fd, 0);
		if (addr == MAP_FAILED)
		{
			ABORT("mmap() failed: %s", strerror(errno));
		}
		semaphore = reinterpret_cast<linux::SharedSemaphore *>(addr);

		// If the region was created, initialize its semaphore, otherwise simply
		// increment its reference count.
		if (fd < 0)
		{
			new (semaphore) linux::SharedSemaphore();
		}
		else
		{
			semaphore->addRef();
		}
	}

	~External()
	{
		if (semaphore)
		{
			if (semaphore->deref())
			{
				semaphore->~SharedSemaphore();
			}
			::munmap(semaphore, sw::memoryPageSize());
			memfd.close();
			semaphore = nullptr;
		}
	}

	int exportFd() const
	{
		return memfd.exportFd();
	}

	void wait()
	{
		semaphore->wait();
	}

	void signal()
	{
		semaphore->signal();
	}

private:
	linux::MemFd memfd;
	linux::SharedSemaphore* semaphore = nullptr;
};

}  // namespace vk

#endif  // VK_SEMAPHORE_EXTERNAL_LINUX_H_
