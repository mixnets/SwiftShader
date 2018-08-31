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
#include <memory.h>

namespace vk
{

class Image : public VkObject<Image, VkImage>
{
public:
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
			queueFamilyIndicesSize, pAllocator, GetAllocationScope()));
		if(queueFamilyIndices)
		{
			memcpy(queueFamilyIndices, pCreateInfo->pQueueFamilyIndices, queueFamilyIndicesSize);
		}
	}

	~Image() = delete;

	void destroy(const VkAllocationCallbacks* pAllocator) override
	{
		vk::deallocate(queueFamilyIndices, pAllocator);
		queueFamilyIndices = nullptr;
		queueFamilyIndexCount = 0;
	}

	bool validate() const override
	{
		return !!queueFamilyIndices;
	}

	VkDeviceSize getSize() const;

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

	void getImageMipTailInfo(VkSparseImageMemoryRequirements* pSparseMemoryRequirements)
	{
		pSparseMemoryRequirements->imageMipTailFirstLod = 1;
		pSparseMemoryRequirements->imageMipTailSize = getSize();
		pSparseMemoryRequirements->imageMipTailOffset = getSize();
		pSparseMemoryRequirements->imageMipTailStride = 0;
	}

	void getSubresourceLayout(const VkImageSubresource* pSubresource, VkSubresourceLayout* pLayout) const
	{
		if(pSubresource->arrayLayer >= arrayLayers)
		{
			pLayout->offset = getSize();
			pLayout->size = 0;
			pLayout->rowPitch = 0;
			pLayout->depthPitch = 0;
			pLayout->arrayPitch = 0;
		}
		else
		{
			if(pSubresource->mipLevel > 0 || pSubresource->arrayLayer > 0)
			{
				UNIMPLEMENTED();
			}

			uint32_t bpp = getBytesPerPixel();
			pLayout->offset = 0;
			pLayout->size = computeNumberOfPixels(extent.width, extent.height, extent.depth) * bpp;
			pLayout->rowPitch = extent.width * bpp;
			pLayout->depthPitch = pLayout->rowPitch * extent.height;
			pLayout->arrayPitch = pLayout->depthPitch * extent.depth;
		}
	}

	VkImageType              getImageType() const { return imageType; }
	VkFormat                 getFormat() const { return format; }
	VkSampleCountFlagBits    getSamples() const { return samples; }
	VkImageTiling            getTiling() const { return tiling; }
	VkImageUsageFlags        getUsage() const { return usage; }

	static VkImageAspectFlags getImageAspect(VkFormat format);

private:
	VkDeviceSize computeNumberOfPixels(uint32_t width, uint32_t height, uint32_t depth) const;
	uint32_t getBytesPerPixel() const;

	VkDeviceMemory           deviceMemory = nullptr;
	VkDeviceSize             memoryOffset = 0;
	VkImageCreateFlags       flags = 0;
	VkImageType              imageType = VK_IMAGE_TYPE_2D;
	VkFormat                 format = VK_FORMAT_UNDEFINED;
	VkExtent3D               extent = {0, 0, 0};
	uint32_t                 mipLevels = 0;
	uint32_t                 arrayLayers = 0;
	VkSampleCountFlagBits    samples = VK_SAMPLE_COUNT_1_BIT;
	VkImageTiling            tiling = VK_IMAGE_TILING_OPTIMAL;
	VkImageUsageFlags        usage = 0;
	VkSharingMode            sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	VkImageLayout            initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	uint32_t                 queueFamilyIndexCount = 0;
	uint32_t*                queueFamilyIndices = nullptr;
};

static inline Image* Cast(VkImage object)
{
	return reinterpret_cast<Image*>(object);
}

} // namespace vk

#endif // VK_IMAGE_HPP_