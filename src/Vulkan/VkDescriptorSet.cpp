// Copyright 2020 The SwiftShader Authors. All Rights Reserved.
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

#include "VkDescriptorSet.hpp"
#include "VkDescriptorSetLayout.hpp"
#include "VkImageView.hpp"
#include "VkPipelineLayout.hpp"

namespace vk {

void DescriptorSet::ContentsChanged(const Array &descriptorSets, const PipelineLayout *layout)
{
	if(layout)
	{
		uint32_t descriptorSetCount = layout->getDescriptorSetCount();
		ASSERT(descriptorSetCount <= MAX_BOUND_DESCRIPTOR_SETS);

		for(uint32_t i = 0; i < descriptorSetCount; ++i)
		{
			DescriptorSet *descriptorSet = descriptorSets[i];
			marl::lock lock(descriptorSet->header.mutex);
			uint32_t bindingCount = layout->getBindingCount(i);
			for(uint32_t j = 0; j < bindingCount; ++j)
			{
				VkDescriptorType type = layout->getDescriptorType(i, j);
				uint32_t descriptorCount = layout->getDescriptorCount(i, j);
				uint32_t descriptorSize = layout->getDescriptorSize(i, j);
				uint8_t *descriptorMemory = descriptorSet->data + layout->getBindingOffset(i, j);

				for(uint32_t k = 0; k < descriptorCount; k++)
				{
					switch(type)
					{
						case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
						case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
						{
							SampledImageDescriptor *descriptor = reinterpret_cast<SampledImageDescriptor *>(descriptorMemory);
							if(descriptor->memoryOwner.view && !descriptor->memoryOwner.readOnly)
							{
								descriptor->memoryOwner.view->contentsChanged();
							}
						}
						break;
						case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
						case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
						{
							StorageImageDescriptor *descriptor = reinterpret_cast<StorageImageDescriptor *>(descriptorMemory);
							if(descriptor->memoryOwner.view && !descriptor->memoryOwner.readOnly)
							{
								descriptor->memoryOwner.view->contentsChanged();
							}
						}
						break;
						default:
							break;
					}
					descriptorMemory += descriptorSize;
				}
			}
		}
	}
}

void DescriptorSet::PrepareForSampling(const Array &descriptorSets, const PipelineLayout *layout)
{
	if(layout)
	{
		uint32_t descriptorSetCount = layout->getDescriptorSetCount();
		ASSERT(descriptorSetCount <= MAX_BOUND_DESCRIPTOR_SETS);

		for(uint32_t i = 0; i < descriptorSetCount; ++i)
		{
			DescriptorSet *descriptorSet = descriptorSets[i];
			if(!descriptorSet)
			{
				continue;
			}

			marl::lock lock(descriptorSet->header.mutex);
			uint32_t bindingCount = layout->getBindingCount(i);
			for(uint32_t j = 0; j < bindingCount; ++j)
			{
				VkDescriptorType type = layout->getDescriptorType(i, j);
				uint32_t descriptorCount = layout->getDescriptorCount(i, j);
				uint32_t descriptorSize = layout->getDescriptorSize(i, j);
				uint8_t *descriptorMemory = descriptorSet->data + layout->getBindingOffset(i, j);

				for(uint32_t k = 0; k < descriptorCount; k++)
				{
					switch(type)
					{
						case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
						case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
						{
							SampledImageDescriptor *descriptor = reinterpret_cast<SampledImageDescriptor *>(descriptorMemory);
							if(descriptor->memoryOwner.view)
							{
								descriptor->memoryOwner.view->prepareForSampling();
							}
						}
						break;
						case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
						case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
						{
							StorageImageDescriptor *descriptor = reinterpret_cast<StorageImageDescriptor *>(descriptorMemory);
							if(descriptor->memoryOwner.view)
							{
								descriptor->memoryOwner.view->prepareForSampling();
							}
						}
						break;
						default:
							break;
					}
				}
			}
		}
	}
}

}  // namespace vk