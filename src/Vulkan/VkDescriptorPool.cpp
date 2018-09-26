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

#include "VkDescriptorPool.hpp"

namespace vk
{

DescriptorPool::DescriptorPool(const VkDescriptorPoolCreateInfo* pCreateInfo, char* membersMemory) :
	maxSets(pCreateInfo->maxSets)
{
	size_t poolsSize = sizeof(Pool) * pCreateInfo->poolSizeCount;
	pools = reinterpret_cast<Pool*>(membersMemory);
	sets = reinterpret_cast<DescriptorSet*>(membersMemory + poolsSize);

	for(uint32_t i = 0; i < pCreateInfo->poolSizeCount; i++)
	{
		const VkDescriptorPoolSize& poolSize = pCreateInfo->pPoolSizes[i];
		pools[i].type = poolSize.type;
		pools[i].count = poolSize.descriptorCount;
	}
}

void DescriptorPool::destroy(const VkAllocationCallbacks* pAllocator)
{
	vk::deallocate(pools, pAllocator); // sets is in the same allocation
	used_set_indices.clear();
}

size_t DescriptorPool::ComputeRequiredAllocationSize(const VkDescriptorPoolCreateInfo* pCreateInfo)
{
	size_t poolsSize = sizeof(Pool) * pCreateInfo->poolSizeCount;
	size_t setsSize = sizeof(DescriptorSet) * pCreateInfo->maxSets;
	return poolsSize + setsSize;
}

VkResult DescriptorPool::allocate(uint32_t descriptorSetCount, const VkDescriptorSetLayout* pSetLayouts, VkDescriptorSet* pDescriptorSets)
{
	for(uint32_t i = 0; i < descriptorSetCount; i++)
	{
		size_t freeIndex = getFreeSetIndex();
		if(freeIndex >= maxSets)
		{
			return VK_ERROR_OUT_OF_POOL_MEMORY;
		}
		sets[freeIndex].init(pSetLayouts[i]);
		pDescriptorSets[i] = sets[freeIndex];
	}

	return VK_SUCCESS;
}

void DescriptorPool::free(uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets)
{
	for(uint32_t i = 0; i < descriptorSetCount; i++)
	{
		auto descriptorSet = Cast(pDescriptorSets[i]);
		size_t index = (descriptorSet - sets) / sizeof(DescriptorSet);
		used_set_indices.erase(index);
	}
}

void DescriptorPool::reset()
{
	// recycles all of the resources from all of the descriptor sets
}

size_t DescriptorPool::getFreeSetIndex() const
{
	size_t free_index = 0;
	for(auto i : used_set_indices)
	{
		if(i == free_index)
		{
			++free_index;
		}
		else
		{
			break;
		}
	}
	return free_index;
}

} // namespace vk