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

	uint32_t queueID = 0;
	for(uint32_t i = 0; i < pCreateInfo->queueCreateInfoCount; i++)
	{
		const VkDeviceQueueCreateInfo& queueCreateInfo = pCreateInfo->pQueueCreateInfos[i];

		for(uint32_t j = 0; j < queueCreateInfo.queueCount; j++, queueID++)
		{
			queues[queueID] = vk::Queue::newDispatchable(pAllocator,
				queueCreateInfo.queueFamilyIndex, queueCreateInfo.pQueuePriorities[j]);
		}
	}

	// Keep this->queueCount to 0 until all queues are constructed
	this->queueCount = queueCount;
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

VkQueue Device::getQueue(uint32_t queueFamilyIndex, uint32_t queueIndex) const
{
	return queues[queueIndex];
}

} // namespace vk