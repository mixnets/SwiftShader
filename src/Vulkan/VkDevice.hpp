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

#include "VkDebug.hpp"
#include "VkMemory.h"
#include "vulkan/vk_icd.h"

namespace vk
{

class Device final
{
	VK_LOADER_DATA loaderData = { ICD_LOADER_MAGIC };
public:
	Device(const VkAllocationCallbacks* pAllocator, VkPhysicalDevice pPhysicalDevice,
	       const VkDeviceCreateInfo* pCreateInfo);
	~Device() = delete;
	operator VkDevice();
	void* operator new(size_t count, const VkAllocationCallbacks* pAllocator);
	void operator delete(void* ptr, const VkAllocationCallbacks* pAllocator);
	void destroy(const VkAllocationCallbacks* pAllocator);

	VkQueue getQueue(uint32_t queueFamilyIndex, uint32_t queueIndex);
	void waitForFences(uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll, uint64_t timeout);
	void waitIdle();
	void getImageSparseMemoryRequirements(VkImage image, uint32_t* pSparseMemoryRequirementCount,
	                                      VkSparseImageMemoryRequirements* pSparseMemoryRequirements) const;
private:
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkQueue* queues = nullptr;
	uint32_t queueCount = 0;
};

static Device* Cast(VkDevice device) { return reinterpret_cast<Device*>(device); }

} // namespace vk

#endif // VK_DEVICE_HPP_
