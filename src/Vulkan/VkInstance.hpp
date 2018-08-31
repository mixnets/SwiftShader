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

#include "VkDebug.hpp"
#include "VkMemory.h"
#include "vulkan/vk_icd.h"

namespace vk
{

class Instance final
{
	VK_LOADER_DATA loaderData = { ICD_LOADER_MAGIC };
public:
	Instance(const VkAllocationCallbacks* pAllocator);
	~Instance() = delete;;
	operator VkInstance();
	void* operator new(size_t count, const VkAllocationCallbacks* pAllocator);
	void operator delete(void* ptr, const VkAllocationCallbacks* pAllocator);
	void destroy(const VkAllocationCallbacks* pAllocator);

	uint32_t getPhysicalDeviceCount() const { return physicalDeviceCount; }
	VkPhysicalDevice getPhysicalDevice() const { return physicalDevice; }
	void enumeratePhysicalDeviceGroups(uint32_t* pPhysicalDeviceGroupCount,
                                       VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties) const;
private:
	uint32_t physicalDeviceCount = 0;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
};

static Instance* Cast(VkInstance instance) { return reinterpret_cast<Instance*>(instance); }

} // namespace vk

#endif // VK_INSTANCE_HPP_