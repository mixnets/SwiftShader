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
#include "VkDescriptorSetLayout.hpp"
#include <cstring>

namespace vk
{
	DescriptorPool::DescriptorPool(const VkDescriptorPoolCreateInfo* pCreateInfo, void* mem) :
		maxSets(pCreateInfo->maxSets)
	{
		// TODO: Add checks for descriptor set types using the info from pCreateInfo->pPoolSizes

		char* hostMemory = reinterpret_cast<char*>(mem);
		sets = reinterpret_cast<DescriptorSet*>(hostMemory);
		memset(sets, 0, sizeof(DescriptorSet) * maxSets);
	}

	void DescriptorPool::destroy(const VkAllocationCallbacks* pAllocator)
	{
		for(uint32_t i = 0; i < maxSets; i++)
		{
			vk::deallocate(sets[i].set, DEVICE_MEMORY);
		}
		vk::deallocate(sets, pAllocator);
	}

	size_t DescriptorPool::ComputeRequiredAllocationSize(const VkDescriptorPoolCreateInfo* pCreateInfo)
	{
		return sizeof(DescriptorSet) * pCreateInfo->maxSets;
	}

	VkResult DescriptorPool::allocate(uint32_t descriptorSetCount, const VkDescriptorSetLayout* pSetLayouts, VkDescriptorSet* pDescriptorSets)
	{
		for(uint32_t i = 0; i < descriptorSetCount; i++)
		{
			pDescriptorSets[i] = VK_NULL_HANDLE;
		}

		for(uint32_t i = 0; i < descriptorSetCount; i++)
		{
			size_t freeIndex = getFreeSetIndex();
			if(freeIndex >= maxSets)
			{
				return VK_ERROR_OUT_OF_POOL_MEMORY;
			}

			size_t requiredSize = Cast(pSetLayouts[i])->getSize();
			if(sets[freeIndex].size < requiredSize)
			{
				vk::deallocate(sets[freeIndex].set, DEVICE_MEMORY);
				sets[freeIndex].set = static_cast<VkDescriptorSet>(
					vk::allocate(requiredSize, REQUIRED_MEMORY_ALIGNMENT, DEVICE_MEMORY));
				sets[freeIndex].size = requiredSize;
			}
			sets[freeIndex].used = true;
			nbSets++;

			pDescriptorSets[i] = sets[freeIndex].set;
		}

		return VK_SUCCESS;
	}

	void DescriptorPool::free(uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets)
	{
		for(uint32_t i = 0; i < descriptorSetCount; i++)
		{
			for(uint32_t s = 0; s < maxSets; s++)
			{
				if(sets[s].set == pDescriptorSets[i])
				{
					sets[s].used = false;
					nbSets--;
					break;
				}
			}
		}
	}

	VkResult DescriptorPool::reset()
	{
		for(uint32_t i = 0; i < maxSets; i++)
		{
			sets[i].used = false;
		}
		nbSets = 0;

		return VK_SUCCESS;
	}

	size_t DescriptorPool::getFreeSetIndex() const
	{
		for(uint32_t i = nbSets; i < maxSets; i++)
		{
			if(!sets[i].used)
			{
				return i;
			}
		}
		for(uint32_t i = 0; i < nbSets; i++)
		{
			if(!sets[i].used)
			{
				return i;
			}
		}

		return maxSets; // no set available
	}

} // namespace vk