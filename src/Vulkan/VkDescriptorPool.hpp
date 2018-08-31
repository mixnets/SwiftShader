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

#ifndef VK_DESCRIPTOR_POOL_HPP_
#define VK_DESCRIPTOR_POOL_HPP_

#include "VkDescriptorSet.hpp"
#include <set>

namespace vk
{

VkClass(DescriptorPool)
{
public:
	DescriptorPool(const VkAllocationCallbacks* pAllocator, const VkDescriptorPoolCreateInfo* pCreateInfo) :
		maxSets(pCreateInfo->maxSets)
	{
		pools = static_cast<Pool*>(vk::allocate(sizeof(Pool) * pCreateInfo->poolSizeCount, pAllocator, GetAllocationScope()));
		sets = static_cast<DescriptorSet*>(vk::allocate(sizeof(DescriptorSet) * maxSets, pAllocator, GetAllocationScope()));

		for(uint32_t i = 0; i < pCreateInfo->poolSizeCount; i++)
		{
			const VkDescriptorPoolSize& poolSize = pCreateInfo->pPoolSizes[i];
			pools[i].type = poolSize.type;
			pools[i].count = poolSize.descriptorCount;
		}
	}

	~DescriptorPool() = delete;

	void destroy(const VkAllocationCallbacks* pAllocator) override
	{
	}

	void allocate(uint32_t descriptorSetCount, const VkDescriptorSetLayout* pSetLayouts, VkDescriptorSet* pDescriptorSets)
	{
		for(uint32_t i = 0; i < descriptorSetCount; i++)
		{
			size_t freeIndex = getFreeSetIndex();
			pDescriptorSets[i] = *(new (sets + freeIndex) DescriptorSet(pSetLayouts[i]));
		}
	}

	void free(uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets)
	{
		for(uint32_t i = 0; i < descriptorSetCount; i++)
		{
			auto descriptorSet = Cast(pDescriptorSets[i]);
			size_t index = (descriptorSet - sets) / sizeof(DescriptorSet);
			used_set_indices.erase(index);
		}
	}

	void reset()
	{
		// recycles all of the resources from all of the descriptor sets
	}

private:
	size_t getFreeSetIndex() const
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

	struct Pool
	{
		VkDescriptorType type = VK_DESCRIPTOR_TYPE_SAMPLER;
		uint32_t count = 0;
	};
	Pool* pools = nullptr;
	DescriptorSet* sets = nullptr;
	std::set<size_t> used_set_indices;

	uint32_t maxSets = 0;
	uint32_t numSets = 0;
};

} // namespace vk

#endif // VK_DESCRIPTOR_POOL_HPP_
