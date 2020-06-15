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
#include "VkDescriptorPool.hpp"
#include "VkDescriptorSetLayout.hpp"

namespace vk {

DescriptorSetMemoryOwners::DescriptorSetMemoryOwners(DescriptorSetLayout *layout)
{
	uint32_t bindingsArraySize = layout->getBindingsArraySize();
	memoryOwners.resize(bindingsArraySize);
	for(uint32_t i = 0; i < bindingsArraySize; i++)
	{
		uint32_t descriptorCount = layout->getDescriptorCount(i);
		memoryOwners[i].resize(descriptorCount);
	}
}

void DescriptorSetMemoryOwners::increment(uint32_t &bindingNumber, uint32_t &arrayElement) const
{
	ASSERT(bindingNumber < memoryOwners.size());

	if(arrayElement >= (memoryOwners[bindingNumber].size() - 1))
	{
		++bindingNumber;
		arrayElement = 0;
	}
	else
	{
		++arrayElement;
	}
}

void DescriptorSetMemoryOwners::prepareForSampling()
{
	for(auto bindingData : memoryOwners)
	{
		for(auto arrayElementData : bindingData)
		{
			if(arrayElementData.view)
			{
				arrayElementData.view->prepareForSampling();
			}
		}
	}
}

void DescriptorSetMemoryOwners::contentsChanged()
{
	for(auto bindingData : memoryOwners)
	{
		for(auto arrayElementData : bindingData)
		{
			if(arrayElementData.view && !arrayElementData.readOnly)
			{
				arrayElementData.view->contentsChanged();
			}
		}
	}
}

void DescriptorSetMemoryOwners::storeMetaData(uint32_t bindingNumber, uint32_t arrayElement, ImageView *imageView, bool readOnly)
{
	memoryOwners[bindingNumber][arrayElement] = { imageView, readOnly };
}

void DescriptorSetMemoryOwners::copyTo(uint32_t srcBinding, uint32_t srcArrayElement, DescriptorSetMemoryOwners &dstSet, uint32_t dstBinding, uint32_t dstArrayElement, uint32_t descriptorCount)
{
	for(uint32_t i = 0; i < descriptorCount; ++i)
	{
		dstSet.memoryOwners[dstBinding][dstArrayElement] = memoryOwners[srcBinding][srcArrayElement];
		increment(srcBinding, srcArrayElement);
		dstSet.increment(dstBinding, dstArrayElement);
	}
}

void DescriptorSet::init()
{
	header.layout = nullptr;
	header.pool = nullptr;
}

void DescriptorSet::prepareForSampling()
{
	if(header.pool)
	{
		header.pool->prepareForSampling(this);
	}
}

void DescriptorSet::contentsChanged()
{
	if(header.pool)
	{
		header.pool->contentsChanged(this);
	}
}

void DescriptorSet::storeMetaData(uint32_t bindingNumber, uint32_t arrayElement, ImageView *imageView, bool readOnly)
{
	if(header.pool)
	{
		header.pool->storeMetaData(this, bindingNumber, arrayElement, imageView, readOnly);
	}
}

void DescriptorSet::copyMetadata(uint32_t srcBinding, uint32_t srcArrayElement, DescriptorSet *dstSet, uint32_t dstBinding, uint32_t dstArrayElement, uint32_t descriptorCount)
{
	if(dstSet)
	{
		header.pool->copyMetadata(this, srcBinding, srcArrayElement, dstSet, dstBinding, dstArrayElement, descriptorCount);
	}
}

}  // namespace vk