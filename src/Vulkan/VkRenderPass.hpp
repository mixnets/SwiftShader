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

#ifndef VK_RENDER_PASS_HPP_
#define VK_RENDER_PASS_HPP_

#include "VkObject.hpp"

namespace vk {

VkClass(RenderPass) {
public:
	constexpr static VkSystemAllocationScope kAllocationScope = VK_SYSTEM_ALLOCATION_SCOPE_OBJECT;

	RenderPass(const VkAllocationCallbacks* pAllocator, const VkRenderPassCreateInfo* pCreateInfo) :
		attachmentCount(pCreateInfo->attachmentCount), subpassCount(pCreateInfo->subpassCount),
		dependencyCount(pCreateInfo->dependencyCount)
	{
		attachments = reinterpret_cast<VkAttachmentDescription*>(
			vk::allocate(attachmentCount * sizeof(VkAttachmentDescription),
			pAllocator, kAllocationScope, pCreateInfo->pAttachments));

		subpasses = reinterpret_cast<VkSubpassDescription*>(
			vk::allocate(subpassCount * sizeof(VkSubpassDescription),
			pAllocator, kAllocationScope, pCreateInfo->pSubpasses));

		dependencies = reinterpret_cast<VkSubpassDependency*>(
			vk::allocate(dependencyCount * sizeof(VkSubpassDependency),
			pAllocator, kAllocationScope, pCreateInfo->pDependencies));
	}

	~RenderPass() = delete;

	void destroy(const VkAllocationCallbacks* pAllocator) override
	{
		vk::deallocate(attachments, pAllocator);
		attachments = nullptr;
		attachmentCount = 0;

		vk::deallocate(subpasses, pAllocator);
		subpasses = nullptr;
		subpassCount = 0;

		vk::deallocate(dependencies, pAllocator);
		dependencies = nullptr;
		dependencyCount = 0;
	}

private:
	uint32_t                 attachmentCount;
	VkAttachmentDescription* attachments;
	uint32_t                 subpassCount;
	VkSubpassDescription*    subpasses;
	uint32_t                 dependencyCount;
	VkSubpassDependency*     dependencies;
};

} // namespace vk

#endif // VK_RENDER_PASS_HPP_