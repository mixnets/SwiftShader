// Copyright 2018 The SwiftShader Authors. All Rights Reserved.
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

#ifndef VK_SEMAPHORE_HPP_
#define VK_SEMAPHORE_HPP_

#include "VkConfig.h"
#include "VkObject.hpp"

#if SWIFTSHADER_EXTERNAL_SEMAPHORE_TYPE == SWIFTSHADER_EXTERNAL_SEMAPHORE_MEMFD
#include "System/MemFdLinux.hpp"
#endif

namespace vk
{

class Semaphore : public Object<Semaphore, VkSemaphore>
{
public:
	Semaphore(const VkSemaphoreCreateInfo* pCreateInfo, void* mem);
	~Semaphore();

#if SWIFTSHADER_EXTERNAL_SEMAPHORE_TYPE == SWIFTSHADER_EXTERNAL_SEMAPHORE_MEMFD
	class MappedSemaphore;

	// Import an external semaphore handle. If |fd| is -1, the function will
	// allocate a new external semaphore instead of importing it.
	VkResult importSemaphoreFd(int fd);

	// Return a copy of the external semaphore's file descriptor.
	int exportSemaphoreFd() const;
#endif

	static size_t ComputeRequiredAllocationSize(const VkSemaphoreCreateInfo* pCreateInfo);

	void wait();
	void wait(const VkPipelineStageFlags& flag);
	void signal();

private:
#if SWIFTSHADER_EXTERNAL_SEMAPHORE_TYPE == SWIFTSHADER_EXTERNAL_SEMAPHORE_MEMFD
    MemFdLinux memfd;
	MappedSemaphore* semaphore = nullptr;
#endif
};

static inline Semaphore* Cast(VkSemaphore object)
{
	return Semaphore::Cast(object);
}

} // namespace vk

#endif // VK_SEMAPHORE_HPP_
