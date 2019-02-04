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

#include "VkDescriptorSetLayout.hpp"
#include <algorithm>
#include <cstring>

namespace
{

static bool UsesImmutableSamplers(const VkDescriptorSetLayoutBinding& binding)
{
	return (((binding.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER) ||
	        (binding.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)) &&
	        (binding.pImmutableSamplers != nullptr));
}

}

namespace vk
{

DescriptorSetLayout::DescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo* pCreateInfo, void* mem) :
	flags(pCreateInfo->flags), bindingCount(pCreateInfo->bindingCount), bindings(reinterpret_cast<VkDescriptorSetLayoutBinding*>(mem))
{
	char* host_memory = static_cast<char*>(mem) + bindingCount * sizeof(VkDescriptorSetLayoutBinding);
	bindingOffsets = reinterpret_cast<size_t*>(host_memory);
	host_memory += bindingCount * sizeof(size_t);

	size_t offset = 0;
	for(uint32_t i = 0; i < bindingCount; i++)
	{
		bindings[i] = pCreateInfo->pBindings[i];
		if(UsesImmutableSamplers(bindings[i]))
		{
			size_t immutableSamplersSize = bindings[i].descriptorCount * sizeof(VkSampler);
			bindings[i].pImmutableSamplers = reinterpret_cast<const VkSampler*>(host_memory);
			host_memory += immutableSamplersSize;
			memcpy(const_cast<VkSampler*>(bindings[i].pImmutableSamplers),
			       pCreateInfo->pBindings[i].pImmutableSamplers,
			       immutableSamplersSize);
		}
		else
		{
			bindings[i].pImmutableSamplers = nullptr;
		}
		bindingOffsets[i] = offset;
		offset += bindings[i].descriptorCount * GetDescriptorSize(bindings[i].descriptorType);
	}
}

void DescriptorSetLayout::destroy(const VkAllocationCallbacks* pAllocator)
{
	for(uint32_t i = 0; i < bindingCount; i++)
	{
		if(UsesImmutableSamplers(bindings[i]))
		{
			// A single allocation is used for all immutable samplers, so only a single deallocation is needed.
			vk::deallocate(const_cast<VkSampler*>(bindings[i].pImmutableSamplers), pAllocator);
			break;
		}
	}

	vk::deallocate(bindings, pAllocator);
}

size_t DescriptorSetLayout::ComputeRequiredAllocationSize(const VkDescriptorSetLayoutCreateInfo* pCreateInfo)
{
	size_t allocationSize = pCreateInfo->bindingCount * (sizeof(VkDescriptorSetLayoutBinding) + sizeof(size_t));

	for(uint32_t i = 0; i < pCreateInfo->bindingCount; i++)
	{
		if(UsesImmutableSamplers(pCreateInfo->pBindings[i]))
		{
			allocationSize += pCreateInfo->pBindings[i].descriptorCount * sizeof(VkSampler);
		}
	}

	return allocationSize;
}

size_t DescriptorSetLayout::GetDescriptorSize(VkDescriptorType type)
{
	switch(type)
	{
	case VK_DESCRIPTOR_TYPE_SAMPLER:
	case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
	case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
	case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
	case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
		return sizeof(VkDescriptorImageInfo);
	case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
	case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
		return sizeof(VkBufferView);
	case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
	case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
	case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
	case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
		return sizeof(VkDescriptorBufferInfo);
	default:
		UNIMPLEMENTED("Unsupported Descriptor Type");
	}

	return 0;
}

size_t DescriptorSetLayout::getSize() const
{
	size_t size = 0;
	for(uint32_t i = 0; i < bindingCount; i++)
	{
		size += bindings[i].descriptorCount * GetDescriptorSize(bindings[i].descriptorType);
	}
	return size;
}

const VkDescriptorSetLayoutBinding& DescriptorSetLayout::getBindingInfo(uint32_t binding, size_t* byteOffset) const
{
	for(uint32_t i = 0; i < bindingCount; i++)
	{
		if(binding == bindings[i].binding)
		{
			*byteOffset = bindingOffsets[i];
			return bindings[i];
		}
	}

	ASSERT(false); // Bindings should always be found
	*byteOffset = 0;
	return bindings[0];
}

void DescriptorSetLayout::initialize(VkDescriptorSet descriptorSet)
{
	// Use a pointer to this descriptor set layout as the descriptor set's header
	*reinterpret_cast<DescriptorSetLayout**>(descriptorSet) = this;
	char* mem = reinterpret_cast<char*>(descriptorSet) + sizeof(DescriptorSetLayout*);

	for(uint32_t i = 0; i < bindingCount; i++)
	{
		size_t typeSize = GetDescriptorSize(bindings[i].descriptorType);
		if(UsesImmutableSamplers(bindings[i]))
		{
			for(uint32_t j = 0; j < bindings[i].descriptorCount; j++)
			{
				VkDescriptorImageInfo* imageInfo = reinterpret_cast<VkDescriptorImageInfo*>(mem);
				imageInfo->sampler = bindings[i].pImmutableSamplers[j];
				mem += typeSize;
			}
		}
		else
		{
			mem += bindings[i].descriptorCount * typeSize;
		}
	}
}

void DescriptorSetLayout::UpdateDescriptorSet(const VkWriteDescriptorSet& descriptorWrites)
{
	DescriptorSetLayout* dstLayout = *reinterpret_cast<DescriptorSetLayout**>(descriptorWrites.dstSet);
	ASSERT(dstLayout);

	size_t byteOffset = 0;
	const VkDescriptorSetLayoutBinding& dstBinding = dstLayout->getBindingInfo(descriptorWrites.dstBinding, &byteOffset);
	ASSERT(descriptorWrites.descriptorType == dstBinding.descriptorType);

	size_t typeSize = GetDescriptorSize(descriptorWrites.descriptorType);
	char* memToWrite = reinterpret_cast<char*>(descriptorWrites.dstSet) +
	                   sizeof(DescriptorSetLayout*) + byteOffset +
	                   typeSize * descriptorWrites.dstArrayElement;

	const char* memToRead = nullptr;
	switch(descriptorWrites.descriptorType)
	{
	case VK_DESCRIPTOR_TYPE_SAMPLER:
	case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
	case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
	case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
	case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
		memToRead = reinterpret_cast<const char*>(descriptorWrites.pImageInfo);
		break;
	case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
	case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
		memToRead = reinterpret_cast<const char*>(descriptorWrites.pTexelBufferView);
		break;
	case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
	case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
	case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
	case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
		memToRead = reinterpret_cast<const char*>(descriptorWrites.pBufferInfo);
		break;
	default:
		UNIMPLEMENTED();
		break;
	}

	// If the dstBinding has fewer than descriptorCount array elements remaining
	// starting from dstArrayElement, then the remainder will be used to update
	// the subsequent binding - dstBinding+1 starting at array element zero. If
	// a binding has a descriptorCount of zero, it is skipped. This behavior
	// applies recursively, with the update affecting consecutive bindings as
	// needed to update all descriptorCount descriptors.
	memcpy(memToWrite, memToRead, typeSize * descriptorWrites.descriptorCount);
}

void DescriptorSetLayout::UpdateDescriptorSet(const VkCopyDescriptorSet& descriptorCopies)
{
	DescriptorSetLayout* srcLayout = *reinterpret_cast<DescriptorSetLayout**>(descriptorCopies.srcSet);
	ASSERT(srcLayout);

	size_t srcByteOffset = 0;
	const VkDescriptorSetLayoutBinding& srcBinding = srcLayout->getBindingInfo(descriptorCopies.srcBinding, &srcByteOffset);

	size_t srcTypeSize = GetDescriptorSize(srcBinding.descriptorType);
	char* memToRead = reinterpret_cast<char*>(descriptorCopies.srcSet) +
	                  sizeof(DescriptorSetLayout*) + srcByteOffset +
	                  srcTypeSize * descriptorCopies.srcArrayElement;

	DescriptorSetLayout* dstLayout = *reinterpret_cast<DescriptorSetLayout**>(descriptorCopies.dstSet);
	ASSERT(dstLayout);

	size_t dstByteOffset = 0;
	const VkDescriptorSetLayoutBinding& dstBinding = srcLayout->getBindingInfo(descriptorCopies.srcBinding, &dstByteOffset);

	size_t dstTypeSize = GetDescriptorSize(dstBinding.descriptorType);
	char* memToWrite = reinterpret_cast<char*>(descriptorCopies.dstSet) +
	                   sizeof(DescriptorSetLayout*) + dstByteOffset +
	                   dstTypeSize * descriptorCopies.dstArrayElement;

	ASSERT(srcBinding.descriptorType == dstBinding.descriptorType);
	size_t memSize = dstTypeSize * descriptorCopies.descriptorCount;
	memcpy(memToWrite, memToRead, memSize);
}

} // namespace vk