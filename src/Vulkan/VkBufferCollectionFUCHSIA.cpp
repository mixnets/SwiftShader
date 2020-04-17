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

#if VK_USE_PLATFORM_FUCHSIA

#	include "VkBufferCollectionFUCHSIA.hpp"

#	include "VkConfig.h"
#	include "VkDevice.hpp"
#	include "VkPhysicalDevice.hpp"
#	include "VkGetProcAddress.h"

#	include "System/Debug.hpp"

#	include <lib/zx/channel.h>
#	include <zircon/syscalls.h>

namespace vk {

// Technical note: Since there is no official spec for the extension, most of
// of the code here was inspired from the one found in:
//
//   $AOSP/device/generic/goldfish-opengl/system/vulkan_enc/ResourceTracker.cpp
//

// Global connection to the fuchsia.sysmem.Allocator service.

// vk::icdFuchsiaServiceConnect should be set during library initialization
// to set the service connection callback pointer. Normally this is performed
// by the Vulkan loader calling vk_icdInitializeConnectToServiceCallback early on.

// Use FuchsiaSysmemAllocator::get() to retrieve a reference to it.
// Use is_bound() on the result to check that the connection was properly
// established (this would typically fail if the service is not listed in
// the component's manifest of the current process).
class FuchsiaSysmemAllocator
{
public:
	static fuchsia::sysmem::AllocatorSyncPtr &get()
	{
		static FuchsiaSysmemAllocator sInstance;
		return sInstance.sysmemAllocator;
	}

private:
	FuchsiaSysmemAllocator()
	{
		if(!vk::icdFuchsiaServiceConnectCallback)
		{
			TRACE("No callback pointer to connect to Fuchsia services!");
			return;
		}

		zx::channel remote, local;
		zx_status_t status = zx::channel::create(0, &remote, &local);
		if(status != ZX_OK)
		{
			TRACE("zx::channel::create() failed with %d", status);
			return;
		}

		VkResult result = (*vk::icdFuchsiaServiceConnectCallback)("/svc/fuchsia.sysmem.Allocator", remote.release());
		if(result != VK_SUCCESS)
		{
			TRACE("Cannot connect to fuchsia.sysmem.Allocator service: %d", result);
			return;
		}
		sysmemAllocator.Bind(std::move(local));
	}

	fuchsia::sysmem::AllocatorSyncPtr sysmemAllocator;

	static PFN_vkConnectToServiceAddr serviceConnect;
};

// static
PFN_vkConnectToServiceAddr FuchsiaSysmemAllocator::serviceConnect = nullptr;

// Convert a Vulkan image usage bitmask into the equivalent sysmem.BufferUsage.vulkan value.
static uint32_t vulkanImageUsageToBufferUsage(VkImageUsageFlags image_usage)
{
	uint32_t usage = 0;

#	define CHECK_USAGE_BIT(vulkan_bit_, sysmem_bit_)        \
		if(image_usage & VK_IMAGE_USAGE_##vulkan_bit_##_BIT) \
			usage |= fuchsia::sysmem::vulkanUsage##sysmem_bit_;

	CHECK_USAGE_BIT(TRANSFER_DST, TransferDst)
	CHECK_USAGE_BIT(TRANSFER_SRC, TransferSrc)
	CHECK_USAGE_BIT(SAMPLED, Sampled)
	CHECK_USAGE_BIT(STORAGE, Storage)

	CHECK_USAGE_BIT(COLOR_ATTACHMENT, ColorAttachment)
	CHECK_USAGE_BIT(DEPTH_STENCIL_ATTACHMENT, StencilAttachment)
	CHECK_USAGE_BIT(TRANSIENT_ATTACHMENT, TransientAttachment)
	CHECK_USAGE_BIT(INPUT_ATTACHMENT, InputAttachment)

#	undef CHECK_USAGE_BIT

	return usage;
}

// A list of supported Vulkan format values and their corresponding
// sysmem.PixelFormatType value, and bytes per pixel.
//
// clang-format off
#	define LIST_VULKAN_FORMATS(macro)           \
		macro(R8G8B8A8_UNORM,      R8G8B8A8, 4) \
		macro(R8G8B8A8_SRGB,       R8G8B8A8, 4) \
		macro(B8G8R8A8_UNORM,      BGRA32,   4) \
		macro(B8G8R8A8_SRGB,       BGRA32,   4) \
		macro(R5G6B5_UNORM_PACK16, RGB565,   3) \
		macro(B8G8R8_UNORM,        BGR24,    3) \
		macro(B8G8R8_SRGB,         BGR24,    3)

// clang-format on

// Convert a Vulkan format into the corresponding sysmem.PixelFormatType
// Returns PixelFormatType::INVALID is not recognized/supported.
fuchsia::sysmem::PixelFormatType vulkanFormatToPixelFormatType(VkFormat format)
{
#	define CHECK_FORMAT(vulkan_, sysmem_, bytes_) \
		case VK_FORMAT_##vulkan_: return fuchsia::sysmem::PixelFormatType::sysmem_;

	switch(format)
	{
		LIST_VULKAN_FORMATS(CHECK_FORMAT)
		default:;
	}

#	undef CHECK_FORMAT
	TRACE("Unsupported Vulkan image format: %u", format);
	return fuchsia::sysmem::PixelFormatType::INVALID;
}

size_t vulkanFormatToPixelBytes(VkFormat format)
{
#	define CHECK_PIXEL_BYTES(vulkan_, sysmem_, bytes_) \
		case VK_FORMAT_##vulkan_: return bytes_;

	switch(format)
	{
		LIST_VULKAN_FORMATS(CHECK_PIXEL_BYTES)
		default:
			return 0;
	}
#	undef CHECK_PIXEL_BYTES
}

// Convert a given Vulkan image create info struct into a sysmem ImageFormatConstraints struct
fuchsia::sysmem::ImageFormatConstraints vulkanImageCreateInfoToImageFormatConstraints(
    const VkImageCreateInfo *pImageInfo)
{
	VkFormat format = pImageInfo->format;

	// IMPORTANT NOTE: SwiftShader currently assumes that a VkImage's stride is _always_
	// exactly equal to the |bytesPerRow| value below. This is not a problem in the typical
	// case where the swapchain image buffers are allocated by SwiftShader (see examples
	// under src/WSI/).
	//
	// However, this does not work in the case of Fuchsia, where swapchain buffers and images
	// are allocated very differently, and by a Vulkan layer, which redirects functions like
	// vkGetPhysicalDeviceSurfaceCapabilitiesKHR() or vkCreateSwapchainKHR() entirely, including
	// vkAcquireNextImageKHR() and vkPresentImageKHR().
	//
	// In a nutshell, there are only two ways to deal with that at the moment:
	//
	//  - Set ImageFormatConstraints::max_bytes_per_row to |bytesPerRow| to ensure that
	//    all allocated swapchain image buffers have the "right" stride as expected by
	//    SwiftShader. This also means that buffer allocation may fail if the display
	//    implementation (i.e. Scenic view or Display controller framebuffer) requires a
	//    different stride value.
	//
	//  - Set ImageFormatConstraints::max_bytes_per_row to UINT32_MAX to allow buffer
	//    allocation with larger stride values. In this case, rendering with SwiftShader
	//    will be incorrect due to misaligned image row offsets.
	//
	// Note that in the Win32 swapchain implementation, SwiftShader deals with stride differences
	// in vkPresentImageKHR() (by blitting, the SwiftShader-allocated imnage buffer into the
	// window's DBI buffer, which may have a different stride).
	//
	// Unfortunately, this technique cannot be used here because vkPresentImageKHR() is entirely
	// redirected to the image pipe Vulkan layer which assumes "proper" buffer allocation.
	//
	size_t bytesPerRow = vk::Format(format).pitchB(pImageInfo->extent.width, 0, true);

	return {
		.pixel_format = {
		    .type = vulkanFormatToPixelFormatType(pImageInfo->format),
		},
		.color_spaces_count = 1,
		.color_space = {
		    { {
		        .type = fuchsia::sysmem::ColorSpaceType::SRGB,
		    } },
		},
		.min_coded_width = 0,
		.max_coded_width = 0,
		.min_coded_height = 0,
		.max_coded_height = 0,
		.min_bytes_per_row = 0,
		.max_bytes_per_row = 0,
		.max_coded_width_times_coded_height = 0xffffffff,
		.layers = 1,
		.coded_width_divisor = 1,
		.coded_height_divisor = 1,
		.bytes_per_row_divisor = 1,
		.start_offset_divisor = 1,
		.display_width_divisor = 1,
		.display_height_divisor = 1,
		.required_min_coded_width = pImageInfo->extent.width,
		.required_max_coded_width = 0,  // NOTE: Do not use 0xffffffff here or allocation will fal.
		.required_min_coded_height = pImageInfo->extent.height,
		.required_max_coded_height = 0,  // NOTE: Do not use 0xffffffff here or allocation will fail.
		.required_min_bytes_per_row = static_cast<uint32_t>(bytesPerRow),
		// See IMPORT NOTE above regarding the max value below.
		.required_max_bytes_per_row = static_cast<uint32_t>(bytesPerRow),
	};
}

BufferCollectionFUCHSIA::BufferCollectionFUCHSIA(const VkBufferCollectionCreateInfoFUCHSIA *pCreateInfo, void *mem)
    : sysmemAllocator(FuchsiaSysmemAllocator::get())
{
}

// static
size_t BufferCollectionFUCHSIA::ComputeRequiredAllocationSize(const VkBufferCollectionCreateInfoFUCHSIA *pCreateInfo)
{
	return 0;
}

void BufferCollectionFUCHSIA::destroy(const VkAllocationCallbacks *pAllocator)
{
}

VkResult BufferCollectionFUCHSIA::init(const VkBufferCollectionCreateInfoFUCHSIA *pCreateInfo)
{
	if(!sysmemAllocator.is_bound())
		return VK_ERROR_INITIALIZATION_FAILED;

	fuchsia::sysmem::BufferCollectionTokenSyncPtr token;
	if(pCreateInfo->collectionToken)
	{
		token.Bind(zx::channel(pCreateInfo->collectionToken));
	}
	else
	{
		zx_status_t status = sysmemAllocator->AllocateSharedCollection(token.NewRequest());
		if(status != ZX_OK)
		{
			TRACE("sysmem.Allocator.AllocateSharedCollection failed: %d", status);
			return VK_ERROR_INITIALIZATION_FAILED;
		}
	}
	zx_status_t status = sysmemAllocator->BindSharedCollection(std::move(token),
	                                                           sysmemCollection.NewRequest());
	if(status != ZX_OK)
	{
		TRACE("sysmem.Allocator.BindSharedCollection failed: %d", status);
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	return VK_SUCCESS;
}

VkResult BufferCollectionFUCHSIA::setConstraints(const VkImageCreateInfo *pImageInfo)
{
	size_t minSizeBytes =
	    pImageInfo->extent.width * pImageInfo->extent.height *
	    vulkanFormatToPixelBytes(pImageInfo->format);

	fuchsia::sysmem::BufferCollectionConstraints constraints = {
		.usage = {
		    .cpu = fuchsia::sysmem::cpuUsageRead | fuchsia::sysmem::cpuUsageWrite,
		    .vulkan = vulkanImageUsageToBufferUsage(pImageInfo->usage),
		},
		.min_buffer_count = 1,
		.has_buffer_memory_constraints = true,
		.buffer_memory_constraints = {
		    .min_size_bytes = static_cast<uint32_t>(minSizeBytes),
		    .max_size_bytes = 0xffffffff,
		    .inaccessible_domain_supported = false,
		    .heap_permitted_count = 0,
		},
		.image_format_constraints_count = 1,
		.image_format_constraints = {
		    vulkanImageCreateInfoToImageFormatConstraints(pImageInfo),
		    {},
		},
	};

	zx_status_t status = sysmemCollection->SetConstraints(true, constraints);
	if(status != ZX_OK)
	{
		TRACE("sysmem.BufferCollection.setConstraints() failed: %d", status);
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	return VK_SUCCESS;
}

VkResult BufferCollectionFUCHSIA::setBufferConstraints(const VkBufferConstraintsInfoFUCHSIA *pBufferConstraintsInfo)
{
	// NOTE: This call is never called by any code in the Fuchsia source tree
	// so it's difficult to know what it should do here.
	ASSERT_MSG(false, "TODO(digit)");
	// TODO(digit): FIDL synchronous call to set buffer constraints on the collection.
	return VK_SUCCESS;
}

VkResult BufferCollectionFUCHSIA::getInfo2() const
{
	if(hasInfo2)
		return VK_SUCCESS;

	zx_status_t status2;
	zx_status_t status = sysmemCollection->WaitForBuffersAllocated(&status2, &info2);
	if(status != ZX_OK || status2 != ZX_OK)
	{
		TRACE("WaitForBuffersAllocated failed: %d %d", status, status2);
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	if(!info2.settings.has_image_format_constraints)
	{
		TRACE("Settings has_image_format_constraints==false!");
		return VK_ERROR_INITIALIZATION_FAILED;
	}

	hasInfo2 = true;
	return VK_SUCCESS;
}

VkResult BufferCollectionFUCHSIA::takeBufferVmo(uint32_t index,
                                                zx_handle_t *vmoHandle, uint64_t *vmoUsableStart)
{
	VkResult result = getInfo2();
	if(result != VK_SUCCESS)
		return result;

	if(index >= info2.buffer_count)
	{
		return VK_ERROR_INITIALIZATION_FAILED;
	}

	*vmoHandle = info2.buffers[index].vmo.release();
	*vmoUsableStart = info2.buffers[index].vmo_usable_start;
	return VK_SUCCESS;
}

VkResult BufferCollectionFUCHSIA::getProperties(VkDevice device, VkBufferCollectionPropertiesFUCHSIA *pProperties) const
{
	VkResult result = getInfo2();
	if(!result)
		return result;

	pProperties->count = info2.buffer_count;

	const VkPhysicalDeviceMemoryProperties &memoryProperties =
	    vk::Cast(device)->getPhysicalDevice()->getMemoryProperties();

	// All SwiftShader memory types support this!
	pProperties->memoryTypeBits = (1U << memoryProperties.memoryTypeCount) - 1U;
	// TODO(digit): Verify that this is true? Are we introducing a new type of memory here?

	return VK_SUCCESS;
}

}  // namespace vk

#endif  // VK_USE_PLATFORM_FUCHSIA
