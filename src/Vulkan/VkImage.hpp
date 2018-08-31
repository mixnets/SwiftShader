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

#ifndef VK_IMAGE_HPP_
#define VK_IMAGE_HPP_

#include "VkObject.hpp"

namespace vk {

VkClass(Image) {
public:
	Image(const VkAllocationCallbacks* pAllocator, VkImageCreateFlags flags,
	      VkImageType imageType, VkFormat format, VkExtent3D extent,
	      uint32_t mipLevels, uint32_t arrayLayers, VkSampleCountFlagBits samples,
	      VkImageTiling tiling, VkImageUsageFlags usage, VkSharingMode sharingMode,
	      uint32_t pQueueFamilyIndexCount, const uint32_t* pQueueFamilyIndices,
	      VkImageLayout initialLayout) :
		flags(flags),
		imageType(imageType),
		format(format),
		extent(extent),
		mipLevels(mipLevels),
		arrayLayers(arrayLayers),
		samples(samples),
		tiling(tiling),
		usage(usage),
		sharingMode(sharingMode),
		queueFamilyIndexCount(pQueueFamilyIndexCount),
		initialLayout(initialLayout)
	{
		size_t queueFamilyIndicesSize = sizeof(uint32_t) * queueFamilyIndexCount;
		queueFamilyIndices = reinterpret_cast<uint32_t*>(
			vk::allocate(queueFamilyIndicesSize, pAllocator, getSystemAllocationScope(), pQueueFamilyIndices));
	}

	void destroy(const VkAllocationCallbacks* pAllocator) override
	{
		vk::deallocate(queueFamilyIndices, pAllocator);
		queueFamilyIndices = nullptr;
		queueFamilyIndexCount = 0;
	}

	VkDeviceSize getSize();

	const VkMemoryRequirements GetMemoryRequirements()
	{
		VkMemoryRequirements memoryRequirements;
		memoryRequirements.alignment = 4;
		memoryRequirements.memoryTypeBits = 1;
		memoryRequirements.size = getSize();
		return memoryRequirements;
	}

	void bind(VkDeviceMemory pDeviceMemory, VkDeviceSize pMemoryOffset)
	{
		deviceMemory = pDeviceMemory;
		memoryOffset = pMemoryOffset;
	}

	static VkSystemAllocationScope getSystemAllocationScope() { return VK_SYSTEM_ALLOCATION_SCOPE_OBJECT; }

private:
	VkDeviceMemory           deviceMemory = nullptr;
	VkDeviceSize             memoryOffset = 0;
	VkImageCreateFlags       flags;
	VkImageType              imageType;
	VkFormat                 format;
	VkExtent3D               extent;
	uint32_t                 mipLevels;
	uint32_t                 arrayLayers;
	VkSampleCountFlagBits    samples;
	VkImageTiling            tiling;
	VkImageUsageFlags        usage;
	VkSharingMode            sharingMode;
	VkImageLayout            initialLayout;
	uint32_t                 queueFamilyIndexCount;
	uint32_t*                queueFamilyIndices;
};

} // namespace vk

#endif // VK_IMAGE_HPP_