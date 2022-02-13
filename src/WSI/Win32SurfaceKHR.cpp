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

#include "System/Debug.hpp"
#include "Vulkan/VkDeviceMemory.hpp"

#include <string.h>

namespace {
VkResult getWindowSize(HWND hwnd, VkExtent2D &windowSize)
{
	RECT clientRect = {};
	if(!IsWindow(hwnd) || !GetClientRect(hwnd, &clientRect))
	{
		windowSize = { 0, 0 };
		return VK_ERROR_SURFACE_LOST_KHR;
	}

	windowSize = { static_cast<uint32_t>(clientRect.right - clientRect.left),
		           static_cast<uint32_t>(clientRect.bottom - clientRect.top) };

	return VK_SUCCESS;
}
}  // namespace

namespace vk {

Win32SurfaceKHR::Win32SurfaceKHR(const VkWin32SurfaceCreateInfoKHR *pCreateInfo, void *mem)
    : hwnd(pCreateInfo->hwnd)
{
	ASSERT(IsWindow(hwnd) == TRUE);
	windowContext = GetDC(hwnd);
	bitmapContext = CreateCompatibleDC(windowContext);
}

void Win32SurfaceKHR::destroySurface(const VkAllocationCallbacks *pAllocator)
{
	ReleaseDC(hwnd, windowContext);
	DeleteDC(bitmapContext);
}

size_t Win32SurfaceKHR::ComputeRequiredAllocationSize(const VkWin32SurfaceCreateInfoKHR *pCreateInfo)
{
	return 0;
}

VkResult Win32SurfaceKHR::getSurfaceCapabilities(VkSurfaceCapabilitiesKHR *pSurfaceCapabilities) const
{
	setCommonSurfaceCapabilities(pSurfaceCapabilities);

	VkExtent2D extent;
	VkResult result = getWindowSize(hwnd, extent);
	pSurfaceCapabilities->currentExtent = extent;
	pSurfaceCapabilities->minImageExtent = extent;
	pSurfaceCapabilities->maxImageExtent = extent;
	return result;
}

void* Win32SurfaceKHR::allocateImageMemory(PresentImage *image, const VkMemoryAllocateInfo &allocateInfo)
{
	auto& bitmap = bitmaps[image];

	int stride = image->getImage()->rowPitchBytes(VK_IMAGE_ASPECT_COLOR_BIT, 0);
	int bytesPerPixel = static_cast<int>(image->getImage()->getFormat(VK_IMAGE_ASPECT_COLOR_BIT).bytes());
	int width = stride / bytesPerPixel;
	int height = allocateInfo.allocationSize / stride;

	BITMAPINFO bitmapInfo = {};
	bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFO);
	bitmapInfo.bmiHeader.biBitCount = bytesPerPixel * 8;
	bitmapInfo.bmiHeader.biPlanes = 1;
	bitmapInfo.bmiHeader.biHeight = -static_cast<LONG>(height);  // Negative for top-down DIB, origin in upper-left corner
	bitmapInfo.bmiHeader.biWidth = width;
	bitmapInfo.bmiHeader.biCompression = BI_RGB;

	void* framebuffer = nullptr;
	bitmap = CreateDIBSection(bitmapContext, &bitmapInfo, DIB_RGB_COLORS, &framebuffer, 0, 0);
	return framebuffer;
}

void Win32SurfaceKHR::releaseImageMemory(PresentImage *image)
{
	auto it = bitmaps.find(image);
	assert(it != bitmaps.end());
	auto& bitmap = it->second;
	DeleteObject(bitmap);
	bitmaps.erase(it);
}
void Win32SurfaceKHR::attachImage(PresentImage *image)
{
	// Nothing to do here, the current implementation based on GDI blits on
	// present instead of associating the image with the surface.
}

void Win32SurfaceKHR::detachImage(PresentImage *image)
{
	// Nothing to do here, the current implementation based on GDI blits on
	// present instead of associating the image with the surface.
}

VkResult Win32SurfaceKHR::present(PresentImage *image)
{
	const VkExtent3D &extent = image->getImage()->getExtent();
	auto it = bitmaps.find(image);
	assert(it != bitmaps.end());
	auto& bitmap = it->second;
	SelectObject(bitmapContext, bitmap);
	StretchBlt(windowContext, 0, 0, extent.width, extent.height, bitmapContext, 0, 0, extent.width, extent.height, SRCCOPY);
	SelectObject(bitmapContext, NULL);
	return VK_SUCCESS;
}

}  // namespace vk