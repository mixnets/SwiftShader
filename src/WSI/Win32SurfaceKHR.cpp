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
	//int screen = DefaultScreen(pDisplay);
	//gc = libX11->XDefaultGC(pDisplay, screen);

	//XVisualInfo xVisual;
	//Status status = libX11->XMatchVisualInfo(pDisplay, screen, 32, TrueColor, &xVisual);
	//bool match = (status != 0 && xVisual.blue_mask ==0xFF);
	//visual = match ? xVisual.visual : libX11->XDefaultVisual(pDisplay, screen);

	//UNIMPLEMENTED();

	SetLastError(0);
	DWORD error = GetLastError();
	assert(error == 0);

	SetLastError(0);
	DWORD error2 = GetLastError();
	assert(error2 == 0);

	RECT client;
	BOOL status = GetClientRect(hwnd, &client);

	/*VkExtent3D extent = vk::Cast(image->image)->getMipLevelExtent(0);

	int bytes_per_line = vk::Cast(image->image)->rowPitchBytes(VK_IMAGE_ASPECT_COLOR_BIT, 0);
	char* buffer = static_cast<char*>(vk::Cast(image->imageMemory)->getOffsetPointer(0));*/

	//XImage* xImage = libX11->XCreateImage(pDisplay, visual, attr.depth, ZPixmap, 0, buffer, extent.width, extent.height, 32, bytes_per_line);

	//imageMap[image] = xImage;
	DWORD error22 = GetLastError();
	assert(error22 == 0);
		windowContext = GetDC(hwnd);
		DWORD error3 = GetLastError();
	assert(error3 == 0);
		bitmapContext = CreateCompatibleDC(windowContext);
		DWORD error4 = GetLastError();
	assert(error4 == 0);
		BITMAPINFO bitmapInfo = {};
		bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFO);
		bitmapInfo.bmiHeader.biBitCount = 32;
		bitmapInfo.bmiHeader.biPlanes = 1;
		bitmapInfo.bmiHeader.biHeight = client.top - client.bottom;// (LONG)extent.height;
		bitmapInfo.bmiHeader.biWidth = client.right - client.left;//extent.width;
		bitmapInfo.bmiHeader.biCompression = BI_RGB;

		HBITMAP bitmap = CreateDIBSection(bitmapContext, &bitmapInfo, DIB_RGB_COLORS, &framebuffer, 0, 0);
		SelectObject(bitmapContext, bitmap);
	DWORD error5 = GetLastError();
	assert(error5 == 0);
		RECT bounds = {};
		GetClientRect(hwnd, &bounds);
	DWORD error6 = GetLastError();
	assert(error6 == 0);
		ClientToScreen(hwnd, (POINT*)&bounds);
		ClientToScreen(hwnd, (POINT*)&bounds + 1);
		DWORD error7 = GetLastError();
	assert(error7 == 0);
	//	imageMap[image] = framebuffer;

	//	DWORD error = GetLastError();
	//assert(error == 0);
}

void Win32SurfaceKHR::destroySurface(const VkAllocationCallbacks *pAllocator)
{
	UNIMPLEMENTED();
}

size_t Win32SurfaceKHR::ComputeRequiredAllocationSize(const VkWin32SurfaceCreateInfoKHR *pCreateInfo)
{
	//UNIMPLEMENTED();
	return 0;
}

void Win32SurfaceKHR::getSurfaceCapabilities(VkSurfaceCapabilitiesKHR *pSurfaceCapabilities) const
{
	SetLastError(0);
	DWORD error = GetLastError();
	assert(error == 0);

	SurfaceKHR::getSurfaceCapabilities(pSurfaceCapabilities);

	DWORD error2 = GetLastError();
	assert(error2 == 0);

	//XWindowAttributes attr;
	//libX11->XGetWindowAttributes(pDisplay, window, &attr);
	//VkExtent2D extent = {static_cast<uint32_t>(attr.width), static_cast<uint32_t>(attr.height)};

	RECT client;
	BOOL status = GetClientRect(hwnd, &client);

	DWORD error3 = GetLastError();
	assert(error3 == 0);

	if(status == 0)
	{
		UNIMPLEMENTED();
	///	return error(EGL_BAD_NATIVE_WINDOW, false);
	}

	int windowWidth = client.right - client.left;
	int windowHeight = client.bottom - client.top;

	VkExtent2D extent = {static_cast<uint32_t>(windowWidth), static_cast<uint32_t>(windowHeight)};
//	UNIMPLEMENTED();

//	pSurfaceCapabilities = {};
	pSurfaceCapabilities->currentExtent = extent;
	pSurfaceCapabilities->minImageExtent = extent;
	pSurfaceCapabilities->maxImageExtent = extent;

	DWORD error4 = GetLastError();
	assert(error4 == 0);
}

void Win32SurfaceKHR::attachImage(PresentImage* image)
{
	//XWindowAttributes attr;
	//libX11->XGetWindowAttributes(pDisplay, window, &attr);


}

void Win32SurfaceKHR::detachImage(PresentImage* image)
{
	//auto it = imageMap.find(image);
	//if(it != imageMap.end())
	//{
	//	XImage* xImage = it->second;
	//	xImage->data = nullptr; // the XImage does not actually own the buffer
	//	XDestroyImage(xImage);
	//	imageMap.erase(image);
	//}

	UNIMPLEMENTED();
}

void Win32SurfaceKHR::present(PresentImage* image)
{
//	auto it = imageMap.find(image);
//	if(it != imageMap.end())
	{
	//	void* fb = it->second;

		if(framebuffer)
		{
			VkExtent3D extent = vk::Cast(image->image)->getMipLevelExtent(0);
	//		libX11->XPutImage(pDisplay, window, gc, xImage, 0, 0, 0, 0, extent.width, extent.height);

		//	memcpy(framebuffer, vk::Cast(image->imageMemory)->getOffsetPointer(0), extent.width * extent.height * 4);
		//	memset(framebuffer, -1, 1280*800*4);

			uint32_t *im = (uint32_t*)vk::Cast(image->imageMemory)->getOffsetPointer(0);
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

		DWORD error = GetLastError();
	assert(error == 0);
		}
	}
}

}