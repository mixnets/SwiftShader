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

#ifndef VK_PIPELINE_HPP_
#define VK_PIPELINE_HPP_

#include "VkObject.hpp"

namespace vk
{

VkClass(Pipeline)
{
public:
	Pipeline(const VkAllocationCallbacks* pAllocator, const VkGraphicsPipelineCreateInfo& pCreateInfo) :
		graphicsInfo(&pCreateInfo)
	{
		// FIXME(sugoi): link pCreateInfo with the shader module
	}

	Pipeline(const VkAllocationCallbacks* pAllocator, const VkComputePipelineCreateInfo& pCreateInfo) :
		computeInfo(&pCreateInfo)
	{
		// FIXME(sugoi): link pCreateInfo with the shader module
	}

	~Pipeline() = delete;

private:
	const VkGraphicsPipelineCreateInfo* graphicsInfo = nullptr;
	const VkComputePipelineCreateInfo* computeInfo = nullptr;
};

} // namespace vk

#endif // VK_PIPELINE_HPP_
