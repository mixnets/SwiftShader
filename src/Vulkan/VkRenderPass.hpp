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

namespace vk
{

class RenderPass : public Object<RenderPass, VkRenderPass>
{
public:
	RenderPass(const VkRenderPassCreateInfo* pCreateInfo, void* mem);
	~RenderPass() = delete;
	void destroy(const VkAllocationCallbacks* pAllocator);

	void getRenderAreaGranularity(VkExtent2D* pGranularity) const;

	static size_t ComputeRequiredAllocationSize(const VkRenderPassCreateInfo* pCreateInfo);

	void begin();
	void end();

private:
	uint32_t                 attachmentCount = 0;
	VkAttachmentDescription* attachments = nullptr;
	uint32_t                 subpassCount = 0;
	VkSubpassDescription*    subpasses = nullptr;
	uint32_t                 dependencyCount = 0;
	VkSubpassDependency*     dependencies = nullptr;
};

static inline RenderPass* Cast(VkRenderPass object)
{
	return reinterpret_cast<RenderPass*>(object);
}

} // namespace vk

#endif // VK_RENDER_PASS_HPP_