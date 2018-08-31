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

#ifndef VK_OBJECT_HPP_
#define VK_OBJECT_HPP_

#include "Memory.hpp"
#include "VkConfig.h"
#include "VkMemory.h"
#include <memory.h>

namespace vk
{

void* allocate(size_t count, const VkAllocationCallbacks* pAllocator, VkSystemAllocationScope allocationScope, const void* src)
{
	// Note: Should be a template function using std::alignof() instead of using the following constant alignment
	static const size_t alignment = REQUIRED_MEMORY_ALIGNMENT;
	void* dst = pAllocator ?
		pAllocator->pfnAllocation(nullptr, count, alignment, allocationScope) :
		sw::allocate(count, alignment);
	if(src)
	{
		::memcpy(dst, src, count);
	}
	return dst;
}

void deallocate(void* ptr, const VkAllocationCallbacks* pAllocator)
{
	pAllocator ? pAllocator->pfnFree(nullptr, ptr) : sw::deallocate(ptr);
}

} // namespace vk

#endif // VK_OBJECT_HPP_