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

#ifndef VK_QUEUE_HPP_
#define VK_QUEUE_HPP_

#include "VkDebug.hpp"
#include "VkMemory.h"
#include "vulkan/vk_icd.h"

namespace vk
{

class Queue final
{
	VK_LOADER_DATA loaderData = { ICD_LOADER_MAGIC };
public:
	Queue(uint32_t pFamilyIndex, float pPriority);
	~Queue() = delete;
	operator VkQueue();
	void* operator new(size_t count, const VkAllocationCallbacks* pAllocator);
	void operator delete(void* ptr, const VkAllocationCallbacks* pAllocator);
	void destroy(const VkAllocationCallbacks* pAllocator) {}

	void submit(uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence);
	void bindSparse(uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence);
	void waitIdle();

private:
	void signal(VkFence fence);

	uint32_t familyIndex = 0;
	float    priority = 0.0f;
};

static Queue* Cast(VkQueue queue) { return reinterpret_cast<Queue*>(queue); }

} // namespace vk

#endif // VK_QUEUE_HPP_
