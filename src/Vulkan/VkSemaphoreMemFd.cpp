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

#include "System/Memory.hpp"

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

namespace vk
{

// Implementation note:
//
// Each external semaphore is implemented as a one-page shared memory region,
// created through memfd_create(), that contains a small process-shared mutex
// and condition variable to guard against a single boolean |signaled| flag.
//
// Using a full 4 kiB page for this seems wasteful, but is the only way to
// satisfy Vulkan which requires the underlying file descriptor to be
// copyable with dup()/dup2()/dup3() and sent to other processes through a
// unix socket.

class Semaphore::MappedSemaphore
{
public:
	MappedSemaphore()
	{
		pthread_mutexattr_t mutex_attr;
		pthread_mutexattr_init(&mutex_attr);
		pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED);
		pthread_mutex_init(&mutex, &mutex_attr);
		pthread_mutexattr_destroy(&mutex_attr);

		pthread_condattr_t cond_attr;
		pthread_condattr_init(&cond_attr);
		pthread_condattr_setpshared(&cond_attr, PTHREAD_PROCESS_SHARED);
		pthread_cond_init(&cond, &cond_attr);
		pthread_condattr_destroy(&cond_attr);
	}

	~MappedSemaphore()
	{
		pthread_cond_destroy(&cond);
		pthread_mutex_destroy(&mutex);
	}

	void wait()
	{
		pthread_mutex_lock(&mutex);
		while (!signaled) {
			pthread_cond_wait(&cond, &mutex);
		}
		pthread_mutex_unlock(&mutex);
	}

	void signal()
	{
		pthread_mutex_lock(&mutex);
		signaled = true;
		pthread_cond_signal(&cond);
		pthread_mutex_unlock(&mutex);
	}

private:
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	bool signaled = false;
};

Semaphore::Semaphore(const VkSemaphoreCreateInfo* pCreateInfo, void* mem) {}

VkResult Semaphore::importSemaphoreFd(int fd) {
	if (fd < 0) {
		// Name is only used for debugging.
		static int counter = 0;
		char name[64];
		snprintf(name, sizeof(name), "SwiftShaderVulkanSemaphore%d", ++counter);
		if (!memfd.allocate(name, sw::memoryPageSize()))
		{
			return VK_ERROR_OUT_OF_DEVICE_MEMORY;
		}
	}
	void* addr = ::mmap(nullptr, sw::memoryPageSize(), PROT_READ|PROT_WRITE, MAP_SHARED,
						memfd.fd, 0);
	if (addr == MAP_FAILED)
	{
		WARN("mmap() returned MAP_FAILED: %s", strerror(errno));
		memfd.close();
		return VK_ERROR_OUT_OF_DEVICE_MEMORY;
	}
	semaphore = new (addr) MappedSemaphore();
	return VK_SUCCESS;
}

int Semaphore::exportSemaphoreFd() const {
	return memfd.exportFd();
}

Semaphore::~Semaphore()
{
	if (semaphore)
	{
		semaphore->~MappedSemaphore();
		::munmap(semaphore, sw::memoryPageSize());
		semaphore = nullptr;
	}
	memfd.close();
}

size_t Semaphore::ComputeRequiredAllocationSize(const VkSemaphoreCreateInfo* pCreateInfo)
{
	return 0;
}

void Semaphore::wait()
{
	if (semaphore)
	{
		semaphore->wait();
	}
}

void Semaphore::wait(const VkPipelineStageFlags& flags)
{
    // VkPipelineStageFlags is the pipeline stage at which the semaphore wait will occur
	wait();
}

void Semaphore::signal()
{
	if (semaphore)
	{
		semaphore->signal();
	}
}

}  // namespace vk
