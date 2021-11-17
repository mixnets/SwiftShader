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

#include "VkDevice.hpp"
#include "VkImageView.hpp"
#include "VkPipelineLayout.hpp"
#include "System/Math.hpp"

namespace vk {

void DescriptorSet::ParseDescriptors(const Array &descriptorSets, const PipelineLayout *layout, Device *device, NotificationType notificationType)
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
						memoryOwner = reinterpret_cast<SampledImageDescriptor *>(descriptorMemory)->memoryOwner;
						break;
					case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
					case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
						memoryOwner = reinterpret_cast<StorageImageDescriptor *>(descriptorMemory)->memoryOwner;
						break;
					default:
						break;
					}
					if(memoryOwner)
					{
						if(notificationType == PREPARE_FOR_SAMPLING)
						{
							device->prepareForSampling(memoryOwner);
						}
						else if((notificationType == CONTENTS_CHANGED) && (type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE))
						{
							device->contentsChanged(memoryOwner, Image::USING_STORAGE);
						}
					}
					descriptorMemory += descriptorSize;
				}
			}
		}
	}
}

void DescriptorSet::ContentsChanged(const Array &descriptorSets, const PipelineLayout *layout, Device *device)
{
	ParseDescriptors(descriptorSets, layout, device, CONTENTS_CHANGED);
}

void DescriptorSet::PrepareForSampling(const Array &descriptorSets, const PipelineLayout *layout, Device *device)
{
	ParseDescriptors(descriptorSets, layout, device, PREPARE_FOR_SAMPLING);
}

size_t DescriptorSet::ensureSize(BufferDescriptor *bufferDescriptor, size_t requestedSize)
{
	ASSERT(requestedSize <= MAX_INLINE_UNIFORM_BLOCK_SIZE);
	size_t newSize = sw::min(requestedSize, MAX_INLINE_UNIFORM_BLOCK_SIZE);
	size_t curSize = bufferDescriptor->sizeInBytes;
	if(newSize > curSize)
	{
		auto itEnd = header.dynamicAllocations.end();
		auto it = std::find(header.dynamicAllocations.begin(), itEnd, bufferDescriptor->ptr);
		if(it != itEnd)
		{
			void* newPtr = vk::allocateDeviceMemory(newSize, vk::REQUIRED_MEMORY_ALIGNMENT);
			if(curSize != 0)
			{
				// Keep the old data when increasing the size
				memcpy(newPtr, bufferDescriptor->ptr, curSize);
			}
			// Replace the old pointer for the new one in the vector
			vk::freeDeviceMemory(bufferDescriptor->ptr);
			*it = bufferDescriptor->ptr = newPtr;
		}
		else
		{
			bufferDescriptor->ptr = vk::allocateDeviceMemory(newSize, vk::REQUIRED_MEMORY_ALIGNMENT);
			// Add the new pointer in the vector
			header.dynamicAllocations.push_back(bufferDescriptor->ptr);
		}

		bufferDescriptor->sizeInBytes = newSize;
		bufferDescriptor->robustnessSize = newSize;
	}

	// Return the size that can be copied, based on requested size
	return newSize;
}

void DescriptorSet::clear()
{
	for(auto ptr : header.dynamicAllocations)
	{
		vk::freeDeviceMemory(ptr);
	}
	header.dynamicAllocations.clear();
}

}  // namespace vk