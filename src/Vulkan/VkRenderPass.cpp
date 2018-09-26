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

RenderPass::RenderPass(const VkRenderPassCreateInfo* pCreateInfo, VkAttachmentDescription* pAttachments,
	                   VkSubpassDescription* pSubpasses, VkSubpassDependency* pDependencies) :
	attachmentCount(pCreateInfo->attachmentCount), attachments(pAttachments),
	subpassCount(pCreateInfo->subpassCount), subpasses(pSubpasses),
	dependencyCount(pCreateInfo->dependencyCount), dependencies(pDependencies)
{
}

void RenderPass::destroy(const VkAllocationCallbacks* pAllocator)
{
	RenderPass::DestroyMembers(pAllocator, attachments, subpasses, dependencies);
}

VkResult RenderPass::AllocateMembers(const VkAllocationCallbacks* pAllocator, const VkRenderPassCreateInfo* pCreateInfo,
                                     VkAttachmentDescription** attachments, VkSubpassDescription** subpasses,
                                     VkSubpassDependency** dependencies)
{
	size_t attachmentSize = pCreateInfo->attachmentCount * sizeof(VkAttachmentDescription);
	size_t subpassesSize = pCreateInfo->subpassCount * sizeof(VkSubpassDescription);
	size_t dependenciesSize = pCreateInfo->dependencyCount * sizeof(VkSubpassDependency);
	char* uniqueAllocation = reinterpret_cast<char*>(
		vk::allocate(attachmentSize + subpassesSize + dependenciesSize, pAllocator, GetAllocationScope()));
	if(!uniqueAllocation)
	{
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	*attachments = reinterpret_cast<VkAttachmentDescription*>(uniqueAllocation);
	memcpy(*attachments, pCreateInfo->pAttachments, attachmentSize);
	uniqueAllocation += attachmentSize;

	*subpasses = reinterpret_cast<VkSubpassDescription*>(uniqueAllocation);
	memcpy(*subpasses, pCreateInfo->pSubpasses, subpassesSize);
	uniqueAllocation += subpassesSize;

	*dependencies = reinterpret_cast<VkSubpassDependency*>(uniqueAllocation);
	memcpy(*dependencies, pCreateInfo->pDependencies, dependenciesSize);

	return VK_SUCCESS;
}

void RenderPass::DestroyMembers(const VkAllocationCallbacks* pAllocator,
	VkAttachmentDescription* attachments, VkSubpassDescription* subpasses, VkSubpassDependency* dependencies)
{
	vk::deallocate(attachments, pAllocator); // subpasses and dependencies are in the same allocation
}

} // namespace vk