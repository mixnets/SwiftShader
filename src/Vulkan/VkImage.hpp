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

#include "VkBuffer.hpp"

namespace vk {

VkClass(Image) {
public:
	constexpr static VkSystemAllocationScope kAllocationScope = VK_SYSTEM_ALLOCATION_SCOPE_OBJECT;

	Image(const VkAllocationCallbacks* pAllocator, const VkImageCreateInfo* pCreateInfo) :
		flags(pCreateInfo->flags),
		imageType(pCreateInfo->imageType),
		format(pCreateInfo->format),
		extent(pCreateInfo->extent),
		mipLevels(pCreateInfo->mipLevels),
		arrayLayers(pCreateInfo->arrayLayers),
		samples(pCreateInfo->samples),
		tiling(pCreateInfo->tiling),
		usage(pCreateInfo->usage),
		sharingMode(pCreateInfo->sharingMode),
		queueFamilyIndexCount(pCreateInfo->queueFamilyIndexCount),
		initialLayout(pCreateInfo->initialLayout)
	{
		size_t queueFamilyIndicesSize = sizeof(uint32_t) * queueFamilyIndexCount;
		queueFamilyIndices = reinterpret_cast<uint32_t*>(vk::allocate(
			queueFamilyIndicesSize, pAllocator, kAllocationScope, pCreateInfo->pQueueFamilyIndices));
	}

	~Image() = delete;

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
		memoryRequirements.alignment = vk::REQUIRED_MEMORY_ALIGNMENT;
		memoryRequirements.memoryTypeBits = vk::REQUIRED_MEMORY_TYPE_BITS;
		memoryRequirements.size = getSize();
		return memoryRequirements;
	}

	void bind(VkDeviceMemory pDeviceMemory, VkDeviceSize pMemoryOffset)
	{
		deviceMemory = pDeviceMemory;
		memoryOffset = pMemoryOffset;
	}

	void copyTo(VkImageLayout srcImageLayout, VkBuffer dstBuffer,
	            uint32_t regionCount, const VkBufferImageCopy* pRegions)
	{
		for(uint32_t i = 0; i < regionCount; i++)
		{
			copyTo(srcImageLayout, dstBuffer, pRegions[i]);
		}
	}

	void copyTo(VkImageLayout srcImageLayout, VkBuffer dstBuffer, const VkBufferImageCopy& pRegion)
	{
		// FIXME : This is where we would use the blitter
	}

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