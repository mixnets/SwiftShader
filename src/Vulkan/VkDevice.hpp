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

#ifndef VK_DEVICE_HPP_
#define VK_DEVICE_HPP_

#include "VkPhysicalDevice.hpp"
#include "VkQueue.hpp"

namespace vk {

VkClass(Device) {
public:
	Device(const VkAllocationCallbacks* pAllocator, VkPhysicalDevice pPhysicalDevice,
	       uint32_t pQueueCreateInfoCount, const VkDeviceQueueCreateInfo* pQueueCreateInfos)
		: physicalDevice(Cast(pPhysicalDevice))
	{
		uint32_t queueCount = 0;
		for(uint32_t i = 0; i < pQueueCreateInfoCount; i++)
		{
			queueCount += pQueueCreateInfos[i].queueCount;
		}

		queues = reinterpret_cast<Queue**>(
			vk::allocate(sizeof(Queue*) * queueCount, pAllocator, Queue::getSystemAllocationScope()));

		uint32_t queueID = 0;
		for(uint32_t i = 0; i < pQueueCreateInfoCount; i++)
		{
			const VkDeviceQueueCreateInfo& queueCreateInfo = pQueueCreateInfos[i];

			for(uint32_t j = 0; j < queueCreateInfo.queueCount; j++, queueID++)
			{
				queues[queueID] = new (pAllocator) Queue(
					queueCreateInfo.queueFamilyIndex, queueCreateInfo.pQueuePriorities[j]);
			}
		}
	}

	void destroy(const VkAllocationCallbacks* pAllocator) override
	{
		for(uint32_t i = 0; i < queueCount; i++) {
			vk::destroy(Cast(queues[i]), pAllocator);
		}
		vk::deallocate(queues, pAllocator);
		queues = nullptr;
		queueCount = 0;
	}

	static VkSystemAllocationScope getSystemAllocationScope() { return VK_SYSTEM_ALLOCATION_SCOPE_DEVICE; }

	VkQueue getQueue(uint32_t queueFamilyIndex, uint32_t queueIndex)
	{
		return Cast(queues[queueIndex]);
	}

private:
	PhysicalDevice* physicalDevice;
	Queue** queues;
	uint32_t queueCount;
};

} // namespace vk

#endif // VK_DEVICE_HPP_
