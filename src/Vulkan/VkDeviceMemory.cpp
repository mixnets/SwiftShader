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

#include "VkDeviceMemory.hpp"

namespace vk
{

DeviceMemory::DeviceMemory(const VkMemoryAllocateInfo* pCreateInfo, const Memory& mem) :
	size(pCreateInfo->allocationSize), buffer(mem.device)
{
	ASSERT(size && buffer);
}

void DeviceMemory::destroy(const VkAllocationCallbacks* pAllocator)
{
	vk::deallocate(buffer, DEVICE_MEMORY);
}

MemorySize DeviceMemory::ComputeRequiredAllocationSize(const VkMemoryAllocateInfo* pCreateInfo)
{
	// buffer is "GPU memory", so we use device memory for it
	return MemorySize(0, pCreateInfo->allocationSize);
}

void* DeviceMemory::map(VkDeviceSize offset, VkDeviceSize size)
{
	return reinterpret_cast<char*>(buffer) + offset;
}

void DeviceMemory::unmap()
{
}

void DeviceMemory::flush()
{
	// FIXME: Resource sync() goes here
}

void DeviceMemory::invalidate()
{
	// Possibly a noop for SwiftShader
}

VkDeviceSize DeviceMemory::getCommittedMemoryInBytes() const
{
	return size;
}

} // namespace vk