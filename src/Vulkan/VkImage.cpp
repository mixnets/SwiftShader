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

#include "VkBuffer.hpp"
#include "VkConfig.h"
#include "VkDeviceMemory.hpp"
#include "VkImage.hpp"
#include "Device/Blitter.hpp"
#include <memory.h>

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
	memoryRequirements.size = getSize();
	return memoryRequirements;
}

void Image::bind(VkDeviceMemory pDeviceMemory, VkDeviceSize pMemoryOffset)
{
	deviceMemory = pDeviceMemory;
	memoryOffset = pMemoryOffset;
}

void Image::copyTo(VkImage dstImage, const VkImageCopy& pRegion)
{
	// FIXME : This is where we would use the blitter

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

	const sw::SliceRectF sRect(
		static_cast<float>(pRegion.srcOffset.x),
		static_cast<float>(pRegion.srcOffset.y),
		static_cast<float>(pRegion.srcOffset.x + pRegion.extent.width),
		static_cast<float>(pRegion.srcOffset.y + pRegion.extent.height),
		pRegion.srcOffset.z);
	const sw::SliceRect dRect(pRegion.dstOffset.x, pRegion.dstOffset.y,
		pRegion.dstOffset.x + pRegion.extent.width,
		pRegion.dstOffset.y + pRegion.extent.height,
		pRegion.dstOffset.z);

	ScopedSurface srcSurface(this);
	ScopedSurface dstSurface(Cast(dstImage));
	sw::Blitter blitter;
	blitter.blit(srcSurface.get(), sRect, dstSurface.get(), dRect, {false, false, false});
}

void Image::copyTo(VkBuffer dstBuffer, const VkBufferImageCopy& pRegion)
{
	// FIXME : This is where we would use the blitter

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

	Cast(dstBuffer)->copyFrom(Cast(deviceMemory)->getOffsetPointer(0),
		sw::Surface::sliceB(pRegion.imageExtent.width, pRegion.imageExtent.height,
		                    getBorder(), format, false) * pRegion.imageExtent.depth,
		pRegion.bufferOffset);
}

void Image::copyFrom(VkBuffer srcBuffer, const VkBufferImageCopy& pRegion)
{
	// FIXME : This is where we would use the blitter

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

	Cast(srcBuffer)->copyTo(Cast(deviceMemory)->getOffsetPointer(0),
		sw::Surface::sliceB(pRegion.imageExtent.width, pRegion.imageExtent.height,
			getBorder(), format, false) * pRegion.imageExtent.depth,
		pRegion.bufferOffset);
}

void Image::getImageMipTailInfo(VkSparseImageMemoryRequirements* pSparseMemoryRequirements)
{
	pSparseMemoryRequirements->imageMipTailFirstLod = 1;
	pSparseMemoryRequirements->imageMipTailSize = getSize();
	pSparseMemoryRequirements->imageMipTailOffset = getSize();
	pSparseMemoryRequirements->imageMipTailStride = 0;
}

void Image::getSubresourceLayout(const VkImageSubresource* pSubresource, VkSubresourceLayout* pLayout) const
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

		pLayout->offset = 0;
		pLayout->rowPitch = sw::Surface::pitchB(extent.width, getBorder(), format, false);
		pLayout->depthPitch = sw::Surface::sliceB(extent.width, extent.height, getBorder(), format, false);
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

int Image::getBorder() const
{
	return ((flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) && (imageType == VK_IMAGE_TYPE_2D)) ? 1 : 0;
}

VkDeviceSize Image::getSize() const
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
	return numPixels * sw::Surface::bytes(format);
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

void Image::clear(const VkClearValue& pClearValue, const VkRect2D& pRenderArea, unsigned int rgbaMask)
{
	VkFormat clearFormat = VK_FORMAT_R32G32B32A32_SFLOAT;
	if(sw::Surface::isSignedNonNormalizedInteger(format))
	{
		clearFormat = VK_FORMAT_R32G32B32A32_SINT;
	}
	else if(sw::Surface::isUnsignedNonNormalizedInteger(format))
	{
		clearFormat = VK_FORMAT_R32G32B32A32_UINT;
	}

	const sw::Rect rect(
		pRenderArea.offset.x, pRenderArea.offset.y,
		pRenderArea.offset.x + pRenderArea.extent.width,
		pRenderArea.offset.y + pRenderArea.extent.height);
	const sw::SliceRect dRect(rect);

	ScopedSurface scopedSurface(this);
	sw::Blitter blitter;
	blitter.clear((void*)pClearValue.color.float32, clearFormat, scopedSurface.get(), dRect, rgbaMask);
}

Image::ScopedSurface::ScopedSurface(Image* image)
{
	surface = sw::Surface::create(
		image->extent.width, image->extent.height, image->extent.depth, image->format, Cast(image->deviceMemory)->getOffsetPointer(0),
		sw::Surface::pitchB(image->extent.width, image->getBorder(), image->format, false),
		sw::Surface::sliceB(image->extent.width, image->extent.height, image->getBorder(), image->format, false));
}

Image::ScopedSurface::~ScopedSurface()
{
	delete surface;
}

} // namespace vk