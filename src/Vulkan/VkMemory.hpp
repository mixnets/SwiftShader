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

#ifndef VK_MEMORY_HPP_
#define VK_MEMORY_HPP_

#include "Vulkan/VulkanPlatform.hpp"

namespace vk {

void *allocateDeviceMemory(size_t bytes, size_t alignment);
void freeDeviceMemory(void *memory);

void *allocate(size_t bytes, size_t alignment);
void deallocate(void *memory);  ///////////////// rename to free

// To make it verbose which allocations do not have an allocation callback pointer.
// Note that even if a pointer is available, it can also be null itself.
constexpr VkAllocationCallbacks *NULL_ALLOCATION_CALLBACKS = nullptr;  ///////////////// indicates a bug "Objects that are allocated from pools do not specify their own allocator. When an implementation requires host memory for such an object, that memory is sourced from the object’s parent pool’s allocator."

void *allocate_(size_t bytes, size_t alignment, const VkAllocationCallbacks *pAllocator, VkSystemAllocationScope allocationScope);

template<typename T>
T *allocateObject(size_t bytes, const VkAllocationCallbacks *pAllocator)
{
	return static_cast<T *>(allocate(bytes, alignof(T), pAllocator, T::GetAllocationScope()));
}
///////////////// rename to free
void deallocate(void *memory, const VkAllocationCallbacks *pAllocator);

}  // namespace vk

#endif  // VK_MEMORY_HPP_
