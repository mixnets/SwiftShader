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

#include <cstring>

namespace vk
{

PipelineLayout::PipelineLayout(const VkPipelineLayoutCreateInfo* pCreateInfo, void* mem)
{
	setLayoutCount = pCreateInfo->setLayoutCount;
	memcpy(&setLayouts[0], pCreateInfo->pSetLayouts, sizeof(VkDescriptorSetLayout) * setLayoutCount);

	// TODO: push constants
}

void PipelineLayout::destroy(const VkAllocationCallbacks* pAllocator)
{
}

size_t PipelineLayout::ComputeRequiredAllocationSize(const VkPipelineLayoutCreateInfo* pCreateInfo)
{
	return sizeof(PipelineLayout) +
		pCreateInfo->setLayoutCount * sizeof(setLayouts[0]);
}

VkDescriptorSetLayout PipelineLayout::getDescriptorSetLayout(uint32_t index) const
{
	ASSERT(index < setLayoutCount);
	return setLayouts[index];
}

} // namespace vk
