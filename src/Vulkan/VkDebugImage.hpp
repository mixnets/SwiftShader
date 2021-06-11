#ifndef _VK_DEBUG_IMAGE_HPP_
#define _VK_DEBUG_IMAGE_HPP_

#include "VkImage.hpp"

namespace vk {
#if !defined(NDEBUG) && defined(DEBUG_IMAGES)
void writeImageToFile(const Image *buffer, const char *fileName);
#else
#	define writeImageToFile(...)
#endif  // IMAGE_DEBUG
};      // namespace vk

#endif