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
#include "VkQueue.hpp"

namespace vk
{

Device::Device(VkPhysicalDevice pPhysicalDevice, uint32_t pQueueCount, VkQueue* pQueues)
	: physicalDevice(pPhysicalDevice), queueCount(pQueueCount), queues(pQueues)
{
}

void Device::destroy(const VkAllocationCallbacks* pAllocator)
{
	DestroyQueues(pAllocator, queueCount, queues);
}

VkResult Device::AllocateQueues(const VkAllocationCallbacks* pAllocator, const VkDeviceCreateInfo* pCreateInfo,
							    uint32_t& queueCount, VkQueue** queues)
{
	for(uint32_t i = 0; i < pCreateInfo->queueCreateInfoCount; i++)
	{
		queueCount += pCreateInfo->pQueueCreateInfos[i].queueCount;
	}

	*queues = reinterpret_cast<VkQueue*>(vk::allocate(sizeof(Queue*) * queueCount, REQUIRED_MEMORY_ALIGNMENT, pAllocator));

	if(*queues)
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
					(*queues)[queueID] = *queue;
				}
				else // OOM
				{
					DestroyQueues(pAllocator, queueID, *queues);
					return VK_ERROR_OUT_OF_HOST_MEMORY;
				}
			}
		}
	}
	else
	{
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	return VK_SUCCESS;
}

void Device::DestroyQueues(const VkAllocationCallbacks* pAllocator, const uint32_t& queuesToDestroy, VkQueue* queues)
{
	// Release queues
	for(uint32_t q = 0; q < queuesToDestroy; ++q)
	{
		vk::destroy(queues[q], pAllocator);
	}
	// Release queue array
	vk::deallocate(queues, pAllocator);
}

VkQueue Device::getQueue(uint32_t queueFamilyIndex, uint32_t queueIndex) const
{
	return queues[queueIndex];
}

} // namespace vk
