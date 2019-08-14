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

// An implementation of vk::Semaphore for Linux, Android and OS X.

#include <pthread.h>

namespace vk
{

namespace posix {

class PosixSemaphore {
public:
	PosixSemaphore() = default;

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

}  // namespace posix

// External posix semaphores require a shared memory region that can be shared
// across processes. For Linux, see VkSemaphoreImplLinux.h which uses a memfd
// backed region. This header is a fallback for other systems.
class Semaphore::Impl {
public:
	Impl(const VkSemaphoreCreateInfo* pCreateInfo)
	{
		if (pCreateInfo->pNext)
		{
			UNIMPLEMENTED("pCreateInfo->pNext");
		}
	}

	void wait()
	{
		semaphore.wait();
	}

	void wait(const VkPipelineStageFlags& flags)
	{
		semaphore.wait();
	}

	void signal()
	{
		semaphore.signal();
	}

private:
	posix::PosixSemaphore semaphore;
};

}  // namespace vk

#endif  // VK_SEMAPHORE_IMPL_POSIX_H_
