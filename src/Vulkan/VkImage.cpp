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
#include "VkImage.hpp"
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
	initialLayout(pCreateInfo->initialLayout)
{
}

void Image::destroy(const VkAllocationCallbacks* pAllocator)
{
}

size_t Image::ComputeRequiredAllocationSize(const VkImageCreateInfo* pCreateInfo)
{
	return 0;
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
	// Image copy does not perform any conversion, it simply copies memory from
	// an image to another image that has the same number of bytes per pixel.
	Image* dst = Cast(dstImage);
	int srcBytes = bytes();
	ASSERT(srcBytes == dst->bytes());

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

	const char* srcMem = static_cast<const char*>(Cast(deviceMemory)->getOffsetPointer(computeOffsetB(pRegion.srcOffset)));
	char* dstMem = static_cast<char*>(Cast(dst->deviceMemory)->getOffsetPointer(dst->computeOffsetB(pRegion.dstOffset)));

	int srcPitchB = pitchB();
	int srcSliceB = sliceB();
	int dstPitchB = dst->pitchB();
	int dstSliceB = dst->sliceB();

	bool isSingleLine = (pRegion.extent.height == 1) && (pRegion.extent.depth == 1);
	bool isEntireLine = (pRegion.extent.width == extent.width) && (pRegion.extent.width == dst->extent.width) && (srcPitchB == dstPitchB);
	bool isSinglePlane = (pRegion.extent.depth == 1);
	bool isEntirePlane = isEntireLine && (pRegion.extent.height == extent.height) && (pRegion.extent.height == dst->extent.height) && (srcSliceB == dstSliceB);

	if(isSingleLine || (isEntireLine && (isSinglePlane || isEntirePlane)))
	{
		// Single memcpy
		memcpy(dstMem, srcMem, isSingleLine ? pRegion.extent.width * srcBytes : (isSinglePlane ? pRegion.extent.height * srcPitchB : pRegion.extent.depth * srcSliceB));
	}
	else if(isEntireLine) // Copy plane by plane
	{
		for(uint32_t z = 0; z < pRegion.extent.depth; z++, dstMem += dstSliceB, srcMem += srcSliceB)
		{
			memcpy(dstMem, srcMem, pRegion.extent.height * srcPitchB);
		}
	}
	else // Copy line by line
	{
		for(uint32_t z = 0; z < pRegion.extent.depth; z++)
		{
			for(uint32_t y = 0; y < pRegion.extent.height; y++, dstMem += dstPitchB, srcMem += srcPitchB)
			{
				memcpy(dstMem, srcMem, pRegion.extent.width * srcBytes);
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

	Cast(dstBuffer)->copyFrom(Cast(deviceMemory)->getOffsetPointer(0),
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

	Cast(srcBuffer)->copyTo(Cast(deviceMemory)->getOffsetPointer(0),
		sw::Surface::sliceB(pRegion.imageExtent.width, pRegion.imageExtent.height,
			getBorder(), format, false) * pRegion.imageExtent.depth,
		pRegion.bufferOffset);
}

VkDeviceSize Image::computeOffsetB(VkOffset3D offset) const
{
	return offset.z * sliceB() + offset.y * pitchB() + offset.x * bytes();
}

int Image::pitchB() const
{
	return sw::Surface::pitchB(extent.width, getBorder(), format, false);
}

int Image::sliceB() const
{
	return sw::Surface::sliceB(extent.width, extent.height, getBorder(), format, false);
}

int Image::bytes() const
{
	return sw::Surface::bytes(format);
}

int Image::getBorder() const
{
	return ((flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) && (imageType == VK_IMAGE_TYPE_2D)) ? 1 : 0;
}

VkDeviceSize Image::getSize() const
{
	if(mipLevels > 1)
	{
		UNIMPLEMENTED();
	}

	return extent.depth * sliceB();
}

} // namespace vk