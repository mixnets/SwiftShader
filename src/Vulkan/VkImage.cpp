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
#include "VkImage.hpp"
#include <memory.h>

namespace vk
{

Image::Image(const VkImageCreateInfo* pCreateInfo, const Memory& mem) :
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
	queueFamilyIndices(reinterpret_cast<uint32_t*>(mem.host))
{
	size_t queueFamilyIndicesSize = sizeof(uint32_t) *pCreateInfo->queueFamilyIndexCount;
	memcpy(queueFamilyIndices, pCreateInfo->pQueueFamilyIndices, queueFamilyIndicesSize);
}

void Image::destroy(const VkAllocationCallbacks* pAllocator)
{
	vk::deallocate(queueFamilyIndices, pAllocator);
}

MemorySize Image::ComputeRequiredAllocationSize(const VkImageCreateInfo* pCreateInfo)
{
	return MemorySize(sizeof(uint32_t) *pCreateInfo->queueFamilyIndexCount, 0);
}

const VkMemoryRequirements Image::getMemoryRequirements() const
{
	VkMemoryRequirements memoryRequirements;
	memoryRequirements.alignment = vk::REQUIRED_MEMORY_ALIGNMENT;
	memoryRequirements.memoryTypeBits = vk::REQUIRED_MEMORY_TYPE_BITS;
	memoryRequirements.size = getSize();
	return memoryRequirements;
}

void Image::bind(VkDeviceMemory pDeviceMemory, VkDeviceSize pMemoryOffset)
{
	deviceMemory = pDeviceMemory;
	memoryOffset = pMemoryOffset;
}

void Image::copyTo(VkImageLayout srcImageLayout, VkBuffer dstBuffer,
	uint32_t regionCount, const VkBufferImageCopy* pRegions)
{
	for(uint32_t i = 0; i < regionCount; i++)
	{
		copyTo(srcImageLayout, dstBuffer, pRegions[i]);
	}
}

void Image::copyTo(VkImageLayout srcImageLayout, VkBuffer dstBuffer, const VkBufferImageCopy& pRegion)
{
	// FIXME : This is where we would use the blitter
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

		uint32_t bpp = getBytesPerPixel();
		pLayout->offset = 0;
		pLayout->size = computeNumberOfPixels(extent.width, extent.height, extent.depth) * bpp;
		pLayout->rowPitch = extent.width * bpp;
		pLayout->depthPitch = pLayout->rowPitch * extent.height;
		pLayout->arrayPitch = pLayout->depthPitch * extent.depth;
	}
}

VkDeviceSize Image::computeNumberOfPixels(uint32_t width, uint32_t height, uint32_t depth) const
{
	uint32_t levels = mipLevels;

	bool isCube = (flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) && (imageType == VK_IMAGE_TYPE_2D);
	if(isCube)
	{
		width += 2; // For seamless cube border
		height += 2; // For seamless cube border
	}

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

uint32_t Image::getBytesPerPixel() const
{
	switch(format)
	{
	case VK_FORMAT_R4G4_UNORM_PACK8:
	case VK_FORMAT_R8_UNORM:
	case VK_FORMAT_R8_SNORM:
	case VK_FORMAT_R8_USCALED:
	case VK_FORMAT_R8_SSCALED:
	case VK_FORMAT_R8_UINT:
	case VK_FORMAT_R8_SINT:
	case VK_FORMAT_R8_SRGB:
	case VK_FORMAT_S8_UINT:
		return 1;
	case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
	case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
	case VK_FORMAT_R5G6B5_UNORM_PACK16:
	case VK_FORMAT_B5G6R5_UNORM_PACK16:
	case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
	case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
	case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
	case VK_FORMAT_R8G8_UNORM:
	case VK_FORMAT_R8G8_SNORM:
	case VK_FORMAT_R8G8_USCALED:
	case VK_FORMAT_R8G8_SSCALED:
	case VK_FORMAT_R8G8_UINT:
	case VK_FORMAT_R8G8_SINT:
	case VK_FORMAT_R8G8_SRGB:
	case VK_FORMAT_R16_UNORM:
	case VK_FORMAT_R16_SNORM:
	case VK_FORMAT_R16_USCALED:
	case VK_FORMAT_R16_SSCALED:
	case VK_FORMAT_R16_UINT:
	case VK_FORMAT_R16_SINT:
	case VK_FORMAT_R16_SFLOAT:
	case VK_FORMAT_D16_UNORM:
		return 2;
	case VK_FORMAT_R8G8B8_UNORM:
	case VK_FORMAT_R8G8B8_SNORM:
	case VK_FORMAT_R8G8B8_USCALED:
	case VK_FORMAT_R8G8B8_SSCALED:
	case VK_FORMAT_R8G8B8_UINT:
	case VK_FORMAT_R8G8B8_SINT:
	case VK_FORMAT_R8G8B8_SRGB:
	case VK_FORMAT_B8G8R8_UNORM:
	case VK_FORMAT_B8G8R8_SNORM:
	case VK_FORMAT_B8G8R8_USCALED:
	case VK_FORMAT_B8G8R8_SSCALED:
	case VK_FORMAT_B8G8R8_UINT:
	case VK_FORMAT_B8G8R8_SINT:
	case VK_FORMAT_B8G8R8_SRGB:
	case VK_FORMAT_D16_UNORM_S8_UINT:
		return 3;
	case VK_FORMAT_R8G8B8A8_UNORM:
	case VK_FORMAT_R8G8B8A8_SNORM:
	case VK_FORMAT_R8G8B8A8_USCALED:
	case VK_FORMAT_R8G8B8A8_SSCALED:
	case VK_FORMAT_R8G8B8A8_UINT:
	case VK_FORMAT_R8G8B8A8_SINT:
	case VK_FORMAT_R8G8B8A8_SRGB:
	case VK_FORMAT_B8G8R8A8_UNORM:
	case VK_FORMAT_B8G8R8A8_SNORM:
	case VK_FORMAT_B8G8R8A8_USCALED:
	case VK_FORMAT_B8G8R8A8_SSCALED:
	case VK_FORMAT_B8G8R8A8_UINT:
	case VK_FORMAT_B8G8R8A8_SINT:
	case VK_FORMAT_B8G8R8A8_SRGB:
	case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
	case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
	case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
	case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
	case VK_FORMAT_A8B8G8R8_UINT_PACK32:
	case VK_FORMAT_A8B8G8R8_SINT_PACK32:
	case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
	case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
	case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
	case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
	case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
	case VK_FORMAT_A2R10G10B10_UINT_PACK32:
	case VK_FORMAT_A2R10G10B10_SINT_PACK32:
	case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
	case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
	case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
	case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
	case VK_FORMAT_A2B10G10R10_UINT_PACK32:
	case VK_FORMAT_A2B10G10R10_SINT_PACK32:
	case VK_FORMAT_R16G16_UNORM:
	case VK_FORMAT_R16G16_SNORM:
	case VK_FORMAT_R16G16_USCALED:
	case VK_FORMAT_R16G16_SSCALED:
	case VK_FORMAT_R16G16_UINT:
	case VK_FORMAT_R16G16_SINT:
	case VK_FORMAT_R16G16_SFLOAT:
	case VK_FORMAT_R32_UINT:
	case VK_FORMAT_R32_SINT:
	case VK_FORMAT_R32_SFLOAT:
	case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
	case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
	case VK_FORMAT_X8_D24_UNORM_PACK32:
	case VK_FORMAT_D32_SFLOAT:
	case VK_FORMAT_D24_UNORM_S8_UINT:
		return 4;
	case VK_FORMAT_D32_SFLOAT_S8_UINT:
		return 5;
	case VK_FORMAT_R16G16B16_UNORM:
	case VK_FORMAT_R16G16B16_SNORM:
	case VK_FORMAT_R16G16B16_USCALED:
	case VK_FORMAT_R16G16B16_SSCALED:
	case VK_FORMAT_R16G16B16_UINT:
	case VK_FORMAT_R16G16B16_SINT:
	case VK_FORMAT_R16G16B16_SFLOAT:
		return 6;
	case VK_FORMAT_R16G16B16A16_UNORM:
	case VK_FORMAT_R16G16B16A16_SNORM:
	case VK_FORMAT_R16G16B16A16_USCALED:
	case VK_FORMAT_R16G16B16A16_SSCALED:
	case VK_FORMAT_R16G16B16A16_UINT:
	case VK_FORMAT_R16G16B16A16_SINT:
	case VK_FORMAT_R16G16B16A16_SFLOAT:
	case VK_FORMAT_R32G32_UINT:
	case VK_FORMAT_R32G32_SINT:
	case VK_FORMAT_R32G32_SFLOAT:
	case VK_FORMAT_R64_UINT:
	case VK_FORMAT_R64_SINT:
	case VK_FORMAT_R64_SFLOAT:
		return 8;
	case VK_FORMAT_R32G32B32_UINT:
	case VK_FORMAT_R32G32B32_SINT:
	case VK_FORMAT_R32G32B32_SFLOAT:
		return 12;
	case VK_FORMAT_R32G32B32A32_UINT:
	case VK_FORMAT_R32G32B32A32_SINT:
	case VK_FORMAT_R32G32B32A32_SFLOAT:
	case VK_FORMAT_R64G64_UINT:
	case VK_FORMAT_R64G64_SINT:
	case VK_FORMAT_R64G64_SFLOAT:
		return 16;
	case VK_FORMAT_R64G64B64_UINT:
	case VK_FORMAT_R64G64B64_SINT:
	case VK_FORMAT_R64G64B64_SFLOAT:
		return 24;
	case VK_FORMAT_R64G64B64A64_UINT:
	case VK_FORMAT_R64G64B64A64_SINT:
	case VK_FORMAT_R64G64B64A64_SFLOAT:
		return 32;
	case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
	case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
	case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
	case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
	case VK_FORMAT_BC2_UNORM_BLOCK:
	case VK_FORMAT_BC2_SRGB_BLOCK:
	case VK_FORMAT_BC3_UNORM_BLOCK:
	case VK_FORMAT_BC3_SRGB_BLOCK:
	case VK_FORMAT_BC4_UNORM_BLOCK:
	case VK_FORMAT_BC4_SNORM_BLOCK:
	case VK_FORMAT_BC5_UNORM_BLOCK:
	case VK_FORMAT_BC5_SNORM_BLOCK:
	case VK_FORMAT_BC6H_UFLOAT_BLOCK:
	case VK_FORMAT_BC6H_SFLOAT_BLOCK:
	case VK_FORMAT_BC7_UNORM_BLOCK:
	case VK_FORMAT_BC7_SRGB_BLOCK:
	case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
	case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:
	case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK:
	case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:
	case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:
	case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
	case VK_FORMAT_EAC_R11_UNORM_BLOCK:
	case VK_FORMAT_EAC_R11_SNORM_BLOCK:
	case VK_FORMAT_EAC_R11G11_UNORM_BLOCK:
	case VK_FORMAT_EAC_R11G11_SNORM_BLOCK:
	case VK_FORMAT_ASTC_4x4_UNORM_BLOCK:
	case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:
	case VK_FORMAT_ASTC_5x4_UNORM_BLOCK:
	case VK_FORMAT_ASTC_5x4_SRGB_BLOCK:
	case VK_FORMAT_ASTC_5x5_UNORM_BLOCK:
	case VK_FORMAT_ASTC_5x5_SRGB_BLOCK:
	case VK_FORMAT_ASTC_6x5_UNORM_BLOCK:
	case VK_FORMAT_ASTC_6x5_SRGB_BLOCK:
	case VK_FORMAT_ASTC_6x6_UNORM_BLOCK:
	case VK_FORMAT_ASTC_6x6_SRGB_BLOCK:
	case VK_FORMAT_ASTC_8x5_UNORM_BLOCK:
	case VK_FORMAT_ASTC_8x5_SRGB_BLOCK:
	case VK_FORMAT_ASTC_8x6_UNORM_BLOCK:
	case VK_FORMAT_ASTC_8x6_SRGB_BLOCK:
	case VK_FORMAT_ASTC_8x8_UNORM_BLOCK:
	case VK_FORMAT_ASTC_8x8_SRGB_BLOCK:
	case VK_FORMAT_ASTC_10x5_UNORM_BLOCK:
	case VK_FORMAT_ASTC_10x5_SRGB_BLOCK:
	case VK_FORMAT_ASTC_10x6_UNORM_BLOCK:
	case VK_FORMAT_ASTC_10x6_SRGB_BLOCK:
	case VK_FORMAT_ASTC_10x8_UNORM_BLOCK:
	case VK_FORMAT_ASTC_10x8_SRGB_BLOCK:
	case VK_FORMAT_ASTC_10x10_UNORM_BLOCK:
	case VK_FORMAT_ASTC_10x10_SRGB_BLOCK:
	case VK_FORMAT_ASTC_12x10_UNORM_BLOCK:
	case VK_FORMAT_ASTC_12x10_SRGB_BLOCK:
	case VK_FORMAT_ASTC_12x12_UNORM_BLOCK:
	case VK_FORMAT_ASTC_12x12_SRGB_BLOCK:
	case VK_FORMAT_G8B8G8R8_422_UNORM:
	case VK_FORMAT_B8G8R8G8_422_UNORM:
	case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:
	case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:
	case VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM:
	case VK_FORMAT_G8_B8R8_2PLANE_422_UNORM:
	case VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM:
	case VK_FORMAT_R10X6_UNORM_PACK16:
	case VK_FORMAT_R10X6G10X6_UNORM_2PACK16:
	case VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16:
	case VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16:
	case VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16:
	case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16:
	case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16:
	case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16:
	case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16:
	case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16:
	case VK_FORMAT_R12X4_UNORM_PACK16:
	case VK_FORMAT_R12X4G12X4_UNORM_2PACK16:
	case VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16:
	case VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16:
	case VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16:
	case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16:
	case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16:
	case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16:
	case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16:
	case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16:
	case VK_FORMAT_G16B16G16R16_422_UNORM:
	case VK_FORMAT_B16G16R16G16_422_UNORM:
	case VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM:
	case VK_FORMAT_G16_B16R16_2PLANE_420_UNORM:
	case VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM:
	case VK_FORMAT_G16_B16R16_2PLANE_422_UNORM:
	case VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM:
	case VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG:
	case VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG:
	case VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG:
	case VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG:
	case VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG:
	case VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG:
	case VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG:
	case VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG:
	default:
		UNIMPLEMENTED();
		return 0;
	}
}

VkDeviceSize Image::getSize() const
{
	VkDeviceSize numPixels = 0;

	uint32_t levels = mipLevels;
	bool isCube = (flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) && (imageType == VK_IMAGE_TYPE_2D);
	if(isCube)
	{
		// Add border at each level
		numPixels += computeNumberOfPixels(extent.width + 2, extent.height + 2, extent.depth);
		if(levels > MAX_IMAGE_LEVELS_CUBE) { levels = MAX_IMAGE_LEVELS_CUBE; }
	}
	else
	{
		numPixels += computeNumberOfPixels(extent.width, extent.height, extent.depth);
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

		if(isCube)
		{
			// Add border at each level
			numPixels += computeNumberOfPixels(width + 2, height + 2, depth);
		}
		else
		{
			numPixels += computeNumberOfPixels(width, height, depth);
		}
		--levels;
	}

	// Note: We may need to reserve space for a "descriptor structure" that shaders will read
	return numPixels * getBytesPerPixel();
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