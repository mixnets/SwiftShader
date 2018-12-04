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

#include "VkImageView.hpp"
#include "VkImage.hpp"

namespace vk
{

ImageView::ImageView(const VkImageViewCreateInfo* pCreateInfo, void* mem) :
	image(pCreateInfo->image), viewType(pCreateInfo->viewType), format(pCreateInfo->format),
	components(pCreateInfo->components), subresourceRange(pCreateInfo->subresourceRange)
{
}

size_t ImageView::ComputeRequiredAllocationSize(const VkImageViewCreateInfo* pCreateInfo)
{
	return 0;
}

void ImageView::destroy(const VkAllocationCallbacks* pAllocator)
{
}

void ImageView::clear(const VkClearValue& pClearValue, const VkRect2D& pRenderArea)
{
	// Note: clearing ignores swizzling, so components is ignored.

	auto imageObject = Cast(image);

	if(imageObject->getImageType() != viewType)
	{
		UNIMPLEMENTED();
	}

	if(imageObject->getFormat() != format)
	{
		UNIMPLEMENTED();
	}

	if((subresourceRange.aspectMask != VK_IMAGE_ASPECT_COLOR_BIT) ||
	   (subresourceRange.baseMipLevel != 0) ||
	   (subresourceRange.levelCount != 1) ||
	   (subresourceRange.baseArrayLayer != 0) ||
	   (subresourceRange.layerCount != 1))
	{
		UNIMPLEMENTED();
	}

	imageObject->clear(pClearValue, pRenderArea, 0xF);
}

}