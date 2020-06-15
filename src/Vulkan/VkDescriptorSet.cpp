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
#include "VkMemory.hpp"

namespace vk {

bool DescriptorSetHeader::allocateMemoryOwners(bool forceAllocate)
{
	marl::lock lock(mutex);

	if(!memoryOwners && !forceAllocate)
	{
		return false;
	}

	if(memoryOwners)
	{
		return true;
	}

	bindingsArraySize = layout->getBindingsArraySize();
	size_t allocSize = bindingsArraySize * (sizeof(MemoryOwner *) + sizeof(uint32_t));
	for(uint32_t i = 0; i < bindingsArraySize; i++)
	{
		allocSize += layout->getDescriptorCount(i) * sizeof(MemoryOwner);
	}

	// Perform single allocation for all members
	uint8_t *mem = static_cast<uint8_t *>(vk::allocate(allocSize, REQUIRED_MEMORY_ALIGNMENT, DEVICE_MEMORY));

	memoryOwners = reinterpret_cast<MemoryOwner **>(mem);
	mem += bindingsArraySize * sizeof(MemoryOwner *);

	descriptorCount = reinterpret_cast<uint32_t *>(mem);
	mem += bindingsArraySize * sizeof(uint32_t);

	for(uint32_t i = 0; i < bindingsArraySize; i++)
	{
		descriptorCount[i] = layout->getDescriptorCount(i);
		memoryOwners[i] = reinterpret_cast<MemoryOwner *>(mem);
		mem += descriptorCount[i] * sizeof(MemoryOwner);
	}

	return true;
}

void DescriptorSetHeader::freeMemoryOwners()
{
	marl::lock lock(mutex);

	vk::deallocate(memoryOwners, DEVICE_MEMORY);
	memoryOwners = nullptr;
	bindingsArraySize = 0;
	descriptorCount = nullptr;
}

void DescriptorSetHeader::increment(uint32_t &bindingNumber, uint32_t &arrayElement) const
{
	ASSERT(bindingNumber < bindingsArraySize);

	if(arrayElement >= (descriptorCount[bindingNumber] - 1))
	{
		++bindingNumber;
		arrayElement = 0;
	}
	else
	{
		++arrayElement;
	}
}

void DescriptorSetHeader::prepareForSampling()
{
	marl::lock lock(mutex);

	for(uint32_t i = 0; i < bindingsArraySize; i++)
	{
		for(uint32_t j = 0; j < descriptorCount[i]; j++)
		{
			if(memoryOwners[i][j].view)
			{
				memoryOwners[i][j].view->prepareForSampling();
			}
		}
	}
}

void DescriptorSetHeader::contentsChanged()
{
	marl::lock lock(mutex);

	for(uint32_t i = 0; i < bindingsArraySize; i++)
	{
		for(uint32_t j = 0; j < descriptorCount[i]; j++)
		{
			if(memoryOwners[i][j].view && !memoryOwners[i][j].readOnly)
			{
				memoryOwners[i][j].view->contentsChanged();
			}
		}
	}
}

void DescriptorSetHeader::storeMemoryOwner(uint32_t bindingNumber, uint32_t arrayElement, ImageView *imageView, bool readOnly)
{
	// No new information to add if imageView is null or doesn't require preprocessing
	if(allocateMemoryOwners(imageView && imageView->requiresPreprocessing()))
	{
		marl::lock lock(mutex);
		memoryOwners[bindingNumber][arrayElement] = { imageView, readOnly };
	}
}

void DescriptorSetHeader::copyTo(uint32_t srcBinding, uint32_t srcArrayElement, DescriptorSetHeader &dstSet, uint32_t dstBinding, uint32_t dstArrayElement, uint32_t descriptorCount)
{
	if(&dstSet == this)
	{
		// Don't allocate memory owners to copy information to the
		// same descriptor set if there are no memory owners present
		if(!allocateMemoryOwners(false))
		{
			return;
		}

		marl::lock lock(mutex);
		for(uint32_t i = 0; i < descriptorCount; ++i)
		{
			memoryOwners[dstBinding][dstArrayElement] = memoryOwners[srcBinding][srcArrayElement];
			increment(srcBinding, srcArrayElement);
			increment(dstBinding, dstArrayElement);
		}
	}
	else
	{
		{
			marl::lock dstLock(dstSet.mutex);
			// Don't allocate memory owners to copy information to the destination
			// descriptor set if there are no memory owners present in either set
			if(!allocateMemoryOwners(dstSet.memoryOwners != nullptr))
			{
				return;
			}
		}

		// The source has memory owners, make sure the destination also has them
		dstSet.allocateMemoryOwners();

		marl::lock lock(mutex);
		marl::lock dstLock(dstSet.mutex);
		for(uint32_t i = 0; i < descriptorCount; ++i)
		{
			dstSet.memoryOwners[dstBinding][dstArrayElement] = memoryOwners[srcBinding][srcArrayElement];
			increment(srcBinding, srcArrayElement);
			dstSet.increment(dstBinding, dstArrayElement);
		}
	}
}

}  // namespace vk