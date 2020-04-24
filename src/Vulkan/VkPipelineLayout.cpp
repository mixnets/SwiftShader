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

#include "VkPipelineLayout.hpp"

#include <atomic>
#include <cstring>

namespace vk {

static std::atomic<uint32_t> layoutIdentifierSerial = { 1 };  // Start at 1. 0 is invalid/void layout.

PipelineLayout::PipelineLayout(const VkPipelineLayoutCreateInfo *pCreateInfo, void *mem)
    : identifier(layoutIdentifierSerial++)
    , descriptorSetCount(pCreateInfo->setLayoutCount)
    , pushConstantRangeCount(pCreateInfo->pushConstantRangeCount)
{
	//char *hostMem = reinterpret_cast<char *>(mem);

	//size_t setLayoutsSize = pCreateInfo->setLayoutCount * sizeof(DescriptorSetLayout *);
	//descriptorSetLayouts = reinterpret_cast<const DescriptorSetLayout **>(hostMem);
	//for(uint32_t i = 0; i < pCreateInfo->setLayoutCount; i++)
	//{
	//	descriptorSetLayouts[i] = vk::Cast(pCreateInfo->pSetLayouts[i]);
	//}
	//hostMem += setLayoutsSize;

	//size_t pushConstantRangesSize = pCreateInfo->pushConstantRangeCount * sizeof(VkPushConstantRange);
	//pushConstantRanges = reinterpret_cast<VkPushConstantRange *>(hostMem);
	//memcpy(pushConstantRanges, pCreateInfo->pPushConstantRanges, pushConstantRangesSize);
	//hostMem += pushConstantRangesSize;

	//dynamicOffsetBaseIndices = reinterpret_cast<uint32_t *>(hostMem);
	//uint32_t dynamicOffsetBase = 0;
	//for(uint32_t i = 0; i < descriptorSetCount; i++)
	//{
	//	uint32_t dynamicDescriptorCount = descriptorSetLayouts[i]->getDynamicDescriptorCount();
	//	ASSERT_OR_RETURN((dynamicOffsetBase + dynamicDescriptorCount) <= MAX_DESCRIPTOR_SET_COMBINED_BUFFERS_DYNAMIC);
	//	dynamicOffsetBaseIndices[i] = dynamicOffsetBase;
	//	dynamicOffsetBase += dynamicDescriptorCount;
	//}

	//uint32_t *hostMem = reinterpret_cast<char *>(mem);

	//size_t setLayoutsSize = pCreateInfo->setLayoutCount * sizeof(DescriptorSetLayout *);
	//descriptorSetLayouts = reinterpret_cast<const DescriptorSetLayout **>(hostMem);
	//for(uint32_t i = 0; i < pCreateInfo->setLayoutCount; i++)
	//{
	//	descriptorSetLayouts[i] = vk::Cast(pCreateInfo->pSetLayouts[i]);
	//}
	//hostMem += setLayoutsSize;

	Binding *bindingStorage = reinterpret_cast<Binding *>(mem);
	uint32_t dynamicOffsetIndex = 0;
	for(uint32_t i = 0; i < pCreateInfo->setLayoutCount; i++)
	{
		const vk::DescriptorSetLayout *setLayout = vk::Cast(pCreateInfo->pSetLayouts[i]);
		uint32_t bindingsArraySize = setLayout->getBindingsArraySize();
		sets[i] = bindingStorage;
		bindingStorage += bindingsArraySize;

		//dynamicDescriptorCount[i] = setLayout->getDynamicDescriptorCount();

		for(uint32_t j = 0; j < bindingsArraySize; j++)
		{
			sets[i][j].descriptorType = setLayout->getDescriptorType(j);
			sets[i][j].offset = setLayout->getBindingOffset(j);
			sets[i][j].dynamicOffsetIndex = dynamicOffsetIndex;

			if(DescriptorSetLayout::IsDescriptorDynamic(sets[i][j].descriptorType))
			{
				dynamicOffsetIndex += setLayout->getDescriptorCount(j);
			}
		}
	}

	size_t pushConstantRangesSize = pCreateInfo->pushConstantRangeCount * sizeof(VkPushConstantRange);
	pushConstantRanges = reinterpret_cast<VkPushConstantRange *>(bindingStorage);
	memcpy(pushConstantRanges, pCreateInfo->pPushConstantRanges, pushConstantRangesSize);
	//hostMem += pushConstantRangesSize;
}

void PipelineLayout::destroy(const VkAllocationCallbacks *pAllocator)
{
	vk::deallocate(sets[0], pAllocator);  // pushConstantRanges are in the same allocation
}

size_t PipelineLayout::ComputeRequiredAllocationSize(const VkPipelineLayoutCreateInfo *pCreateInfo)
{
	uint32_t bindingsCount = 0;
	for(uint32_t i = 0; i < pCreateInfo->setLayoutCount; i++)
	{
		bindingsCount += vk::Cast(pCreateInfo->pSetLayouts[i])->getBindingsArraySize();
	}

	return bindingsCount * sizeof(Binding) +                                   // sets[]
	       pCreateInfo->pushConstantRangeCount * sizeof(VkPushConstantRange);  // pushConstantRanges[]

	//	return (pCreateInfo->setLayoutCount * sizeof(DescriptorSetLayout *)) +        // descriptorSetLayouts[]
	//	       (pCreateInfo->pushConstantRangeCount * sizeof(VkPushConstantRange)) +  // pushConstantRanges[]
	//	       (pCreateInfo->setLayoutCount * sizeof(uint32_t));                      // dynamicOffsetBaseIndices[]
}

size_t PipelineLayout::getDescriptorSetCount() const
{
	return descriptorSetCount;
}

//uint32_t PipelineLayout::getDynamicDescriptorCount(uint32_t setNumber) const
//{
//	//	return getDescriptorSetLayout(setNumber)->getDynamicDescriptorCount();
//	ASSERT(setNumber < descriptorSetCount);
//	return dynamicDescriptorCount[setNumber];
//}

//uint32_t PipelineLayout::getDynamicOffsetBaseIndex(uint32_t setNumber) const
//{
//	return dynamicOffsetBaseIndices[setNumber];
//}

uint32_t PipelineLayout::getDynamicOffsetIndex(uint32_t setNumber, uint32_t bindingNumber) const
{
	ASSERT(setNumber < descriptorSetCount /* && sets[setNumber]*/);
	return /*getDynamicOffsetBaseIndex(setNumber) + */ sets[setNumber][bindingNumber].dynamicOffsetIndex;
}

uint32_t PipelineLayout::getBindingOffset(uint32_t setNumber, uint32_t bindingNumber) const
{
	//	return getDescriptorSetLayout(setNumber)->getBindingOffset(bindingNumber);
	ASSERT(setNumber < descriptorSetCount /* && sets[setNumber]*/);
	return sets[setNumber][bindingNumber].offset;
}

VkDescriptorType PipelineLayout::getDescriptorType(uint32_t setNumber, uint32_t bindingNumber) const
{
	//return getDescriptorSetLayout(setNumber)->getDescriptorType(bindingNumber);
	ASSERT(setNumber < descriptorSetCount /* && sets[setNumber]*/);
	return sets[setNumber][bindingNumber].descriptorType;
}

uint32_t PipelineLayout::getDescriptorSize(uint32_t setNumber, uint32_t bindingNumber) const
{
	return DescriptorSetLayout::GetDescriptorSize(getDescriptorType(setNumber, bindingNumber));
}

bool PipelineLayout::isDescriptorDynamic(uint32_t setNumber, uint32_t bindingNumber) const
{
	return DescriptorSetLayout::IsDescriptorDynamic(getDescriptorType(setNumber, bindingNumber));
}

//DescriptorSetLayout const *PipelineLayout::getDescriptorSetLayout(size_t descriptorSet) const
//{
//	ASSERT(descriptorSet < descriptorSetCount);
//	return descriptorSetLayouts[descriptorSet];
//}

}  // namespace vk
