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
#include "VkDescriptorSet.hpp"
#include "VkDescriptorSetLayout.hpp"
#include <algorithm>
#include <memory>

namespace vk
{

DescriptorPool::DescriptorPool(const VkDescriptorPoolCreateInfo* pCreateInfo, void* mem) :
	pool(reinterpret_cast<VkDescriptorSet>(mem)),
	poolSize(ComputeRequiredAllocationSize(pCreateInfo))
{
}

void DescriptorPool::destroy(const VkAllocationCallbacks* pAllocator)
{
	vk::deallocate(pool, pAllocator);
}

size_t DescriptorPool::ComputeRequiredAllocationSize(const VkDescriptorPoolCreateInfo* pCreateInfo)
{
	size_t size = pCreateInfo->maxSets * sizeof(DescriptorSet::Header);

	for(uint32_t i = 0; i < pCreateInfo->poolSizeCount; i++)
	{
		size += pCreateInfo->pPoolSizes[i].descriptorCount * DescriptorSetLayout::GetDescriptorSize(pCreateInfo->pPoolSizes[i].type);
	}

	return size;
}

VkResult DescriptorPool::allocateSets(uint32_t descriptorSetCount, const VkDescriptorSetLayout* pSetLayouts, VkDescriptorSet* pDescriptorSets)
{
	size_t totalSize = 0;
	std::unique_ptr<size_t[]> sizes(new size_t[descriptorSetCount]);
	for(uint32_t i = 0; i < descriptorSetCount; i++)
	{
		pDescriptorSets[i] = VK_NULL_HANDLE;
		auto size = Cast(pSetLayouts[i])->getSize();
		sizes[i] = size;
		totalSize += size;
	}

	if(totalSize > poolSize)
	{
		return VK_ERROR_OUT_OF_POOL_MEMORY;
	}

	// Attempt to allocate single chunk of memory
	if(void* memory = findAvailableMemory(totalSize))
	{
		pDescriptorSets[0] = reinterpret_cast<VkDescriptorSet>(new (memory) DescriptorSet(pSetLayouts[0]));
		for(uint32_t i = 1; i < descriptorSetCount; i++)
		{
			auto ptr = reinterpret_cast<char*>(memory) + sizes[i - 1];
			pDescriptorSets[i] = reinterpret_cast<VkDescriptorSet>(new (ptr) DescriptorSet(pSetLayouts[i]));
			nodes.insert(Node(pDescriptorSets[i], sizes[i]));
		}
		return VK_SUCCESS;
	}

	// Atttempt to allocate each descriptor set separately
	for(uint32_t i = 0; i < descriptorSetCount; i++)
	{
		if(void* memory = findAvailableMemory(sizes[i]))
		{
			pDescriptorSets[i] = reinterpret_cast<VkDescriptorSet>(new (memory) DescriptorSet(pSetLayouts[i]));
		}
		else
		{
			// vkAllocateDescriptorSets can be used to create multiple descriptor sets. If the
			// creation of any of those descriptor sets fails, then the implementation must
			// destroy all successfully created descriptor set objects from this command, set
			// all entries of the pDescriptorSets array to VK_NULL_HANDLE and return the error.
			for(uint32_t j = 0; j < i; j++)
			{
				freeSet(pDescriptorSets[j]);
				pDescriptorSets[j] = VK_NULL_HANDLE;
			}
			return (computeTotalFreeSize() > totalSize) ? VK_ERROR_FRAGMENTED_POOL : VK_ERROR_OUT_OF_POOL_MEMORY;
		}
		nodes.insert(Node(pDescriptorSets[i], sizes[i]));
	}

	return VK_SUCCESS;
}

void* DescriptorPool::findAvailableMemory(size_t size)
{
	if(nodes.empty())
	{
		return pool;
	}

	// First, look for space at the end of the pool
	const auto itLast = nodes.rbegin();
	ptrdiff_t itemStart = reinterpret_cast<char*>(itLast->set) - reinterpret_cast<char*>(pool);
	ptrdiff_t nextItemStart = itemStart + itLast->size;
	size_t freeSpace = poolSize - nextItemStart;
	if(freeSpace >= size)
	{
		return reinterpret_cast<void*>(nextItemStart);
	}

	// Second, look for space at the beginning of the pool
	const auto itBegin = nodes.end();
	freeSpace = reinterpret_cast<char*>(itBegin->set) - reinterpret_cast<char*>(pool);
	if(freeSpace >= size)
	{
		return pool;
	}

	// Finally, look between existing pool items
	const auto itEnd = nodes.end();
	auto nextIt = itBegin;
	++nextIt;
	for(auto it = itBegin; nextIt != itEnd; ++it, ++nextIt)
	{
		auto freeSpaceStart = reinterpret_cast<char*>(it->set) + it->size;
		freeSpace = reinterpret_cast<char*>(nextIt->set) - freeSpaceStart;
		if(freeSpace >= size)
		{
			return freeSpaceStart;
		}
	}

	return nullptr;
}

void DescriptorPool::freeSets(uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets)
{
	for(uint32_t i = 0; i < descriptorSetCount; i++)
	{
		freeSet(pDescriptorSets[i]);
	}
}

void DescriptorPool::freeSet(const VkDescriptorSet descriptorSet)
{
	const auto itEnd = nodes.end();
	auto it = std::find(nodes.begin(), itEnd, descriptorSet);
	if(it != itEnd)
	{
		nodes.erase(it);
	}
}

VkResult DescriptorPool::reset()
{
	nodes.clear();

	return VK_SUCCESS;
}

size_t DescriptorPool::computeTotalFreeSize() const
{
	size_t totalFreeSize = 0;

	// Compute space at the end of the pool
	const auto itLast = nodes.rbegin();
	totalFreeSize += poolSize - ((reinterpret_cast<char*>(itLast->set) - reinterpret_cast<char*>(pool)) + itLast->size);

	// Compute space at the beginning of the pool
	const auto itBegin = nodes.end();
	totalFreeSize += reinterpret_cast<char*>(itBegin->set) - reinterpret_cast<char*>(pool);

	// Finally, look between existing pool items
	const auto itEnd = nodes.end();
	auto nextIt = itBegin;
	++nextIt;
	for(auto it = itBegin; nextIt != itEnd; ++it, ++nextIt)
	{
		totalFreeSize += (reinterpret_cast<char*>(nextIt->set) - reinterpret_cast<char*>(it->set)) - it->size;
	}

	return totalFreeSize;
}

} // namespace vk