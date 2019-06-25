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

#include "Win32SurfaceKHR.hpp"

#include "Vulkan/VkDeviceMemory.hpp"
#include "Vulkan/VkDebug.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <string.h>

namespace vk {

Win32SurfaceKHR::Win32SurfaceKHR(const VkWin32SurfaceCreateInfoKHR *pCreateInfo, void *mem) :
		hinstance(pCreateInfo->hinstance),
		hwnd(pCreateInfo->hwnd)
{
	ASSERT(IsWindow(hwnd) == TRUE);

	RECT clientRect = {};
	BOOL status = GetClientRect(hwnd, &clientRect);
	ASSERT(status != 0);

	windowContext = GetDC(hwnd);
	bitmapContext = CreateCompatibleDC(windowContext);

	BITMAPINFO bitmapInfo = {};
	bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFO);
	bitmapInfo.bmiHeader.biBitCount = 32;
	bitmapInfo.bmiHeader.biPlanes = 1;
	bitmapInfo.bmiHeader.biHeight = clientRect.top - clientRect.bottom;
	bitmapInfo.bmiHeader.biWidth = clientRect.right - clientRect.left;
	bitmapInfo.bmiHeader.biCompression = BI_RGB;

	bitmap = CreateDIBSection(bitmapContext, &bitmapInfo, DIB_RGB_COLORS, &framebuffer, 0, 0);
	ASSERT(bitmap != NULL);
	SelectObject(bitmapContext, bitmap);
}

void Win32SurfaceKHR::destroySurface(const VkAllocationCallbacks *pAllocator)
{
	SelectObject(bitmapContext, NULL);
	DeleteObject(bitmap);
	ReleaseDC(hwnd, windowContext);
	DeleteDC(bitmapContext);
}

size_t Win32SurfaceKHR::ComputeRequiredAllocationSize(const VkWin32SurfaceCreateInfoKHR *pCreateInfo)
{
	return 0;
}

void Win32SurfaceKHR::getSurfaceCapabilities(VkSurfaceCapabilitiesKHR *pSurfaceCapabilities) const
{
	SurfaceKHR::getSurfaceCapabilities(pSurfaceCapabilities);

	RECT clientRect = {};
	BOOL status = GetClientRect(hwnd, &clientRect);
	ASSERT(status != 0);

	int windowWidth = clientRect.right - clientRect.left;
	int windowHeight = clientRect.bottom - clientRect.top;

	VkExtent2D extent = {static_cast<uint32_t>(windowWidth), static_cast<uint32_t>(windowHeight)};
	pSurfaceCapabilities->currentExtent = extent;
	pSurfaceCapabilities->minImageExtent = extent;
	pSurfaceCapabilities->maxImageExtent = extent;
}

void Win32SurfaceKHR::attachImage(PresentImage* image)
{
	// Nothing to do here, the current implementation based on GDI blits on
	// present instead of associating the image with the surface.
}

void Win32SurfaceKHR::detachImage(PresentImage* image)
{
	// Nothing to do here, the current implementation based on GDI blits on
	// present instead of associating the image with the surface.
}

void Win32SurfaceKHR::present(PresentImage* image)
{
	if(!framebuffer)
	{
		return;
	}

	VkExtent3D extent = image->getImage()->getMipLevelExtent(VK_IMAGE_ASPECT_COLOR_BIT, 0);

	uint32_t *im = (uint32_t*)image->getImageMemory()->getOffsetPointer(0);
	uint32_t *fb = (uint32_t*)framebuffer;

	for(uint32_t y = 0; y < extent.height; y++)
	{
		for(uint32_t x = 0; x < extent.width; x++)
		{
			*fb = (*im & 0xFF00FF00) | ((*im & 0x000000FF) << 16) | ((*im & 0x00FF0000) >> 16);

			fb++;
			im++;
		}
	}

	StretchBlt(windowContext, 0, 0, extent.width, extent.height, bitmapContext, 0, 0, extent.width, extent.height, SRCCOPY);
}

}