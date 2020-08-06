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
#include "VkImageView.hpp"
#include "VkPipelineLayout.hpp"

namespace vk {

void DescriptorSet::ParseDescriptors(const Array &descriptorSets, const PipelineLayout *layout, NotificationType notificationType)
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
					ImageView *memoryOwner = nullptr;
					switch(type)
					{
						case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
						case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
						{
							SampledImageDescriptor *descriptor = reinterpret_cast<SampledImageDescriptor *>(descriptorMemory);
							if(descriptor->memoryOwner &&
							   (descriptorSet->header.deletedMemoryOwners.find(descriptor->memoryOwner) !=
							    descriptorSet->header.deletedMemoryOwners.end()))
							{
								descriptor->memoryOwner = nullptr;
							}
							memoryOwner = descriptor->memoryOwner;
						}
						break;
						case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
						case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
						{
							StorageImageDescriptor *descriptor = reinterpret_cast<StorageImageDescriptor *>(descriptorMemory);
							if(descriptor->memoryOwner &&
							   (descriptorSet->header.deletedMemoryOwners.find(descriptor->memoryOwner) !=
							    descriptorSet->header.deletedMemoryOwners.end()))
							{
								descriptor->memoryOwner = nullptr;
							}
							memoryOwner = descriptor->memoryOwner;
						}
						break;
						default:
							break;
					}
					if(memoryOwner)
					{
						if(notificationType == PREPARE_FOR_SAMPLING)
						{
							memoryOwner->prepareForSampling();
						}
						else if((notificationType == CONTENTS_CHANGED) && (type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE))
						{
							memoryOwner->contentsChanged();
						}
					}
					descriptorMemory += descriptorSize;
				}
			}
			descriptorSet->header.deletedMemoryOwners.clear();
		}
	}
}

void DescriptorSet::ContentsChanged(const Array &descriptorSets, const PipelineLayout *layout)
{
	ParseDescriptors(descriptorSets, layout, CONTENTS_CHANGED);
}

void DescriptorSet::PrepareForSampling(const Array &descriptorSets, const PipelineLayout *layout)
{
	ParseDescriptors(descriptorSets, layout, PREPARE_FOR_SAMPLING);
}

DescriptorSet::~DescriptorSet()
{
	marl::lock lock(header.mutex);
	for(auto memoryOwner : header.currentMemoryOwners)
	{
		memoryOwner->releaseDescriptorSet(this);
	}
	header.currentMemoryOwners.clear();
	header.deletedMemoryOwners.clear();
}

void DescriptorSet::releaseMemoryOwner(ImageView *imageView)
{
	unref(imageView);

	marl::lock lock(header.mutex);
	header.deletedMemoryOwners.insert(imageView);
}

void DescriptorSet::ref(ImageView *imageView)
{
	marl::lock lock(header.mutex);
	header.currentMemoryOwners.insert(imageView);
}

void DescriptorSet::unref(ImageView *imageView)
{
	marl::lock lock(header.mutex);
	auto it = header.currentMemoryOwners.find(imageView);
	if(it != header.currentMemoryOwners.end())
	{
		header.currentMemoryOwners.erase(it);
	}
}

}  // namespace vk