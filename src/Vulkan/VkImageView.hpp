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

#ifndef VK_IMAGE_VIEW_HPP_
#define VK_IMAGE_VIEW_HPP_

#include "VkDebug.hpp"
#include "VkObject.hpp"
#include "VkImage.hpp"

namespace vk
{

class ImageView : public Object<ImageView, VkImageView>
{
public:
	ImageView(const VkImageViewCreateInfo* pCreateInfo, void* mem) :
		image(pCreateInfo->image), viewType(pCreateInfo->viewType), format(pCreateInfo->format),
		components(pCreateInfo->components), subresourceRange(pCreateInfo->subresourceRange)
	{
	}

	~ImageView() = delete;

	static size_t ComputeRequiredAllocationSize(const VkImageViewCreateInfo* pCreateInfo)
	{
		return 0;
	}

	void destroy(const VkAllocationCallbacks* pAllocator)
	{
	}

	void clear(const VkClearValue& pClearValues, const VkRect2D& pRenderArea)
	{
		auto imageObject = Cast(image);

		ASSERT(imageObject->getImageType() == viewType);
		ASSERT(imageObject->getFormat() == format);
		
		unsigned int rgbaMask = 0;
		switch(components.r)
		{
		case VK_COMPONENT_SWIZZLE_IDENTITY:
		case VK_COMPONENT_SWIZZLE_R:
			rgbaMask |= 0x1;
			break;
		default:
			UNIMPLEMENTED();
		}
		switch(components.g)
		{
		case VK_COMPONENT_SWIZZLE_IDENTITY:
		case VK_COMPONENT_SWIZZLE_G:
			rgbaMask |= 0x2;
			break;
		default:
			UNIMPLEMENTED();
		}
		switch(components.b)
		{
		case VK_COMPONENT_SWIZZLE_IDENTITY:
		case VK_COMPONENT_SWIZZLE_B:
			rgbaMask |= 0x4;
			break;
		default:
			UNIMPLEMENTED();
		}
		switch(components.a)
		{
		case VK_COMPONENT_SWIZZLE_IDENTITY:
		case VK_COMPONENT_SWIZZLE_A:
			rgbaMask |= 0x8;
			break;
		default:
			UNIMPLEMENTED();
		}

		// FIXME: ignoring subresourceRange

		imageObject->clear(pClearValues, pRenderArea, rgbaMask);
	}

private:
	VkImage                    image = VK_NULL_HANDLE;
	VkImageViewType            viewType = VK_IMAGE_VIEW_TYPE_2D;
	VkFormat                   format = VK_FORMAT_UNDEFINED;
	VkComponentMapping         components = {};
	VkImageSubresourceRange    subresourceRange = {};
};

static inline ImageView* Cast(VkImageView object)
{
	return reinterpret_cast<ImageView*>(object);
}

} // namespace vk

#endif // VK_IMAGE_VIEW_HPP_