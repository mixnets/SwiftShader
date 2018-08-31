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

#ifndef VK_PHYSICAL_DEVICE_HPP_
#define VK_PHYSICAL_DEVICE_HPP_

#include "VkDebug.hpp"
#include "VkMemory.h"
#include "vulkan/vk_icd.h"

namespace vk
{

class PhysicalDevice final
{
	VK_LOADER_DATA loaderData = { ICD_LOADER_MAGIC };
public:
	PhysicalDevice();
	~PhysicalDevice() = delete;
	operator VkPhysicalDevice();
	void* operator new(size_t count, const VkAllocationCallbacks* pAllocator);
	void operator delete(void* ptr, const VkAllocationCallbacks* pAllocator);
	void destroy(const VkAllocationCallbacks* pAllocator);

	const VkPhysicalDeviceFeatures& getFeatures() const;
	bool hasFeatures(const VkPhysicalDeviceFeatures& requestedFeatures) const;
	const VkPhysicalDeviceProperties& getProperties() const;
	void getFormatProperties(VkFormat format, VkFormatProperties* pFormatProperties) const;
	void getImageFormatProperties(VkFormat format, VkImageType type, VkImageTiling tiling,
	                              VkImageUsageFlags usage, VkImageCreateFlags flags,
	                              VkImageFormatProperties* pImageFormatProperties) const;
	uint32_t getQueueFamilyPropertyCount() const;
	void getQueueFamilyProperties(uint32_t pQueueFamilyPropertyCount,
	                              VkQueueFamilyProperties* pQueueFamilyProperties) const;
	const VkPhysicalDeviceMemoryProperties& getMemoryProperties() const;
	void getSparseImageFormatProperties(VkFormat format, VkImageType type,
		VkSampleCountFlagBits samples, VkImageUsageFlags usage, VkImageTiling tiling,
		uint32_t* pPropertyCount, VkSparseImageFormatProperties* pProperties) const;

private:
	const VkPhysicalDeviceLimits& getLimits() const;
	VkSampleCountFlags getSampleCounts() const;

	const char *const name = nullptr;
};

static PhysicalDevice* Cast(VkPhysicalDevice physicalDevice) { return reinterpret_cast<PhysicalDevice*>(physicalDevice); }

} // namespace vk

#endif // VK_PHYSICAL_DEVICE_HPP_