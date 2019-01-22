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
#include <cstring>

namespace vk
{

DescriptorSetLayout::DescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo* pCreateInfo, void* mem) :
	flags(pCreateInfo->flags), bindingCount(pCreateInfo->bindingCount), bindings(reinterpret_cast<VkDescriptorSetLayoutBinding*>(mem))
{
	char* host_memory = static_cast<char*>(mem) + bindingCount * sizeof(VkDescriptorSetLayoutBinding);

	for(uint32_t i = 0; i < bindingCount; i++)
	{
		bindings[i] = pCreateInfo->pBindings[i];
		if(useImmutableSamplers(bindings[i]))
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
		if(useImmutableSamplers(bindings[i]))
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
		if(useImmutableSamplers(pCreateInfo->pBindings[i]))
		{
			allocationSize += pCreateInfo->pBindings[i].descriptorCount * sizeof(VkSampler);
		}
	}

	return allocationSize;
}

bool DescriptorSetLayout::useImmutableSamplers(const VkDescriptorSetLayoutBinding& binding)
{
	return (((binding.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER) ||
	         (binding.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)) &&
	        (binding.pImmutableSamplers != nullptr));
}

size_t DescriptorSetLayout::getSize() const
{
	size_t size = 0;
	for(uint32_t i = 0; i < bindingCount; i++)
	{
		size += sizeof(bindings[i].binding);
		switch(bindings[i].descriptorType)
		{
		case VK_DESCRIPTOR_TYPE_SAMPLER:
		case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
		case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
		case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
		case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
		case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
		case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
		case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
		case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
		case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
		case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
			// FIXME: compute a proper size if a structure requires more than a 4 byte index
			size += bindings[i].descriptorCount * sizeof(uint32_t);
			break;
		case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT:
			// According to the Vulkan spec:
			// "if descriptorType is VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT [...]
			//  descriptorCount is the size in bytes of the inline uniform block"
			size += bindings[i].descriptorCount;
			break;
		default:
			UNIMPLEMENTED();
		}
	}
	return size;
}

} // namespace vk