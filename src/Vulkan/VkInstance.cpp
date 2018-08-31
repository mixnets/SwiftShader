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
#include "VkPhysicalDevice.hpp"

namespace vk
{

Instance::Instance(const VkAllocationCallbacks* pAllocator) :
	physicalDeviceCount(1), physicalDevice(*(new (pAllocator) PhysicalDevice()))
{
}

Instance::operator VkInstance()
{
	return reinterpret_cast<VkInstance>(this);
}

void* Instance::operator new(size_t count, const VkAllocationCallbacks* pAllocator)
{
	return vk::allocate(count, pAllocator, VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
}

void Instance::operator delete(void* ptr, const VkAllocationCallbacks* pAllocator)
{
	// Does nothing, objects are deleted through the destroy function
	ASSERT(false);
}

void Instance::destroy(const VkAllocationCallbacks* pAllocator)
{
	vk::destroy(physicalDevice, pAllocator);
	physicalDeviceCount = 0;
}

void Instance::enumeratePhysicalDeviceGroups(uint32_t* pPhysicalDeviceGroupCount,
                                             VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties) const
{
	if(!pPhysicalDeviceGroupProperties)
	{
		*pPhysicalDeviceGroupCount = 1;
	}
	else
	{
		pPhysicalDeviceGroupProperties->physicalDeviceCount = physicalDeviceCount;
		pPhysicalDeviceGroupProperties->physicalDevices[0] = physicalDevice;
		pPhysicalDeviceGroupProperties->subsetAllocation = VK_FALSE;
	}
}

} // namespace vk