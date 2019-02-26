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
#include "VkDestroy.h"

#include <algorithm>

namespace vk
{

SwapchainKHR::SwapchainKHR(const VkSwapchainCreateInfoKHR *pCreateInfo, void *mem) :
	createInfo(*pCreateInfo)
{
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.pQueueFamilyIndices = pCreateInfo->pQueueFamilyIndices;
	imageInfo.queueFamilyIndexCount = pCreateInfo->queueFamilyIndexCount;
	imageInfo.arrayLayers = pCreateInfo->imageArrayLayers;
	imageInfo.extent.height = pCreateInfo->imageExtent.height;
	imageInfo.extent.width = pCreateInfo->imageExtent.width;
	imageInfo.format = pCreateInfo->imageFormat;
	imageInfo.sharingMode = pCreateInfo->imageSharingMode;
	imageInfo.usage = pCreateInfo->imageUsage;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_GENERAL;

	for(imageCount = 0; imageCount < sizeof(images) / sizeof(images[0]); imageCount++)
	{
		if(vk::Image::Create(nullptr, &imageInfo, images + imageCount) != VK_SUCCESS)
		{
			break;
		}
	}
	currentImage = 0;
}

void SwapchainKHR::destroy(const VkAllocationCallbacks *pAllocator)
{
	for(uint32_t i = 0; i < getImageCount(); i++)
	{
		vk::destroy(images[i], pAllocator);
	}
}

size_t SwapchainKHR::ComputeRequiredAllocationSize(const VkSwapchainCreateInfoKHR *pCreateInfo)
{
	return 0;
}

uint32_t SwapchainKHR::getImageCount() const
{
	return imageCount;
}

VkResult SwapchainKHR::getImages(uint32_t *pSwapchainImageCount, VkImage *pSwapchainImages) const
{
	uint32_t count = getImageCount();

	uint32_t i;
	for (i = 0; i < std::min(*pSwapchainImageCount, count); i++)
	{
		pSwapchainImages[i] = images[i];
	}

	*pSwapchainImageCount = i;

	if (*pSwapchainImageCount < count)
	{
		return VK_INCOMPLETE;
	}

	return VK_SUCCESS;
}

}