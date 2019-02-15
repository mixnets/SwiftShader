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
	size_t allocationSize = pCreateInfo->bindingCount * sizeof(VkDescriptorSetLayoutBinding);

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
		return sizeof(VkDescriptorBufferInfo);
	case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
	case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
	case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
	case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
		return sizeof(VkBufferView);
	default:
		UNIMPLEMENTED("Unsupported Descriptor Type");
	}

	return 0;
}

size_t DescriptorSetLayout::getSize() const
{
	size_t size = sizeof(bindingCount);
	for(uint32_t i = 0; i < bindingCount; i++)
	{
		size += sizeof(BindingHeader) + bindings[i].descriptorCount * GetDescriptorSize(bindings[i].descriptorType);
	}
	return size;
}

void DescriptorSetLayout::initialize(VkDescriptorSet descriptorSet)
{
	char* mem = reinterpret_cast<char*>(descriptorSet);
	*reinterpret_cast<uint32_t*>(mem) = bindingCount;
	mem += sizeof(uint32_t);

	for(uint32_t i = 0; i < bindingCount; i++)
	{
		BindingHeader* header = reinterpret_cast<BindingHeader*>(mem);
		header->binding = bindings[i].binding;
		header->descriptorCount = bindings[i].descriptorCount;
		header->descriptorType = bindings[i].descriptorType;

		mem += sizeof(BindingHeader);

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
	uint32_t bindingCount = *reinterpret_cast<uint32_t*>(descriptorWrites.dstSet);
	char* mem = reinterpret_cast<char*>(descriptorWrites.dstSet) + sizeof(uint32_t);

	size_t typeSize = GetDescriptorSize(descriptorWrites.descriptorType);
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

	bool bindingFound = false;
	uint32_t bindingsLeft = descriptorWrites.descriptorCount;
	for(uint32_t i = 0; (i < bindingCount) && (bindingsLeft > 0); i++)
	{
		BindingHeader* bindingHeader = reinterpret_cast<BindingHeader*>(mem);
		mem += sizeof(BindingHeader);

		bool initialBinding = (bindingHeader->binding == descriptorWrites.dstBinding);
		bindingFound |= initialBinding;
		if(bindingFound)
		{
			ASSERT(descriptorWrites.descriptorType == bindingHeader->descriptorType);
			
			// If the dstBinding has fewer than descriptorCount array elements remaining
			// starting from dstArrayElement, then the remainder will be used to update
			// the subsequent binding - dstBinding+1 starting at array element zero. If
			// a binding has a descriptorCount of zero, it is skipped. This behavior
			// applies recursively, with the update affecting consecutive bindings as
			// needed to update all descriptorCount descriptors.
			uint32_t bindingsToWrite = bindingHeader->descriptorCount;
			char* memToWrite = mem;
			if(initialBinding)
			{
				ASSERT(bindingsToWrite > descriptorWrites.dstArrayElement);
				bindingsToWrite -= descriptorWrites.dstArrayElement;
				memToWrite += descriptorWrites.dstArrayElement * typeSize;
			}
			bindingsToWrite = std::min(bindingsLeft, bindingsToWrite);
			bindingsLeft -= bindingsToWrite;

			size_t memSize = typeSize * bindingsToWrite;
			memcpy(memToWrite, memToRead, memSize);
			memToRead += memSize;

		}

		mem += bindingHeader->descriptorCount * GetDescriptorSize(bindingHeader->descriptorType);
	}
}

void DescriptorSetLayout::UpdateDescriptorSet(const VkCopyDescriptorSet& descriptorCopies)
{
	// Find initial destination binding
	uint32_t dstBindingCount = *reinterpret_cast<uint32_t*>(descriptorCopies.dstSet);
	char* dstMem = reinterpret_cast<char*>(descriptorCopies.dstSet) + sizeof(uint32_t);
	uint32_t dstIndex = 0;
	for(; dstIndex < dstBindingCount; dstIndex++)
	{
		BindingHeader* bindingHeader = reinterpret_cast<BindingHeader*>(dstMem);
		if(bindingHeader->binding == descriptorCopies.dstBinding)
		{
			break;
		}
		dstMem += sizeof(BindingHeader) + bindingHeader->descriptorCount * GetDescriptorSize(bindingHeader->descriptorType);
	}

	// Find initial source binding
	uint32_t srcBindingCount = *reinterpret_cast<uint32_t*>(descriptorCopies.srcSet);
	char* srcMem = reinterpret_cast<char*>(descriptorCopies.srcSet) + sizeof(uint32_t);
	uint32_t srcIndex = 0;
	for(; srcIndex < srcBindingCount; srcIndex++)
	{
		BindingHeader* bindingHeader = reinterpret_cast<BindingHeader*>(srcMem);
		if(bindingHeader->binding == descriptorCopies.srcBinding)
		{
			break;
		}
		srcMem += sizeof(BindingHeader) + bindingHeader->descriptorCount * GetDescriptorSize(bindingHeader->descriptorType);
	}
	srcMem = reinterpret_cast<char*>(descriptorCopies.srcSet) + sizeof(uint32_t);

	// Copy source bindings to destination bindings
	bool initialDstBinding = true;
	bool initialSrcBinding = true;
	uint32_t bindingsLeft = descriptorCopies.descriptorCount;
	for(; (dstIndex < dstBindingCount) && (bindingsLeft > 0); dstIndex++)
	{
		BindingHeader* dstBindingHeader = reinterpret_cast<BindingHeader*>(dstMem);
		dstMem += sizeof(BindingHeader);

		size_t dstTypeSize = GetDescriptorSize(dstBindingHeader->descriptorType);
		char* memToWrite = dstMem;

		// Offset inital destination binding by array element
		uint32_t dstBindingsLeft = dstBindingHeader->descriptorCount;
		if(initialDstBinding)
		{
			ASSERT(dstBindingHeader->descriptorCount > descriptorCopies.dstArrayElement);
			dstBindingsLeft -= descriptorCopies.dstArrayElement;
			memToWrite += descriptorCopies.dstArrayElement * dstTypeSize;
			initialDstBinding = false;
		}

		for(; (srcIndex < srcBindingCount) && (dstBindingsLeft > 0); srcIndex++)
		{
			BindingHeader* srcBindingHeader = reinterpret_cast<BindingHeader*>(srcMem);
			srcMem += sizeof(BindingHeader);

			ASSERT(srcBindingHeader->descriptorType == dstBindingHeader->descriptorType);

			size_t srcTypeSize = GetDescriptorSize(srcBindingHeader->descriptorType);
			const char* memToRead = srcMem;

			// Offset inital source binding by array element
			uint32_t srcBindingsLeft = std::min(dstBindingsLeft, srcBindingHeader->descriptorCount);
			uint32_t bindingsToWrite = srcBindingsLeft;
			if(initialSrcBinding)
			{
				ASSERT(srcBindingHeader->descriptorCount > descriptorCopies.srcArrayElement);
				srcBindingsLeft -= descriptorCopies.srcArrayElement;
				memToRead += descriptorCopies.srcArrayElement * srcTypeSize;
				initialSrcBinding = false;
			}

			size_t memSize = dstTypeSize * bindingsToWrite;
			memcpy(memToWrite, memToRead, memSize);
			memToWrite += memSize;

			dstBindingsLeft -= srcBindingsLeft;
		}

		bindingsLeft -= dstBindingsLeft;
	}
}

} // namespace vk