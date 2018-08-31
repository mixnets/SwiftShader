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

VkClass(Instance)
{
public:
	Instance(const VkAllocationCallbacks* pAllocator) :
		physicalDeviceCount(1), physicalDevice(*(new (pAllocator) PhysicalDevice())) {}

	~Instance() = delete;

	void destroy(const VkAllocationCallbacks* pAllocator) override
	{
		vk::destroy(physicalDevice, pAllocator);
		physicalDeviceCount = 0;
	}

	static inline VkSystemAllocationScope GetAllocationScope() { return VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE; }

	uint32_t physicalDeviceCount = 0;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
};

} // namespace vk

#endif // VK_INSTANCE_HPP_