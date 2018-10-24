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

#include "VkDeviceMemory.hpp"
#include "VkBuffer.hpp"
#include "VkConfig.h"
#include "VkDeviceMemory.hpp"
#include "VkImage.hpp"
#include "Device/Blitter.hpp"
#include "Device/Surface.hpp"
#include <cstring>

namespace vk
{

Image::Image(const VkImageCreateInfo* pCreateInfo, void* mem) :
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
	initialLayout(pCreateInfo->initialLayout),
	queueFamilyIndexCount(pCreateInfo->queueFamilyIndexCount),
	queueFamilyIndices(reinterpret_cast<uint32_t*>(mem))
{
	size_t queueFamilyIndicesSize = sizeof(uint32_t) *pCreateInfo->queueFamilyIndexCount;
	memcpy(queueFamilyIndices, pCreateInfo->pQueueFamilyIndices, queueFamilyIndicesSize);
}

void Image::destroy(const VkAllocationCallbacks* pAllocator)
{
	vk::deallocate(queueFamilyIndices, pAllocator);
}

size_t Image::ComputeRequiredAllocationSize(const VkImageCreateInfo* pCreateInfo)
{
	return sizeof(uint32_t) *pCreateInfo->queueFamilyIndexCount;
}

const VkMemoryRequirements Image::getMemoryRequirements() const
{
	VkMemoryRequirements memoryRequirements;
	memoryRequirements.alignment = vk::REQUIRED_MEMORY_ALIGNMENT;
	memoryRequirements.memoryTypeBits = vk::MEMORY_TYPE_GENERIC_BIT;
	memoryRequirements.size = getStorageSize();
	return memoryRequirements;
}

void Image::bind(VkDeviceMemory pDeviceMemory, VkDeviceSize pMemoryOffset)
{
	deviceMemory = pDeviceMemory;
	memoryOffset = pMemoryOffset;
}

void Image::getImageMipTailInfo(VkSparseImageMemoryRequirements* pSparseMemoryRequirements)
{
	pSparseMemoryRequirements->imageMipTailFirstLod = 1;
	pSparseMemoryRequirements->imageMipTailSize = getStorageSize();
	pSparseMemoryRequirements->imageMipTailOffset = getStorageSize();
	pSparseMemoryRequirements->imageMipTailStride = 0;
}

void Image::getSubresourceLayout(const VkImageSubresource* pSubresource, VkSubresourceLayout* pLayout) const
{
	if(pSubresource->arrayLayer >= arrayLayers)
	{
		pLayout->offset = getStorageSize();
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

		uint32_t bpp = bytesPerTexel();
		pLayout->offset = 0;
		pLayout->rowPitch = rowPitchBytes();
		pLayout->depthPitch = slicePitchBytes();
		pLayout->arrayPitch = pLayout->depthPitch * extent.depth;
		pLayout->size = pLayout->arrayPitch;
	}
}

VkDeviceSize Image::computeNumberOfPixels(uint32_t width, uint32_t height, uint32_t depth) const
{
	uint32_t levels = mipLevels;

	width += 2 * getBorder(); // For seamless cube border
	height += 2 * getBorder(); // For seamless cube border

	VkDeviceSize numPixels = samples *
		(arrayLayers > MAX_IMAGE_ARRAY_LAYERS) ? MAX_IMAGE_ARRAY_LAYERS : arrayLayers;

	switch(imageType)
	{
	case VK_IMAGE_TYPE_3D:
		numPixels *= width * height * depth;
		break;
	case VK_IMAGE_TYPE_2D:
		numPixels *= width * height;
		break;
	case VK_IMAGE_TYPE_1D:
		numPixels *= width;
		break;
	default:
		UNIMPLEMENTED();
	}

	return numPixels;
}

void Image::copyTo(VkImage dstImage, const VkImageCopy& pRegion)
{
	// Image copy does not perform any conversion, it simply copies memory from
	// an image to another image that has the same number of bytes per pixel.
	Image* dst = Cast(dstImage);
	int srcBytesPerTexel = bytesPerTexel();
	ASSERT(srcBytesPerTexel == dst->bytesPerTexel());

	if(!((pRegion.srcSubresource.aspectMask == VK_IMAGE_ASPECT_COLOR_BIT) ||
		 (pRegion.srcSubresource.aspectMask == VK_IMAGE_ASPECT_DEPTH_BIT) ||
		 (pRegion.srcSubresource.aspectMask == VK_IMAGE_ASPECT_STENCIL_BIT)) ||
	   (pRegion.srcSubresource.baseArrayLayer != 0) ||
	   (pRegion.srcSubresource.layerCount != 1) ||
	   (pRegion.srcSubresource.mipLevel != 0))
	{
		UNIMPLEMENTED();
	}

	if(!((pRegion.dstSubresource.aspectMask == VK_IMAGE_ASPECT_COLOR_BIT) ||
		 (pRegion.dstSubresource.aspectMask == VK_IMAGE_ASPECT_DEPTH_BIT) ||
		 (pRegion.dstSubresource.aspectMask == VK_IMAGE_ASPECT_STENCIL_BIT)) ||
	   (pRegion.dstSubresource.baseArrayLayer != 0) ||
	   (pRegion.dstSubresource.layerCount != 1) ||
	   (pRegion.dstSubresource.mipLevel != 0))
	{
		UNIMPLEMENTED();
	}

	const char* srcMem = static_cast<const char*>(getTexelPointer(pRegion.srcOffset));
	char* dstMem = static_cast<char*>(dst->getTexelPointer(pRegion.dstOffset));

	int srcRowPitchBytes = rowPitchBytes();
	int srcSlicePitchBytes = slicePitchBytes();
	int dstRowPitchBytes = dst->rowPitchBytes();
	int dstSlicePitchBytes = dst->slicePitchBytes();

	bool isSingleLine = (pRegion.extent.height == 1) && (pRegion.extent.depth == 1);
	bool isEntireLine = (pRegion.extent.width == extent.width) && (pRegion.extent.width == dst->extent.width) && (srcRowPitchBytes == dstRowPitchBytes);
	bool isSinglePlane = (pRegion.extent.depth == 1);
	bool isEntirePlane = isEntireLine && (pRegion.extent.height == extent.height) && (pRegion.extent.height == dst->extent.height) && (srcSlicePitchBytes == dstSlicePitchBytes);

	if(isSingleLine)
	{
		memcpy(dstMem, srcMem, pRegion.extent.width * srcBytesPerTexel); // Copy one line
	}
	else if(isEntireLine && isSinglePlane)
	{
		memcpy(dstMem, srcMem, pRegion.extent.height * srcRowPitchBytes); // Copy one plane
	}
	else if(isEntireLine && isEntirePlane)
	{
		memcpy(dstMem, srcMem, pRegion.extent.depth * srcSlicePitchBytes); // Copy multiple planes
	}
	else if(isEntireLine) // Copy plane by plane
	{
		for(uint32_t z = 0; z < pRegion.extent.depth; z++, dstMem += dstSlicePitchBytes, srcMem += srcSlicePitchBytes)
		{
			memcpy(dstMem, srcMem, pRegion.extent.height * srcRowPitchBytes);
		}
	}
	else // Copy line by line
	{
		for(uint32_t z = 0; z < pRegion.extent.depth; z++)
		{
			for(uint32_t y = 0; y < pRegion.extent.height; y++, dstMem += dstRowPitchBytes, srcMem += srcRowPitchBytes)
			{
				memcpy(dstMem, srcMem, pRegion.extent.width * srcBytesPerTexel);
			}
		}
	}
}

void Image::copyTo(VkBuffer dstBuffer, const VkBufferImageCopy& pRegion)
{
	if((pRegion.imageExtent.width != extent.width) ||
	   (pRegion.imageExtent.height != extent.height) ||
	   (pRegion.imageExtent.depth != extent.depth) ||
	   !((pRegion.imageSubresource.aspectMask == VK_IMAGE_ASPECT_COLOR_BIT) ||
	     (pRegion.imageSubresource.aspectMask == VK_IMAGE_ASPECT_DEPTH_BIT) ||
	     (pRegion.imageSubresource.aspectMask == VK_IMAGE_ASPECT_STENCIL_BIT)) ||
	     (pRegion.imageSubresource.baseArrayLayer != 0) ||
	   (pRegion.imageSubresource.layerCount != 1) ||
	   (pRegion.imageSubresource.mipLevel != 0) ||
	   (pRegion.imageOffset.x != 0) ||
	   (pRegion.imageOffset.y != 0) ||
	   (pRegion.imageOffset.z != 0) ||
	   (pRegion.bufferRowLength != extent.width) ||
	   (pRegion.bufferImageHeight != extent.height))
	{
		UNIMPLEMENTED();
	}

	Cast(dstBuffer)->copyFrom(Cast(deviceMemory)->getOffsetPointer(memoryOffset),
		sw::Surface::sliceB(pRegion.imageExtent.width, pRegion.imageExtent.height,
			getBorder(), format, false) * pRegion.imageExtent.depth,
		pRegion.bufferOffset);
}

void Image::copyFrom(VkBuffer srcBuffer, const VkBufferImageCopy& pRegion)
{
	if((pRegion.imageExtent.width != extent.width) ||
	   (pRegion.imageExtent.height != extent.height) ||
	   (pRegion.imageExtent.depth != extent.depth) ||
	   !((pRegion.imageSubresource.aspectMask == VK_IMAGE_ASPECT_COLOR_BIT) ||
	     (pRegion.imageSubresource.aspectMask == VK_IMAGE_ASPECT_DEPTH_BIT) ||
	     (pRegion.imageSubresource.aspectMask == VK_IMAGE_ASPECT_STENCIL_BIT)) ||
	     (pRegion.imageSubresource.baseArrayLayer != 0) ||
	   (pRegion.imageSubresource.layerCount != 1) ||
	   (pRegion.imageSubresource.mipLevel != 0) ||
	   (pRegion.imageOffset.x != 0) ||
	   (pRegion.imageOffset.y != 0) ||
	   (pRegion.imageOffset.z != 0) ||
	   (pRegion.bufferRowLength != extent.width) ||
	   (pRegion.bufferImageHeight != extent.height))
	{
		UNIMPLEMENTED();
	}

	Cast(srcBuffer)->copyTo(Cast(deviceMemory)->getOffsetPointer(memoryOffset),
		sw::Surface::sliceB(pRegion.imageExtent.width, pRegion.imageExtent.height,
			getBorder(), format, false) * pRegion.imageExtent.depth,
		pRegion.bufferOffset);
}

void* Image::getTexelPointer(const VkOffset3D& offset) const
{
	return Cast(deviceMemory)->getOffsetPointer(texelOffsetBytesInStorage(offset) + memoryOffset);
}

VkDeviceSize Image::texelOffsetBytesInStorage(const VkOffset3D& offset) const
{
	return offset.z * slicePitchBytes() + offset.y * rowPitchBytes() + offset.x * bytesPerTexel();
}

int Image::rowPitchBytes() const
{
	return sw::Surface::pitchB(extent.width, getBorder(), format, false);
}

int Image::slicePitchBytes() const
{
	return sw::Surface::sliceB(extent.width, extent.height, getBorder(), format, false);
}

int Image::bytesPerTexel() const
{
	return sw::Surface::bytes(format);
}

int Image::getBorder() const
{
	return ((flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) && (imageType == VK_IMAGE_TYPE_2D)) ? 1 : 0;
}

void Image::clear(const VkClearValue& clearValue, const VkRect2D& renderArea, const VkImageSubresourceRange& subresourceRange)
{
	if((subresourceRange.aspectMask != VK_IMAGE_ASPECT_COLOR_BIT) ||
	   (subresourceRange.baseMipLevel != 0) ||
	   (subresourceRange.levelCount != 1) ||
	   (subresourceRange.baseArrayLayer != 0) ||
	   (subresourceRange.layerCount != 1))
	{
		UNIMPLEMENTED();
	}

	// Set the proper format for the clear value, as described here:
	// https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#clears-values
	VkFormat clearFormat = VK_FORMAT_R32G32B32A32_SFLOAT;
	if(sw::Surface::isSignedNonNormalizedInteger(format))
	{
		clearFormat = VK_FORMAT_R32G32B32A32_SINT;
	}
	else if(sw::Surface::isUnsignedNonNormalizedInteger(format))
	{
		clearFormat = VK_FORMAT_R32G32B32A32_UINT;
	}

	const sw::Rect rect(renderArea.offset.x, renderArea.offset.y,
	                    renderArea.offset.x + renderArea.extent.width,
	                    renderArea.offset.y + renderArea.extent.height);
	const sw::SliceRect dRect(rect);

	sw::Surface* surface = sw::Surface::create(extent.width, extent.height, extent.depth, format,
		Cast(deviceMemory)->getOffsetPointer(memoryOffset), rowPitchBytes(), slicePitchBytes());
	sw::Blitter blitter;
	blitter.clear((void*)clearValue.color.float32, clearFormat, surface, dRect, 0xF);
	delete surface;
}

VkDeviceSize Image::getStorageSize() const
{
	VkDeviceSize numPixels = 0;

	uint32_t levels = mipLevels;
	numPixels += computeNumberOfPixels(extent.width + 2 * getBorder(), extent.height + 2 * getBorder(), extent.depth);
	if((flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) && (imageType == VK_IMAGE_TYPE_2D) && (levels > MAX_IMAGE_LEVELS_CUBE))
	{
		levels = MAX_IMAGE_LEVELS_CUBE;
	}

	switch(imageType)
	{
	case VK_IMAGE_TYPE_3D:
		if(levels > MAX_IMAGE_LEVELS_3D) { levels = MAX_IMAGE_LEVELS_3D;  }
		break;
	case VK_IMAGE_TYPE_2D:
		if(levels > MAX_IMAGE_LEVELS_2D) { levels = MAX_IMAGE_LEVELS_2D; }
		break;
	case VK_IMAGE_TYPE_1D:
		if(levels > MAX_IMAGE_LEVELS_1D) { levels = MAX_IMAGE_LEVELS_1D; }
		break;
	default:
		UNIMPLEMENTED();
	}

	uint32_t width = extent.width;
	uint32_t height = extent.height;
	uint32_t depth = extent.depth;
	while(levels > 1)
	{
		bool zeroSized = false;
		switch(imageType)
		{
		case VK_IMAGE_TYPE_3D:
			depth >>= 1;
			height >>= 1;
			width >>= 1;
			zeroSized = (!width || !height || !depth);
			break;
		case VK_IMAGE_TYPE_2D:
			height >>= 1;
			width >>= 1;
			zeroSized = (!width || !height);
			break;
		case VK_IMAGE_TYPE_1D:
			width >>= 1;
			zeroSized = (!width);
			break;
		}

		if(zeroSized)
		{
			break;
		}

		numPixels += computeNumberOfPixels(width + 2 * getBorder(), height + 2 * getBorder(), depth);
		--levels;
	}

	// Note: We may need to reserve space for a "descriptor structure" that shaders will read
	return numPixels * bytesPerTexel();
}

VkImageAspectFlags Image::getImageAspect(VkFormat format)
{
	switch(format)
	{
	case VK_FORMAT_D16_UNORM:
	case VK_FORMAT_X8_D24_UNORM_PACK32:
	case VK_FORMAT_D32_SFLOAT:
		return VK_IMAGE_ASPECT_DEPTH_BIT;
	case VK_FORMAT_S8_UINT:
		return VK_IMAGE_ASPECT_STENCIL_BIT;
	case VK_FORMAT_D16_UNORM_S8_UINT:
	case VK_FORMAT_D24_UNORM_S8_UINT:
	case VK_FORMAT_D32_SFLOAT_S8_UINT:
		return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	default:
		return VK_IMAGE_ASPECT_COLOR_BIT;
	}
}

} // namespace vk