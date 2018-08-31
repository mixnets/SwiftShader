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

#include "VkDevice.hpp"
#include "VkConfig.h"
#include "VkImage.hpp"
#include "VkPhysicalDevice.hpp"
#include "VkQueue.hpp"

namespace vk
{

Device::Device(const VkAllocationCallbacks* pAllocator, VkPhysicalDevice pPhysicalDevice,
               const VkDeviceCreateInfo* pCreateInfo)
	: physicalDevice(pPhysicalDevice)
{
	uint32_t queueCount = 0;
	for(uint32_t i = 0; i < pCreateInfo->queueCreateInfoCount; i++)
	{
		queueCount += pCreateInfo->pQueueCreateInfos[i].queueCount;
	}

	queues = reinterpret_cast<VkQueue*>(vk::allocate(sizeof(Queue*) * queueCount, pAllocator));

	if(queues)
	{
		uint32_t queueID = 0;
		for(uint32_t i = 0; i < pCreateInfo->queueCreateInfoCount; i++)
		{
			const VkDeviceQueueCreateInfo& queueCreateInfo = pCreateInfo->pQueueCreateInfos[i];

			if(queueCreateInfo.flags)
			{
				UNIMPLEMENTED();
			}

			for(uint32_t j = 0; j < queueCreateInfo.queueCount; j++, queueID++)
			{
				vk::Queue* queue = new (pAllocator) vk::Queue(
					queueCreateInfo.queueFamilyIndex, queueCreateInfo.pQueuePriorities[j]);
				if(queue)
				{
					queues[queueID] = *queue;
				}
				else
				{
					queues[queueID] = VK_NULL_HANDLE;
				}
			}
		}

		// Keep this->queueCount to 0 until all queues are constructed
		this->queueCount = queueCount;
	}
}

void Device::destroy(const VkAllocationCallbacks* pAllocator)
{
	for(uint32_t i = 0; i < queueCount; i++)
	{
		vk::destroy(queues[i], pAllocator);
	}
	vk::deallocate(queues, pAllocator);
	queues = nullptr;
	queueCount = 0;
}

bool Device::validate() const
{
	for(uint32_t i = 0; i < queueCount; i++)
	{
		if(!queues[i] || !vk::Cast(queues[i])->validate())
		{
			return false;
		}
	}

	return !!queues;
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
