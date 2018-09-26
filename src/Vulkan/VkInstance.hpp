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

#ifndef VK_INSTANCE_HPP_
#define VK_INSTANCE_HPP_

#include "VkPhysicalDevice.hpp"

namespace vk
{

class Instance : public VkDispatchableObject<Instance, VkInstance>
{
public:
	static constexpr VkSystemAllocationScope GetAllocationScope() { return VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE; }

	Instance(PhysicalDevice* pPhysicalDevice);
	~Instance() = delete;
	void destroy(const VkAllocationCallbacks* pAllocator);

	uint32_t getPhysicalDeviceCount() const;
	void getPhysicalDevices(uint32_t pPhysicalDeviceCount, VkPhysicalDevice* pPhysicalDevices) const;
	uint32_t getPhysicalDeviceGroupCount() const;
	void getPhysicalDeviceGroups(uint32_t pPhysicalDeviceGroupCount,
                                 VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties) const;
private:
	uint32_t physicalDeviceCount = 0;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	uint32_t physicalDeviceGroupCount = 1;
};

static inline Instance* Cast(VkInstance object)
{
	return Instance::Cast(object);
}

} // namespace vk

#endif // VK_INSTANCE_HPP_
