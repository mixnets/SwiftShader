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

#include "VkSurfaceKHR.hpp"

#include <algorithm>

namespace vk
{

#ifdef VK_USE_PLATFORM_XLIB_KHR
	SurfaceKHR::SurfaceKHR(const VkXlibSurfaceCreateInfoKHR *pCreateInfo, void* mem) :
	pDisplay(pCreateInfo->dpy),
	window(pCreateInfo->window)
{

}
#endif

void SurfaceKHR::destroy(const VkAllocationCallbacks *pAllocator)
{

}

#ifdef VK_USE_PLATFORM_XLIB_KHR
size_t SurfaceKHR::ComputeRequiredAllocationSize(const VkXlibSurfaceCreateInfoKHR *pCreateInfo)
{
	return 0;
}
#endif

void SurfaceKHR::getSurfaceCapabilities(VkSurfaceCapabilitiesKHR *pSurfaceCapabilities) const
{
	pSurfaceCapabilities->minImageCount = 1;
	pSurfaceCapabilities->maxImageCount = 0;

	pSurfaceCapabilities->maxImageArrayLayers = 1;

#ifdef VK_USE_PLATFORM_XLIB_KHR
	XWindowAttributes attr;
	libX11->XGetWindowAttributes(pDisplay, window, &attr);
	VkExtent2D extent = { static_cast<uint32_t>(attr.width), static_cast<uint32_t>(attr.height) };
#else
	VkExtent2D extent = { 0, 0 };
#endif

	pSurfaceCapabilities->currentExtent = extent;
	pSurfaceCapabilities->minImageExtent = extent;
	pSurfaceCapabilities->maxImageExtent = extent;

	pSurfaceCapabilities->supportedTransforms = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	pSurfaceCapabilities->currentTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	pSurfaceCapabilities->supportedCompositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	pSurfaceCapabilities->supportedUsageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
}

uint32_t SurfaceKHR::getSurfaceFormatsCount() const
{
	return surfaceFormats.size();
}

VkResult SurfaceKHR::getSurfaceFormats(uint32_t *pSurfaceFormatCount, VkSurfaceFormatKHR *pSurfaceFormats) const
{
	uint32_t count = getSurfaceFormatsCount();

	uint32_t i;
	for(i = 0; i < std::min(*pSurfaceFormatCount, count); i++)
	{
		pSurfaceFormats[i] = surfaceFormats[i];
	}

	*pSurfaceFormatCount = i;

	if (*pSurfaceFormatCount < count)
	{
		return VK_INCOMPLETE;
	}

	return VK_SUCCESS;
}

uint32_t SurfaceKHR::getPresentModeCount() const
{
	return presentModes.size();
}


VkResult SurfaceKHR::getPresentModes(uint32_t *pPresentModeCount, VkPresentModeKHR *pPresentModes) const
{
	uint32_t count = getPresentModeCount();

	uint32_t i;
	for(i = 0; i < std::min(*pPresentModeCount, count); i++)
	{
		pPresentModes[i] = presentModes[i];
	}

	*pPresentModeCount = i;

	if (*pPresentModeCount < count)
	{
		return VK_INCOMPLETE;
	}

	return VK_SUCCESS;
}

}