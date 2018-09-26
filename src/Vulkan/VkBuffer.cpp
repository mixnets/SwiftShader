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

#include "VkBuffer.hpp"
#include "VkConfig.h"
#include <memory.h>

namespace vk
{

Buffer::Buffer(const VkBufferCreateInfo* pCreateInfo, char* membersMemory)
	: flags(pCreateInfo->flags), size(pCreateInfo->size), usage(pCreateInfo->usage),
	sharingMode(pCreateInfo->sharingMode), queueFamilyIndexCount(pCreateInfo->queueFamilyIndexCount),
	queueFamilyIndices(reinterpret_cast<uint32_t*>(membersMemory))
{
	memcpy(queueFamilyIndices, pCreateInfo->pQueueFamilyIndices, ComputeRequiredAllocationSize(pCreateInfo));
}

void Buffer::destroy(const VkAllocationCallbacks* pAllocator)
{
	vk::deallocate(queueFamilyIndices, pAllocator);
}

size_t Buffer::ComputeRequiredAllocationSize(const VkBufferCreateInfo* pCreateInfo)
{
	return sizeof(uint32_t) * pCreateInfo->queueFamilyIndexCount;
}

const VkMemoryRequirements Buffer::getMemoryRequirements() const
{
	VkMemoryRequirements memoryRequirements = {};
	memoryRequirements.alignment = vk::REQUIRED_MEMORY_ALIGNMENT;
	memoryRequirements.memoryTypeBits = vk::REQUIRED_MEMORY_TYPE_BITS;
	memoryRequirements.size = size; // Maybe also reserve space for a header containing
		                            // the size of the buffer (for robust buffer access)
	return memoryRequirements;
}

void Buffer::bind(VkDeviceMemory pDeviceMemory, VkDeviceSize pMemoryOffset)
{
	deviceMemory = pDeviceMemory;
	memoryOffset = pMemoryOffset;
}

VkDeviceMemory Buffer::getDeviceMemory() const
{
	return deviceMemory;
}

} // namespace vk