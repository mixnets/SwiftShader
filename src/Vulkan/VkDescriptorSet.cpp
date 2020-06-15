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

namespace vk {

DescriptorSetMetaData::DescriptorSetMetaData(DescriptorSetLayout *layout)
{
	uint32_t bindingsArraySize = layout->getBindingsArraySize();
	data.resize(bindingsArraySize);
	for(uint32_t i = 0; i < bindingsArraySize; i++)
	{
		uint32_t descriptorCount = layout->getDescriptorCount(i);
		data[i].resize(descriptorCount);
		for(uint32_t j = 0; j < descriptorCount; j++)
		{
			data[i][j] = { nullptr, false };
		}
	}
}

void DescriptorSetMetaData::increment(uint32_t &bindingNumber, uint32_t &arrayElement) const
{
	ASSERT(bindingNumber < data.size());

	if(arrayElement >= (data[bindingNumber].size() - 1))
	{
		++bindingNumber;
		arrayElement = 0;
	}
	else
	{
		++arrayElement;
	}
}

void DescriptorSetMetaData::prepareForSampling()
{
	marl::lock lock(mutex);
	for(auto bindingData : data)
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

void DescriptorSetMetaData::contentsChanged()
{
	marl::lock lock(mutex);
	for(auto bindingData : data)
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

void DescriptorSetMetaData::storeMetaData(uint32_t bindingNumber, uint32_t arrayElement, ImageView *imageView, bool readOnly)
{
	marl::lock lock(mutex);
	data[bindingNumber][arrayElement] = { imageView, readOnly };
}

void DescriptorSetMetaData::copyTo(uint32_t descriptorCount, uint32_t srcBinding, uint32_t srcArrayElement, uint32_t dstBinding, uint32_t dstArrayElement, DescriptorSetMetaData *dstSet)
{
	if(dstSet != this)
	{
		marl::lock srcLock(mutex);
		marl::lock dstLock(dstSet->mutex);
		for(uint32_t i = 0; i < descriptorCount; ++i)
		{
			dstSet->data[dstBinding][dstArrayElement] = data[srcBinding][srcArrayElement];
			increment(srcBinding, srcArrayElement);
			dstSet->increment(dstBinding, dstArrayElement);
		}
	}
	else  // copy between 2 regions of the same descriptor set
	{
		marl::lock srcLock(mutex);
		for(uint32_t i = 0; i < descriptorCount; ++i)
		{
			data[dstBinding][dstArrayElement] = data[srcBinding][srcArrayElement];
			increment(srcBinding, srcArrayElement);
			increment(dstBinding, dstArrayElement);
		}
	}
}

void DescriptorSet::init()
{
	header.layout = nullptr;
	header.metadata = nullptr;
}

void DescriptorSet::destroyMetadata()
{
	delete header.metadata;
	header.metadata = nullptr;
}

void DescriptorSet::increment(uint32_t &bindingNumber, uint32_t &arrayElement)
{
	if(header.metadata)
	{
		header.metadata->increment(bindingNumber, arrayElement);
	}
}

void DescriptorSet::prepareForSampling() const
{
	if(header.metadata)
	{
		header.metadata->prepareForSampling();
	}
}

void DescriptorSet::contentsChanged() const
{
	if(header.metadata)
	{
		header.metadata->contentsChanged();
	}
}

void DescriptorSet::storeMetaData(uint32_t bindingNumber, uint32_t arrayElement, ImageView *imageView, bool readOnly)
{
	if(header.metadata)
	{
		header.metadata->storeMetaData(bindingNumber, arrayElement, imageView, readOnly);
	}
}

void DescriptorSet::copyMetadata(uint32_t descriptorCount, uint32_t srcBinding, uint32_t srcArrayElement, uint32_t dstBinding, uint32_t dstArrayElement, DescriptorSet *dstSet) const
{
	if(header.metadata && dstSet->header.metadata)
	{
		header.metadata->copyTo(descriptorCount, srcBinding, srcArrayElement, dstBinding, dstArrayElement, dstSet->header.metadata);
	}
}

}  // namespace vk