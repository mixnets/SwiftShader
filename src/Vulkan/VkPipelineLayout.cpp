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

PipelineLayout::PipelineLayout(const VkPipelineLayoutCreateInfo* pCreateInfo, void* mem) :
	setLayoutCount(pCreateInfo->setLayoutCount), pushConstantRangeCount(pCreateInfo->pushConstantRangeCount)
{
	char* hostMemory = reinterpret_cast<char*>(mem);

	if(setLayoutCount)
	{
		size_t setLayoutsSize = setLayoutCount * sizeof(VkDescriptorSetLayout);
		setLayouts = reinterpret_cast<VkDescriptorSetLayout*>(hostMemory);
		memcpy(setLayouts, pCreateInfo->pSetLayouts, setLayoutsSize);
		hostMemory += setLayoutsSize;
	}

	if(pushConstantRangeCount)
	{
		size_t pushConstantRangesSize = pushConstantRangeCount * sizeof(VkPushConstantRange);
		pushConstantRanges = reinterpret_cast<VkPushConstantRange*>(hostMemory);
		memcpy(pushConstantRanges, pCreateInfo->pPushConstantRanges, pushConstantRangesSize);
	}
}

void PipelineLayout::destroy(const VkAllocationCallbacks* pAllocator)
{
	vk::deallocate(setLayouts, pAllocator); // pPushConstantRanges is in the same allocation
}

size_t PipelineLayout::ComputeRequiredAllocationSize(const VkPipelineLayoutCreateInfo* pCreateInfo)
{
	size_t setLayoutsSize = pCreateInfo->setLayoutCount * sizeof(VkDescriptorSetLayout);
	size_t pushConstantRangesSize = pCreateInfo->pushConstantRangeCount * sizeof(VkPushConstantRange);
	return setLayoutsSize + pushConstantRangesSize;
}

} // namespace vk