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

#ifndef VK_BUFFER_HPP_
#define VK_BUFFER_HPP_

#include "VkConfig.h"
#include "VkObject.hpp"

namespace vk
{

VkClass(Buffer)
{
public:
	Buffer(const VkAllocationCallbacks* pAllocator, const VkBufferCreateInfo* pCreateInfo)
		: flags(pCreateInfo->flags), size(pCreateInfo->size), usage(pCreateInfo->usage),
		  sharingMode(pCreateInfo->sharingMode), queueFamilyIndexCount(pCreateInfo->queueFamilyIndexCount)
	{
		size_t queueFamilyIndicesSize = sizeof(uint32_t) * queueFamilyIndexCount;
		queueFamilyIndices = reinterpret_cast<uint32_t*>(vk::allocate(
			queueFamilyIndicesSize, pAllocator, GetAllocationScope(), pCreateInfo->pQueueFamilyIndices));
	}

	~Buffer() = delete;

	void destroy(const VkAllocationCallbacks* pAllocator) override
	{
		vk::deallocate(queueFamilyIndices, pAllocator);
		queueFamilyIndices = nullptr;
		queueFamilyIndexCount = 0;
	}

	const VkMemoryRequirements GetMemoryRequirements()
	{
		VkMemoryRequirements memoryRequirements = {};
		memoryRequirements.alignment = vk::REQUIRED_MEMORY_ALIGNMENT;
		memoryRequirements.memoryTypeBits = vk::REQUIRED_MEMORY_TYPE_BITS;
		memoryRequirements.size = size; // Maybe also reserve space for a header containing
		                                // the size of the buffer (for robust buffer access)
		return memoryRequirements;
	}

	void bind(VkDeviceMemory pDeviceMemory, VkDeviceSize pMemoryOffset)
	{
		deviceMemory = pDeviceMemory;
		memoryOffset = pMemoryOffset;
	}

	VkDeviceMemory getDeviceMemory()
	{
		return deviceMemory;
	}

private:
	VkDeviceMemory        deviceMemory = nullptr;
	VkDeviceSize          memoryOffset = 0;
	VkBufferCreateFlags   flags = 0;
	VkDeviceSize          size = 0;
	VkBufferUsageFlags    usage = 0;
	VkSharingMode         sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	uint32_t              queueFamilyIndexCount = 0;
	uint32_t*             queueFamilyIndices = nullptr;
};

} // namespace vk

#endif // VK_BUFFER_HPP_