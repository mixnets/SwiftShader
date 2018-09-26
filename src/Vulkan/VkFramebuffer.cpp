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

#include "VkFramebuffer.hpp"
#include <memory.h>

namespace vk
{

Framebuffer::Framebuffer(const VkFramebufferCreateInfo* pCreateInfo, VkImageView* pAttachments) :
	renderPass(pCreateInfo->renderPass), attachmentCount(pCreateInfo->attachmentCount),
	attachments(pAttachments), width(pCreateInfo->width), height(pCreateInfo->height),
	layers(pCreateInfo->layers)
{
}

void Framebuffer::destroy(const VkAllocationCallbacks* pAllocator)
{
	DestroyAttachments(pAllocator, attachments);
}

VkResult Framebuffer::AllocateAttachments(const VkAllocationCallbacks* pAllocator,
	const VkFramebufferCreateInfo* pCreateInfo, VkImageView** pAttachments)
{
	size_t attachmentsSize = pCreateInfo->attachmentCount * sizeof(VkImageView);
	*pAttachments = reinterpret_cast<VkImageView*>(
		vk::allocate(attachmentsSize, pAllocator, GetAllocationScope()));
	if(*pAttachments)
	{
		memcpy(*pAttachments, pCreateInfo->pAttachments, attachmentsSize);
		return VK_SUCCESS;
	}

	return VK_ERROR_OUT_OF_HOST_MEMORY;
}

void Framebuffer::DestroyAttachments(const VkAllocationCallbacks* pAllocator, VkImageView* attachments)
{
	vk::deallocate(attachments, pAllocator);
}

} // namespace vk