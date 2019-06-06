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

#include "VkSwapchainKHR.hpp"

#include "Vulkan/VkDeviceMemory.hpp"
#include "Vulkan/VkFence.hpp"
#include "Vulkan/VkImage.hpp"
#include "Vulkan/VkSemaphore.hpp"

#include <algorithm>

namespace vk
{

SwapchainKHR::SwapchainKHR(const VkSwapchainCreateInfoKHR *pCreateInfo, void *mem) :
	surface(Cast(pCreateInfo->surface)),
	retired(false)
{
	images.resize(pCreateInfo->minImageCount);
	resetImages();
}

void SwapchainKHR::destroy(const VkAllocationCallbacks *pAllocator)
{
	for(auto& currentImage : images)
	{
		if (currentImage.exists())
		{
			surface->detachImage(&currentImage);
			currentImage.clear();
		}
	}

	if(!retired)
	{
		surface->disassociateSwapchain();
	}
}

size_t SwapchainKHR::ComputeRequiredAllocationSize(const VkSwapchainCreateInfoKHR *pCreateInfo)
{
	return 0;
}

void SwapchainKHR::retire()
{
	if(!retired)
	{
		retired = true;
		surface->disassociateSwapchain();

		for(auto& currentImage : images)
		{
			if(currentImage.isAvailable())
			{
				surface->detachImage(&currentImage);
				currentImage.clear();
			}
		}
	}
}

void SwapchainKHR::resetImages()
{
	for(auto& currentImage : images)
	{
		currentImage.clear();
	}
}

VkResult SwapchainKHR::createImages(VkDevice device, const VkSwapchainCreateInfoKHR *pCreateInfo)
{
	resetImages();

	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;

	if(pCreateInfo->flags & VK_SWAPCHAIN_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT_KHR)
	{
		imageInfo.flags |= VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT;
	}

	if(pCreateInfo->flags & VK_SWAPCHAIN_CREATE_PROTECTED_BIT_KHR)
	{
		imageInfo.flags |= VK_IMAGE_CREATE_PROTECTED_BIT;
	}

	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.format = pCreateInfo->imageFormat;
	imageInfo.extent.height = pCreateInfo->imageExtent.height;
	imageInfo.extent.width = pCreateInfo->imageExtent.width;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = pCreateInfo->imageArrayLayers;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.usage = pCreateInfo->imageUsage;
	imageInfo.sharingMode = pCreateInfo->imageSharingMode;
	imageInfo.pQueueFamilyIndices = pCreateInfo->pQueueFamilyIndices;
	imageInfo.queueFamilyIndexCount = pCreateInfo->queueFamilyIndexCount;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_GENERAL;

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = 0;
	allocInfo.memoryTypeIndex = 0;

	VkResult status;
	for(auto& currentImage : images)
	{
		status = currentImage.allocateImage(device, imageInfo);
		if(status != VK_SUCCESS)
		{
			return status;
		}

		allocInfo.allocationSize = currentImage.getImage()->getMemoryRequirements().size;

		status = currentImage.allocateAndBindImageMemory(device, allocInfo);
		if(status != VK_SUCCESS)
		{
			return status;
		}

		surface->attachImage(&currentImage);
	}

	return VK_SUCCESS;
}

uint32_t SwapchainKHR::getImageCount() const
{
	return static_cast<uint32_t >(images.size());
}

VkResult SwapchainKHR::getImages(uint32_t *pSwapchainImageCount, VkImage *pSwapchainImages) const
{
	uint32_t count = getImageCount();

	uint32_t i;
	for (i = 0; i < std::min(*pSwapchainImageCount, count); i++)
	{
		pSwapchainImages[i] = images[i].asVkImage();
	}

	*pSwapchainImageCount = i;

	if (*pSwapchainImageCount < count)
	{
		return VK_INCOMPLETE;
	}

	return VK_SUCCESS;
}

VkResult SwapchainKHR::getNextImage(uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t *pImageIndex)
{
	for(uint32_t i = 0; i < getImageCount(); i++)
	{
		PresentImage& currentImage = images[i];
		if(currentImage.isAvailable())
		{
			currentImage.setStatus(DRAWING);
			*pImageIndex = i;

			if(semaphore)
			{
				vk::Cast(semaphore)->signal();
			}

			if(fence)
			{
				vk::Cast(fence)->complete();
			}

			return VK_SUCCESS;
		}
	}

	return VK_NOT_READY;
}

void SwapchainKHR::present(uint32_t index)
{
	auto & image = images[index];
	image.setStatus(PRESENTING);
	surface->present(&image);
	image.setStatus(AVAILABLE);

	if(retired)
	{
		surface->detachImage(&image);
		image.clear();
	}
}

}