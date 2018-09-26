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

#include "VkObject.hpp"

namespace vk
{

class Device : public VkObject<Device, VkDevice>
{
public:
	static constexpr VkSystemAllocationScope GetAllocationScope() { return VK_SYSTEM_ALLOCATION_SCOPE_DEVICE; }

	~Device() = delete;
	void destroy(const VkAllocationCallbacks* pAllocator);

	VkQueue getQueue(uint32_t queueFamilyIndex, uint32_t queueIndex) const;

private:
	// Dispatchable objects have private constructors
	Device(const VkAllocationCallbacks* pAllocator, VkPhysicalDevice pPhysicalDevice,
	       const VkDeviceCreateInfo* pCreateInfo);
	// Only the base class may instantiate this object
	friend class VkObject<Device, VkDevice>;

	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkQueue* queues = nullptr;
	uint32_t queueCount = 0;
};

static Device* Cast(VkDevice device)
{
	return reinterpret_cast<Device::DispatchableType*>(device)->get();
}

} // namespace vk

#endif // VK_DEVICE_HPP_
