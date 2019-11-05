// Copyright 2019 The SwiftShader Authors. All Rights Reserved.
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

#include "VkBufferCollectionFUCHSIA.hpp"

#include "VkConfig.h"
#include "VkDebug.hpp"
#include "VkDevice.hpp"
#include "VkPhysicalDevice.hpp"

#include <lib/fdio/directory.h>
#include <lib/zx/channel.h>
#include <zircon/syscalls.h>

namespace vk
{

// Technical note: Since there is no official spec for the extension, most of
// of the code here was inspired from the one found in:
//
//   $AOSP/device/generic/goldfish-opengl/system/vulkan_enc/ResourceTracker.cpp
//

// Global connection to the fuchsia.sysmem.Allocator service.

// Use FuchsiaSysmemAllocator::Get() to retrieve a reference to it.
// Use is_bound() on the result to check that the connection was properly
// established (this would typically fail if the service is not listed in
// the component's manifest of the current process).
class FuchsiaSysmemAllocator {
public:
	static fuchsia::sysmem::AllocatorSyncPtr& Get() {
		static FuchsiaSysmemAllocator sInstance;
		return sInstance.sysmemAllocator;
	}

private:
	FuchsiaSysmemAllocator()
	{
		zx::channel remote, local;
		zx_status_t status = zx::channel::create(0, &remote, &local);
		if (status != ZX_OK)
		{
			TRACE("zx::channel::create() failed with %d", status);
			return;
		}

		status = fdio_service_connect("/svc/fuchsia.sysmem.Allocator", remote.release());
		if (status != ZX_OK)
		{
			TRACE("Cannot connect to fuchsia.sysmem.Allocator service: %d", status);
			return;
		}
		sysmemAllocator.Bind(std::move(local));
	}

	fuchsia::sysmem::AllocatorSyncPtr sysmemAllocator;
};

// Convert a Vulkan image usage bitmask into the equivalent sysmem.BufferUsage.vulkan value.
static uint32_t vulkanImageUsageToBufferUsage(VkImageUsageFlags image_usage)
{
	uint32_t usage = 0;

#define CHECK_USAGE_BIT(vulkan_bit_, sysmem_bit_)  \
	if (image_usage & VK_IMAGE_USAGE_ ## vulkan_bit_ ## _BIT) \
		usage |= fuchsia::sysmem::vulkanUsage ## sysmem_bit_;

	CHECK_USAGE_BIT(TRANSFER_DST, TransferDst)
	CHECK_USAGE_BIT(TRANSFER_SRC, TransferSrc)
	CHECK_USAGE_BIT(SAMPLED,      Sampled)
	CHECK_USAGE_BIT(STORAGE,      Storage)

	CHECK_USAGE_BIT(COLOR_ATTACHMENT,         ColorAttachment)
	CHECK_USAGE_BIT(DEPTH_STENCIL_ATTACHMENT, StencilAttachment)
	CHECK_USAGE_BIT(TRANSIENT_ATTACHMENT,     TransientAttachment)
	CHECK_USAGE_BIT(INPUT_ATTACHMENT,         InputAttachment)

#undef CHECK_USAGE_BIT

	return usage;
}

// A list of supported Vulkan format values and their corresponding
// sysmem.PixelFormatType value, and bytes per pixel.
//
#define LIST_VULKAN_FORMATS(macro)          \
	macro(R8G8B8A8_UNORM,      R8G8B8A8, 4) \
	macro(R8G8B8A8_SRGB,       R8G8B8A8, 4) \
	macro(B8G8R8A8_UNORM,      BGRA32,   4) \
	macro(B8G8R8A8_SRGB,       BGRA32,   4) \
	macro(R5G6B5_UNORM_PACK16, RGB565,   3) \
	macro(B8G8R8_UNORM,        BGR24,    3) \
	macro(B8G8R8_SRGB,         BGR24,    3) \

// Convert a Vulkan format into the corresponding sysmem.PixelFormatType
// Returns PixelFormatType::INVALID is not recognized/supported.
fuchsia::sysmem::PixelFormatType vulkanFormatToPixelFormatType(VkFormat format)
{
#define CHECK_FORMAT(vulkan_, sysmem_, bytes_) \
	case VK_FORMAT_ ## vulkan_ : return fuchsia::sysmem::PixelFormatType:: sysmem_;

	switch (format) {
	LIST_VULKAN_FORMATS(CHECK_FORMAT)
		default: ;
	}

#undef CHECK_FORMAT
	TRACE("Unsupported Vulkan image format: %u", format);
	return fuchsia::sysmem::PixelFormatType::INVALID;
}

size_t vulkanFormatToPixelBytes(VkFormat format)
{
#define CHECK_PIXEL_BYTES(vulkan_, sysmem_, bytes_) \
	case VK_FORMAT_ ## vulkan_ : return bytes_;

	switch (format) {
	LIST_VULKAN_FORMATS(CHECK_PIXEL_BYTES)
	default:
		return 0;
	}
#undef CHECK_PIXEL_BYTES
}

// Convert a given Vulkan image create info struct into a sysmem ImageFormatConstraints struct
fuchsia::sysmem::ImageFormatConstraints vulkanImageCreateInfoToImageFormatConstraints(
		const VkImageCreateInfo* pImageInfo)
{
	VkFormat format = pImageInfo->format;

	return {
		.pixel_format = {
			.type = vulkanFormatToPixelFormatType(pImageInfo->format),
		},
		.color_spaces_count = 1,
		.color_space = {
			{
				{ .type = fuchsia::sysmem::ColorSpaceType::SRGB, }
			},
		},
		.min_coded_width = pImageInfo->extent.width,
		.max_coded_width = 0xffffffff,
		.min_coded_height = pImageInfo->extent.height,
		.max_coded_height = 0xffffffff,
		.min_bytes_per_row = pImageInfo->extent.width * vulkanFormatToPixelBytes(format),
		.max_bytes_per_row = 0xffffffff,
		.max_coded_width_times_coded_height = 0xffffffff,
		.layers = 1,
		.coded_width_divisor = 1,
		.coded_height_divisor = 1,
		.bytes_per_row_divisor = 1,
		.start_offset_divisor = 1,
		.display_width_divisor = 1,
		.display_height_divisor = 1,
	};
}

BufferCollectionFUCHSIA::BufferCollectionFUCHSIA(const VkBufferCollectionCreateInfoFUCHSIA* pCreateInfo, void* mem)
	: sysmemAllocator(FuchsiaSysmemAllocator::Get())
{
}

// static
size_t BufferCollectionFUCHSIA::ComputeRequiredAllocationSize(const VkBufferCollectionCreateInfoFUCHSIA* pCreateInfo)
{
	return 0;
}

void BufferCollectionFUCHSIA::destroy(const VkAllocationCallbacks* pAllocator)
{
}

VkResult BufferCollectionFUCHSIA::init(const VkBufferCollectionCreateInfoFUCHSIA* pCreateInfo)
{
	if (!sysmemAllocator.is_bound())
		return VK_ERROR_INITIALIZATION_FAILED;

	fuchsia::sysmem::BufferCollectionTokenSyncPtr token;
	if (pCreateInfo->collectionToken)
	{
		token.Bind(zx::channel(pCreateInfo->collectionToken));
	}
	else
	{
		zx_status_t status = sysmemAllocator->AllocateSharedCollection(token.NewRequest());
		if (status != ZX_OK)
		{
			TRACE("sysmem.Allocator.AllocateSharedCollection failed: %d", status);
			return VK_ERROR_INITIALIZATION_FAILED;
		}
	}
	zx_status_t status = sysmemAllocator->BindSharedCollection(std::move(token),
															   sysmemCollection.NewRequest());
	if (status != ZX_OK)
	{
		TRACE("sysmem.Allocator.BindSharedCollection failed: %d", status);
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	return VK_SUCCESS;
}

VkResult BufferCollectionFUCHSIA::setConstraints(const VkImageCreateInfo* pImageInfo)
{
	size_t min_size_bytes =
			pImageInfo->extent.width * pImageInfo->extent.height *
			vulkanFormatToPixelBytes(pImageInfo->format);

	fuchsia::sysmem::BufferCollectionConstraints constraints = {
		.usage = {
			.vulkan = vulkanImageUsageToBufferUsage(pImageInfo->usage),
		},
		.min_buffer_count = 1,
		.has_buffer_memory_constraints = true,
		.buffer_memory_constraints = {
			.min_size_bytes = min_size_bytes,
			.max_size_bytes = 0xffffffff,
			.inaccessible_domain_supported = true,
			.heap_permitted_count = 1,
			.heap_permitted = { fuchsia::sysmem::HeapType::SYSTEM_RAM },
		},
		.image_format_constraints_count = 1,
		.image_format_constraints = {
			vulkanImageCreateInfoToImageFormatConstraints(pImageInfo),
			{},
		},
	};

	sysmemCollection->SetConstraints(true, constraints);
	// TODO(digit): Error handling here?
	return VK_SUCCESS;
}

VkResult BufferCollectionFUCHSIA::setBufferConstraints(const VkBufferConstraintsInfoFUCHSIA* pBufferConstraintsInfo)
{
	ASSERT_MSG(false, "TODO(digit)");
	// TODO(digit): FIDL synchronous call to set buffer constraints on the collection.
	return VK_SUCCESS;
}

VkResult BufferCollectionFUCHSIA::getProperties(VkDevice device, VkBufferCollectionPropertiesFUCHSIA* pProperties) const
{
	fuchsia::sysmem::BufferCollectionInfo_2 info;
	zx_status_t status2;
	zx_status_t status = sysmemCollection->WaitForBuffersAllocated(&status2, &info);
	if (status != ZX_OK || status2 != ZX_OK)
	{
		TRACE("WaitForBuffersAllocated failed: %d %d", status, status2);
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	if (!info.settings.has_image_format_constraints)
		return VK_ERROR_INITIALIZATION_FAILED;

	pProperties->count = info.buffer_count;

	const VkPhysicalDeviceMemoryProperties& memoryProperties =
			vk::Cast(device)->getPhysicalDevice()->getMemoryProperties();

	// All SwiftShader memory types support this!
	pProperties->memoryTypeBits = (1U << memoryProperties.memoryTypeCount) - 1U;
	// TODO(digit): Verify that this is true? Are we introducing a new type of memory here?

	return VK_SUCCESS;
}

}  // namespace vk
