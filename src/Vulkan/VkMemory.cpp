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

#include "VkMemory.hpp"

#include "VkConfig.hpp"
#include "System/Memory.hpp"

namespace vk {

void *allocateDeviceMemory(size_t bytes, size_t alignment)
{
	return allocate_(bytes, alignment, nullptr, VK_SYSTEM_ALLOCATION_SCOPE_MAX_ENUM);
}

void freeDeviceMemory(void *memory)
{
	deallocate(memory, nullptr);
}

void *allocate(size_t bytes, size_t alignment)
{
	return allocate_(bytes, alignment, nullptr, VK_SYSTEM_ALLOCATION_SCOPE_MAX_ENUM);
}

void deallocate(void *memory)
{
	deallocate(memory, nullptr);
}

void *allocate_(size_t bytes, size_t alignment, const VkAllocationCallbacks *pAllocator, VkSystemAllocationScope allocationScope)
{
	// TODO(b/140991626): Use allocateUninitialized() instead of allocateZeroOrPoison() to improve startup peformance.
	return pAllocator ? pAllocator->pfnAllocation(pAllocator->pUserData, bytes, alignment, allocationScope)
	                  : sw::allocateZeroOrPoison(bytes, alignment);
}

void deallocate(void *memory, const VkAllocationCallbacks *pAllocator)
{
	pAllocator ? pAllocator->pfnFree(pAllocator->pUserData, memory) : sw::deallocate(memory);
}

}  // namespace vk

#endif  // VK_OBJECT_HPP_
