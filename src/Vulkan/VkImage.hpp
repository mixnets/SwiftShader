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

namespace vk
{

class Image : public VkObject<Image, VkImage>
{
public:
	Image(const VkImageCreateInfo* pCreateInfo, uint32_t* pQueueFamilyIndices);
	~Image() = delete;
	void destroy(const VkAllocationCallbacks* pAllocator) override;

	static VkResult AllocateQueueFamilyIndices(const VkAllocationCallbacks* pAllocator,
		const VkImageCreateInfo* pCreateInfo, uint32_t** queueFamilyIndices);
	static void DestroyQueueFamilyIndices(const VkAllocationCallbacks* pAllocator,
		uint32_t* queueFamilyIndices);

	VkDeviceSize getSize() const;
	const VkMemoryRequirements getMemoryRequirements() const;
	void bind(VkDeviceMemory pDeviceMemory, VkDeviceSize pMemoryOffset);
	void copyTo(VkImageLayout srcImageLayout, VkBuffer dstBuffer,
		uint32_t regionCount, const VkBufferImageCopy* pRegions);
	void copyTo(VkImageLayout srcImageLayout, VkBuffer dstBuffer, const VkBufferImageCopy& pRegion);
	void getImageMipTailInfo(VkSparseImageMemoryRequirements* pSparseMemoryRequirements);
	void getSubresourceLayout(const VkImageSubresource* pSubresource, VkSubresourceLayout* pLayout) const;

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