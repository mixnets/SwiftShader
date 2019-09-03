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

#include "Yarn/ConditionVariable.hpp"
#include <mutex>

namespace vk
{

// An implementation of VkSemaphore based on Yarn primitives.
class Semaphore::Impl {
public:
	Impl() = default;

	void wait() {
		std::unique_lock<std::mutex> lock(mutex);
		condition.wait(lock, [this]{ return this->signaled; });
		signaled = false;  // Vulkan requires resetting after waiting.
	}

	void signal() {
		std::unique_lock<std::mutex> lock(mutex);
		if (!signaled)
		{
			signaled = true;
			condition.notify_one();
		}
	}

private:
	std::mutex mutex;
	yarn::ConditionVariable condition;
	bool signaled = false;
};

Semaphore::Semaphore(const VkSemaphoreCreateInfo* pCreateInfo, void* mem) {
	impl = new (mem) Impl();
}

Semaphore::~Semaphore() {
	impl->~Impl();
}

size_t Semaphore::ComputeRequiredAllocationSize(const VkSemaphoreCreateInfo* pCreateInfo) {
	return sizeof(Impl);
}

void Semaphore::wait() {
	impl->wait();
}

void Semaphore::signal() {
	impl->signal();
}

}  // namespace vk
