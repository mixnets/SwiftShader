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
#include "VkDescriptorSet.hpp"

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
		return sizeof(DescriptorSet::Sampler);
	case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
		return sizeof(DescriptorSet::CombinedImageSampler);
	case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
		return sizeof(DescriptorSet::SampledImage);
	case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
		return sizeof(DescriptorSet::StorageImage);
	case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
		return sizeof(DescriptorSet::UniformTexelBuffer);
	case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
		return sizeof(DescriptorSet::StorageTexelBuffer);
	case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
		return sizeof(DescriptorSet::UniformBuffer);
	case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
		return sizeof(DescriptorSet::StorageBuffer);
	case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
		return sizeof(DescriptorSet::UniformBufferDynamic);
	case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
		return sizeof(DescriptorSet::StorageBufferDynamic);
	case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
		return sizeof(DescriptorSet::InputAttachment);
	default:
		UNIMPLEMENTED("Unsupported Descriptor Type");
	}

	return 0;
}

ptrdiff_t DescriptorSetLayout::bindingOffset(VkDescriptorType type, uint32_t bindingIndex)
{
	size_t offset = sizeof(DescriptorSet::Header);

	for(uint32_t i = 0; i < bindingCount; i++)
	{
		const auto& binding = bindings[i];
		if (binding.descriptorType == type) {
			if (bindingIndex < binding.descriptorCount) {
				return offset + bindingIndex * GetDescriptorSize(binding.descriptorType);
			}
			bindingIndex -= binding.descriptorCount;
		}

		offset += binding.descriptorCount * GetDescriptorSize(binding.descriptorType);
	}

	UNREACHABLE("Binding not found");
	return 0;
}

size_t DescriptorSetLayout::getSize() const
{
	size_t size = sizeof(DescriptorSet::Header);
	for(uint32_t i = 0; i < bindingCount; i++)
	{
		size += bindings[i].descriptorCount * GetDescriptorSize(bindings[i].descriptorType);
	}
	return size;
}

} // namespace vk