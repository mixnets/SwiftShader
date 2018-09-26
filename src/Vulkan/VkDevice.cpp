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

#include "VkConfig.h"
#include "VkDebug.hpp"
#include "VkDevice.hpp"
#include "VkConfig.h"
#include "VkImage.hpp"
#include "VkPhysicalDevice.hpp"
#include "VkQueue.hpp"

namespace vk
{

Device::Device(const Device::CreateInfo* info, const Memory& mem)
	: physicalDevice(info->pPhysicalDevice), queues(reinterpret_cast<VkQueue*>(mem.host))
{
	const auto* pCreateInfo = info->pCreateInfo;
	for(uint32_t i = 0; i < pCreateInfo->queueCreateInfoCount; i++)
	{
		const VkDeviceQueueCreateInfo& queueCreateInfo = pCreateInfo->pQueueCreateInfos[i];
		queueCount += info->pCreateInfo->pQueueCreateInfos[i].queueCount;
	}

	char* hostMemory = mem.host + sizeof(Queue*) * queueCount;

	uint32_t queueID = 0;
	for(uint32_t i = 0; i < pCreateInfo->queueCreateInfoCount; i++)
	{
		const VkDeviceQueueCreateInfo& queueCreateInfo = pCreateInfo->pQueueCreateInfos[i];

		for(uint32_t j = 0; j < queueCreateInfo.queueCount; j++, queueID++, hostMemory += sizeof(Queue))
		{
			queues[queueID] = reinterpret_cast<VkQueue>(hostMemory);
			vk::Cast(queues[queueID])->init(queueCreateInfo.queueFamilyIndex, queueCreateInfo.pQueuePriorities[j]);
		}
	}
}

void Device::destroy(const VkAllocationCallbacks* pAllocator)
{
	vk::deallocate(queues, pAllocator); // All individual queue objects are in the same allocation
}

MemorySize Device::ComputeRequiredAllocationSize(const Device::CreateInfo* info)
{
	uint32_t queueCount = 0;
	for(uint32_t i = 0; i < info->pCreateInfo->queueCreateInfoCount; i++)
	{
		queueCount += info->pCreateInfo->pQueueCreateInfos[i].queueCount;
	}

	return MemorySize((sizeof(Queue*) + sizeof(Queue)) * queueCount, 0);
}

VkQueue Device::getQueue(uint32_t queueFamilyIndex, uint32_t queueIndex) const
{
	return queues[queueIndex];
}

void Device::waitForFences(uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll, uint64_t timeout)
{
	// noop
}

void Device::waitIdle()
{
	for(uint32_t i = 0; i < queueCount; i++)
	{
		vk::Cast(queues[i])->waitIdle();
	}
}

void Device::getImageSparseMemoryRequirements(VkImage pImage, uint32_t* pSparseMemoryRequirementCount,
	                                          VkSparseImageMemoryRequirements* pSparseMemoryRequirements) const
{
	if(!pSparseMemoryRequirements)
	{
		*pSparseMemoryRequirementCount = 1;
	}
	else
	{
		auto image = vk::Cast(pImage);
		uint32_t propertyCount = 1;
		vk::Cast(physicalDevice)->getSparseImageFormatProperties(
			image->getFormat(), image->getImageType(), image->getSamples(), image->getUsage(),
			image->getTiling(), &propertyCount, &(pSparseMemoryRequirements->formatProperties));
		image->getImageMipTailInfo(pSparseMemoryRequirements);
	}
}

} // namespace vk
