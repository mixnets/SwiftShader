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

#include "VkRenderPass.hpp"
#include <memory.h>

namespace vk
{

RenderPass::RenderPass(const VkRenderPassCreateInfo* pCreateInfo, const Memory& mem) :
	attachmentCount(pCreateInfo->attachmentCount),
	subpassCount(pCreateInfo->subpassCount),
	dependencyCount(pCreateInfo->dependencyCount)
{
	char* hostMemory = mem.host;

	size_t attachmentSize = pCreateInfo->attachmentCount * sizeof(VkAttachmentDescription);
	attachments = reinterpret_cast<VkAttachmentDescription*>(hostMemory);
	memcpy(attachments, pCreateInfo->pAttachments, attachmentSize);
	hostMemory += attachmentSize;

	size_t subpassesSize = pCreateInfo->subpassCount * sizeof(VkSubpassDescription);
	subpasses = reinterpret_cast<VkSubpassDescription*>(hostMemory);
	memcpy(subpasses, pCreateInfo->pSubpasses, subpassesSize);
	hostMemory += subpassesSize;

	size_t dependenciesSize = pCreateInfo->dependencyCount * sizeof(VkSubpassDependency);
	dependencies = reinterpret_cast<VkSubpassDependency*>(hostMemory);
	memcpy(dependencies, pCreateInfo->pDependencies, dependenciesSize);
}

void RenderPass::destroy(const VkAllocationCallbacks* pAllocator)
{
	vk::deallocate(attachments, pAllocator); // subpasses and dependencies are in the same allocation
}

MemorySize RenderPass::ComputeRequiredAllocationSize(const VkRenderPassCreateInfo* pCreateInfo)
{
	size_t attachmentSize = pCreateInfo->attachmentCount * sizeof(VkAttachmentDescription);
	size_t subpassesSize = pCreateInfo->subpassCount * sizeof(VkSubpassDescription);
	size_t dependenciesSize = pCreateInfo->dependencyCount * sizeof(VkSubpassDependency);

	return MemorySize(attachmentSize + subpassesSize + dependenciesSize, 0);
}

} // namespace vk