#include "VkDebugImage.hpp"

#include "VkFormat.hpp"
#include "VkImage.hpp"

#ifdef DEBUG_IMAGES
#	define STB_IMAGE_IMPLEMENTATION
#	include "stb/stb_image.h"
#	define STB_IMAGE_WRITE_IMPLEMENTATION
#	include "stb/stb_image_write.h"

namespace vk {
void writeImageToFile(const vk::Image *image, const char *fileName)
{
	if(!image)
	{
		return;
	}

	VkImageAspectFlags flags = VK_IMAGE_ASPECT_COLOR_BIT;
	vk::Format format = image->getFormat();
	if(format.isStencil())
	{
		flags = VK_IMAGE_ASPECT_STENCIL_BIT;
	}
	else if(format.isDepth())
	{
		flags = VK_IMAGE_ASPECT_DEPTH_BIT;
	}

	VkImageSubresourceRange range = {
		flags,  // aspectMask
		0,      // baseMipLevel
		1,      // levelCount
		0,      // baseArrayLayer
		1,      // layerCount
	};
	size_t size = image->getSizeInBytes(range);
	VkExtent3D extent = image->getExtent();
	int rowPitchBytes = image->rowPitchBytes((VkImageAspectFlagBits)flags, 0);
	void *mem = malloc(size);
	image->copyTo(reinterpret_cast<uint8_t *>(mem), rowPitchBytes);

	stbi_write_png(fileName, extent.width, extent.height,
	               format.componentCount(), mem, rowPitchBytes);
}

};  // namespace vk

#endif  // DEBUG_IMAGES