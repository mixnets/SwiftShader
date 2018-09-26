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

#include "VkPipelineLayout.hpp"
#include <memory.h>

namespace vk
{

PipelineLayout::PipelineLayout(const VkPipelineLayoutCreateInfo* pCreateInfo,
	VkDescriptorSetLayout* pSetLayouts, VkPushConstantRange* pPushConstantRanges) :
	setLayoutCount(pCreateInfo->setLayoutCount), setLayouts(pSetLayouts),
	pushConstantRangeCount(pCreateInfo->pushConstantRangeCount), pushConstantRanges(pPushConstantRanges)
{

}

void PipelineLayout::destroy(const VkAllocationCallbacks* pAllocator)
{
	DestroyMembers(pAllocator, setLayouts, pushConstantRanges);
}

VkResult PipelineLayout::AllocateMembers(const VkAllocationCallbacks* pAllocator, const VkPipelineLayoutCreateInfo* pCreateInfo,
	VkDescriptorSetLayout** pSetLayouts, VkPushConstantRange** pPushConstantRanges)
{
	size_t setLayoutsSize = pCreateInfo->setLayoutCount * sizeof(VkDescriptorSetLayout);
	size_t pushConstantRangesSize = pCreateInfo->pushConstantRangeCount * sizeof(VkPushConstantRange);
	char* uniqueAllocation = reinterpret_cast<char*>(
		vk::allocate(setLayoutsSize + pushConstantRangesSize, pAllocator, GetAllocationScope()));

	if(!uniqueAllocation)
	{
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	*pSetLayouts = reinterpret_cast<VkDescriptorSetLayout*>(uniqueAllocation);
	memcpy(*pSetLayouts, pCreateInfo->pSetLayouts, setLayoutsSize);

	*pPushConstantRanges = reinterpret_cast<VkPushConstantRange*>(uniqueAllocation + setLayoutsSize);
	memcpy(*pPushConstantRanges, pCreateInfo->pPushConstantRanges, pushConstantRangesSize);

	return VK_SUCCESS;
}

void PipelineLayout::DestroyMembers(const VkAllocationCallbacks* pAllocator,
	VkDescriptorSetLayout* pSetLayouts, VkPushConstantRange* pPushConstantRanges)
{
	vk::deallocate(pSetLayouts, pAllocator); // pPushConstantRanges is in the same allocation
}

} // namespace vk