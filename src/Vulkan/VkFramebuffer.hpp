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
#ifndef VK_FRAMEBUFFER_HPP_
#define VK_FRAMEBUFFER_HPP_
#include "VkObject.hpp"
namespace vk {
VkClass(Framebuffer) {
public:
	constexpr static VkSystemAllocationScope kAllocationScope = VK_SYSTEM_ALLOCATION_SCOPE_OBJECT;

	Framebuffer(const VkAllocationCallbacks* pAllocator, const VkFramebufferCreateInfo* pCreateInfo) :
		renderPass(pCreateInfo->renderPass), attachmentCount(pCreateInfo->attachmentCount),
		width(pCreateInfo->width), height(pCreateInfo->height), layers(pCreateInfo->layers)
	{
		attachments = reinterpret_cast<VkImageView*>(
			vk::allocate(attachmentCount * sizeof(VkImageView),
				pAllocator, kAllocationScope, pCreateInfo->pAttachments));
	}	~Framebuffer() = delete;
	void destroy(const VkAllocationCallbacks* pAllocator) override
	{
		vk::deallocate(attachments, pAllocator);
		attachments = nullptr;
		attachmentCount = 0;
	}
private:
	VkRenderPass renderPass;
	uint32_t     attachmentCount;
	VkImageView* attachments;
	uint32_t     width;
	uint32_t     height;
	uint32_t     layers;
};

} // namespace vk
#endif // VK_FRAMEBUFFER_HPP_
