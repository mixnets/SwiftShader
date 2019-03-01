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

#include "XlibSurfaceKHR.hpp"

#include "Vulkan/VkDeviceMemory.hpp"

#include <string.h>

namespace vk {

XlibSurfaceKHR::XlibSurfaceKHR(const VkXlibSurfaceCreateInfoKHR *pCreateInfo, void *mem) :
		pDisplay(pCreateInfo->dpy),
		window(pCreateInfo->window)
{
	int screen = DefaultScreen(pDisplay);
	gc = libX11->XDefaultGC(pDisplay, screen);

	XVisualInfo xVisual;
	Status status = libX11->XMatchVisualInfo(pDisplay, screen, 32, TrueColor, &xVisual);
	bool match = (status != 0 && xVisual.blue_mask ==0xFF);
	Visual *visual = match ? xVisual.visual : libX11->XDefaultVisual(pDisplay, screen);

	XWindowAttributes attr;
	libX11->XGetWindowAttributes(pDisplay, window, &attr);

	int bytes_per_line = attr.width * 4;
	int bytes_per_image = attr.height * bytes_per_line;
	char* buffer = (char*)malloc(bytes_per_image);
	memset(buffer, 0, bytes_per_image);

	xImage = libX11->XCreateImage(pDisplay, visual, attr.depth, ZPixmap, 0, buffer, attr.width, attr.height, 32, bytes_per_line);

	if(!xImage)
	{
		free(buffer);
	}
}

void XlibSurfaceKHR::destroySurface(const VkAllocationCallbacks *pAllocator)
{
	if(!xImage)
	{
		free(xImage->data);
	}
}

size_t XlibSurfaceKHR::ComputeRequiredAllocationSize(const VkXlibSurfaceCreateInfoKHR *pCreateInfo)
{
	return 0;
}

void XlibSurfaceKHR::getSurfaceCapabilities(VkSurfaceCapabilitiesKHR *pSurfaceCapabilities) const
{
	SurfaceKHR::getSurfaceCapabilities(pSurfaceCapabilities);

	XWindowAttributes attr;
	libX11->XGetWindowAttributes(pDisplay, window, &attr);
	VkExtent2D extent = {static_cast<uint32_t>(attr.width), static_cast<uint32_t>(attr.height)};

	pSurfaceCapabilities->currentExtent = extent;
	pSurfaceCapabilities->minImageExtent = extent;
	pSurfaceCapabilities->maxImageExtent = extent;
}

void XlibSurfaceKHR::draw(VkImage image, VkDeviceMemory imageData)
{
	void* vkdata;
	vk::Cast(imageData)->map(0, vk::Cast(imageData)->getCommittedMemoryInBytes(), &vkdata);
	uint8_t	* vkData = (uint8_t *)vkdata;
	memcpy(xImage->data, vkData, static_cast<size_t>(vk::Cast(imageData)->getCommittedMemoryInBytes()));
	//memset(xImage->data, 255, xImage->bytes_per_line * xImage->height);

	XWindowAttributes attr;
	libX11->XGetWindowAttributes(pDisplay, window, &attr);

	libX11->XPutImage(pDisplay, window, gc, xImage, 0, 0, 0, 0, attr.width, attr.height);
}

}