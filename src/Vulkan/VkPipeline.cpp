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

#include "VkPipeline.hpp"

namespace vk
{

Pipeline::Pipeline(const VkGraphicsPipelineCreateInfo* pCreateInfo, void* mem) :
	graphicsInfo(reinterpret_cast<VkGraphicsPipelineCreateInfo*>(mem))
{
	// FIXME(sugoi): link pCreateInfo with the shader module
	*graphicsInfo = *pCreateInfo;
}

Pipeline::Pipeline(const VkComputePipelineCreateInfo* pCreateInfo, void* mem) :
	computeInfo(reinterpret_cast<VkComputePipelineCreateInfo*>(mem))
{
	// FIXME(sugoi): link pCreateInfo with the shader module
	*computeInfo = *pCreateInfo;
}

void Pipeline::destroy(const VkAllocationCallbacks* pAllocator)
{
	vk::deallocate(graphicsInfo, pAllocator);
	vk::deallocate(computeInfo, pAllocator);
}

size_t Pipeline::ComputeRequiredAllocationSize(const VkGraphicsPipelineCreateInfo* pCreateInfo)
{
	return sizeof(VkGraphicsPipelineCreateInfo);
}

size_t Pipeline::ComputeRequiredAllocationSize(const VkComputePipelineCreateInfo* pCreateInfo)
{
	return sizeof(VkComputePipelineCreateInfo);
}

void Pipeline::bindDescriptorSets(VkPipelineLayout layout, uint32_t firstSet,
                                  uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets,
                                  uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets)
{
	for(uint32_t i = 0; i < descriptorSetCount; i++)
	{
		UNIMPLEMENTED();
	}
}

} // namespace vk
