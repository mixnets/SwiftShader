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

#ifndef VK_PIPELINE_LAYOUT_HPP_
#define VK_PIPELINE_LAYOUT_HPP_

#include "VkObject.hpp"
#include <memory.h>

namespace vk
{

class PipelineLayout : public VkObject<PipelineLayout, VkPipelineLayout>
{
public:
	PipelineLayout(const VkAllocationCallbacks* pAllocator, const VkPipelineLayoutCreateInfo* pCreateInfo) :
		setLayoutCount(pCreateInfo->setLayoutCount), pushConstantRangeCount(pCreateInfo->pushConstantRangeCount)
	{
		size_t setLayoutsSize = setLayoutCount * sizeof(VkDescriptorSetLayout);
		setLayouts = reinterpret_cast<VkDescriptorSetLayout*>(
			vk::allocate(setLayoutsSize, pAllocator, GetAllocationScope()));
		if(setLayouts)
		{
			memcpy(setLayouts, pCreateInfo->pSetLayouts, setLayoutsSize);
		}

		size_t pushConstantRangesSize = pushConstantRangeCount * sizeof(VkPushConstantRange);
		pushConstantRanges = reinterpret_cast<VkPushConstantRange*>(
			vk::allocate(pushConstantRangesSize, pAllocator, GetAllocationScope()));
		if(pushConstantRanges)
		{
			memcpy(pushConstantRanges, pCreateInfo->pPushConstantRanges, pushConstantRangesSize);
		}
	}

	~PipelineLayout() = delete;

	void destroy(const VkAllocationCallbacks* pAllocator) override
	{
		vk::deallocate(setLayouts, pAllocator);
		setLayouts = nullptr;
		setLayoutCount = 0;

		vk::deallocate(pushConstantRanges, pAllocator);
		pushConstantRanges = nullptr;
		pushConstantRangeCount = 0;
	}

	bool validate() const override
	{
		return setLayouts && pushConstantRanges;
	}

private:
	uint32_t               setLayoutCount = 0;
	VkDescriptorSetLayout* setLayouts = nullptr;
	uint32_t               pushConstantRangeCount = 0;
	VkPushConstantRange*   pushConstantRanges = nullptr;
};

static inline PipelineLayout* Cast(VkPipelineLayout object)
{
	return reinterpret_cast<PipelineLayout*>(object);
}

} // namespace vk

#endif // VK_PIPELINE_LAYOUT_HPP_
