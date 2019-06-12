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

#include "VkInstance.hpp"
#include "VkDestroy.h"

namespace vk
{

Instance::Instance(const VkInstanceCreateInfo* pCreateInfo, void* mem, VkPhysicalDevice physicalDevice)
	: physicalDevice(physicalDevice),
	  physicalDeviceCount(physicalDevice ? 1 : 0)
{
}

void Instance::destroy(const VkAllocationCallbacks* pAllocator)
{
	vk::destroy(physicalDevice, pAllocator);
}

uint32_t Instance::getPhysicalDeviceCount() const
{
	return physicalDeviceCount;
}

void Instance::getPhysicalDevices(VkPhysicalDevice* pPhysicalDevices) const
{
	pPhysicalDevices[0] = physicalDevice;
}

uint32_t Instance::getPhysicalDeviceGroupCount() const
{
	return physicalDeviceGroupCount;
}

void Instance::getPhysicalDeviceGroups(VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties) const
{
	pPhysicalDeviceGroupProperties->physicalDeviceCount = physicalDeviceCount;
	pPhysicalDeviceGroupProperties->physicalDevices[0] = physicalDevice;
	pPhysicalDeviceGroupProperties->subsetAllocation = VK_FALSE;
}

} // namespace vk
