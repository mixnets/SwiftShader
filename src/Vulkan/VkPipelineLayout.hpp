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

namespace vk {

VkClass(PipelineLayout) {
public:
	constexpr static VkSystemAllocationScope kAllocationScope = VK_SYSTEM_ALLOCATION_SCOPE_OBJECT;

	PipelineLayout(const VkAllocationCallbacks* pAllocator, const VkPipelineLayoutCreateInfo* pCreateInfo) :
		setLayoutCount(pCreateInfo->setLayoutCount), pushConstantRangeCount(pCreateInfo->pushConstantRangeCount)
	{
		setLayouts = reinterpret_cast<VkDescriptorSetLayout*>(
			vk::allocate(setLayoutCount * sizeof(VkDescriptorSetLayout),
			pAllocator, kAllocationScope, pCreateInfo->pSetLayouts));

		pushConstantRanges = reinterpret_cast<VkPushConstantRange*>(
			vk::allocate(pushConstantRangeCount * sizeof(VkPushConstantRange),
			pAllocator, kAllocationScope, pCreateInfo->pPushConstantRanges));
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

private:
	uint32_t setLayoutCount;
	VkDescriptorSetLayout* setLayouts;
	uint32_t pushConstantRangeCount;
	VkPushConstantRange* pushConstantRanges;
};

} // namespace vk

#endif // VK_PIPELINE_LAYOUT_HPP_
