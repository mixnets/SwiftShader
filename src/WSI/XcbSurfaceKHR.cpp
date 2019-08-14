// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//	http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "XcbSurfaceKHR.hpp"

#include "Vulkan/VkDeviceMemory.hpp"
#include "Vulkan/VkImage.hpp"

#include "libX11.hpp"

namespace vk {

XcbSurfaceKHR::XcbSurfaceKHR(const VkXcbSurfaceCreateInfoKHR *pCreateInfo, void *mem) :
	connection(pCreateInfo->connection),
	window(pCreateInfo->window)
{
}

void XcbSurfaceKHR::destroySurface(const VkAllocationCallbacks *pAllocator)
{

}

size_t XcbSurfaceKHR::ComputeRequiredAllocationSize(const VkXcbSurfaceCreateInfoKHR *pCreateInfo)
{
	return 0;
}

void XcbSurfaceKHR::getSurfaceCapabilities(VkSurfaceCapabilitiesKHR *pSurfaceCapabilities) const
{
	SurfaceKHR::getSurfaceCapabilities(pSurfaceCapabilities);

	auto geom = libX11->xcb_get_geometry_reply(connection, libX11->xcb_get_geometry(connection, window), nullptr);
	VkExtent2D extent = {static_cast<uint32_t>(geom->width), static_cast<uint32_t>(geom->height)};
	free(geom);

	pSurfaceCapabilities->currentExtent = extent;
	pSurfaceCapabilities->minImageExtent = extent;
	pSurfaceCapabilities->maxImageExtent = extent;
}

void XcbSurfaceKHR::attachImage(PresentImage* image)
{
 	auto gc = libX11->xcb_generate_id(connection);

	uint32_t values[2] = {0, 0xffffffff};
 	libX11->xcb_create_gc(connection, gc, window, XCB_GC_FOREGROUND | XCB_GC_BACKGROUND, values);

	graphicsContexts[image] = gc;
}

void XcbSurfaceKHR::detachImage(PresentImage* image)
{
	auto it = graphicsContexts.find(image);
	if(it != graphicsContexts.end())
	{
		libX11->xcb_free_gc(connection, it->second);
		graphicsContexts.erase(image);
	}
}

void XcbSurfaceKHR::present(PresentImage* image)
{
	auto it = graphicsContexts.find(image);
	if(it != graphicsContexts.end())
	{
		// TODO: Convert image if not RGB888.
		VkExtent3D extent = image->getImage()->getMipLevelExtent(VK_IMAGE_ASPECT_COLOR_BIT, 0);
		int stride = image->getImage()->rowPitchBytes(VK_IMAGE_ASPECT_COLOR_BIT, 0);
		auto buffer = reinterpret_cast<uint8_t*>(image->getImageMemory()->getOffsetPointer(0));
		size_t bufferSize = extent.height * stride;
		constexpr int depth = 24; // TODO: Actually use window display depth.

		libX11->xcb_put_image(
			connection,
			XCB_IMAGE_FORMAT_Z_PIXMAP,
			window,
			it->second,
			extent.width,
			extent.height,
			0, 0,       // dst x, y
			0,          // left_pad
			depth,
			bufferSize, // data_len
			buffer      // data
		);

		libX11->xcb_flush(connection);
	}
}

}