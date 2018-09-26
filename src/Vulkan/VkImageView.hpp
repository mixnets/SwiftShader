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

#include "VkObject.hpp"

namespace vk
{

class ImageView : public VkObject<ImageView, VkImageView>
{
public:
	ImageView(const VkImageViewCreateInfo* pCreateInfo) :
		image(pCreateInfo->image), viewType(pCreateInfo->viewType), format(pCreateInfo->format),
		components(pCreateInfo->components), subresourceRange(pCreateInfo->subresourceRange)
	{
	}

	~ImageView() = delete;

	void destroy(const VkAllocationCallbacks* pAllocator) override
	{
	}

private:
	VkImage                    image = VK_NULL_HANDLE;
	VkImageViewType            viewType = VK_IMAGE_VIEW_TYPE_2D;
	VkFormat                   format = VK_FORMAT_UNDEFINED;
	VkComponentMapping         components = { VK_COMPONENT_SWIZZLE_R,
	                                          VK_COMPONENT_SWIZZLE_G,
	                                          VK_COMPONENT_SWIZZLE_B,
	                                          VK_COMPONENT_SWIZZLE_A };
	VkImageSubresourceRange    subresourceRange = {};
};

static inline ImageView* Cast(VkImageView object)
{
	return reinterpret_cast<ImageView*>(object);
}

} // namespace vk

#endif // VK_IMAGE_VIEW_HPP_