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

#ifndef VK_DESCRIPTOR_UPDATE_TEMPLATE_HPP_
#define VK_DESCRIPTOR_UPDATE_TEMPLATE_HPP_

#include "VkObject.hpp"

namespace vk
{
	class DescriptorUpdateTemplate : public Object<DescriptorUpdateTemplate, VkDescriptorUpdateTemplate>
	{
	public:
		DescriptorUpdateTemplate(const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo, void* mem) :
			flags(pCreateInfo->flags),
			descriptorUpdateEntryCount(pCreateInfo->descriptorUpdateEntryCount),
			descriptorUpdateEntries(reinterpret_cast<VkDescriptorUpdateTemplateEntry*>(mem)),
			templateType(pCreateInfo->templateType),
			descriptorSetLayout(pCreateInfo->descriptorSetLayout),
			pipelineBindPoint(pCreateInfo->pipelineBindPoint),
			pipelineLayout(pCreateInfo->pipelineLayout),
			set(pCreateInfo->set)
		{
			for(uint32_t i = 0; i < descriptorUpdateEntryCount; i++)
			{
				descriptorUpdateEntries[i] = pCreateInfo->pDescriptorUpdateEntries[i];
			}
		}

		~DescriptorUpdateTemplate() = delete;

		static size_t ComputeRequiredAllocationSize(const VkDescriptorUpdateTemplateCreateInfo* info)
		{
			return info->descriptorUpdateEntryCount * sizeof(VkDescriptorUpdateTemplateEntry);
		}

	private:
		VkDescriptorUpdateTemplateCreateFlags flags = 0;
		uint32_t                              descriptorUpdateEntryCount = 0;
		VkDescriptorUpdateTemplateEntry*      descriptorUpdateEntries = nullptr;
		VkDescriptorUpdateTemplateType        templateType = VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET;
		VkDescriptorSetLayout                 descriptorSetLayout = VK_NULL_HANDLE;
		VkPipelineBindPoint                   pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		VkPipelineLayout                      pipelineLayout = VK_NULL_HANDLE;
		uint32_t                              set = 0;
	};

	static inline DescriptorUpdateTemplate* Cast(VkDescriptorUpdateTemplate object)
	{
		return reinterpret_cast<DescriptorUpdateTemplate*>(object);
	}

} // namespace vk

#endif // VK_DESCRIPTOR_UPDATE_TEMPLATE_HPP_