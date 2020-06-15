// Copyright 2019 The SwiftShader Authors. All Rights Reserved.
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

#ifndef VK_DESCRIPTOR_SET_HPP_
#define VK_DESCRIPTOR_SET_HPP_

// Intentionally not including VkObject.hpp here due to b/127920555
#include "VkConfig.hpp"
#include "Vulkan/VulkanPlatform.hpp"

#include <array>
#include <cstdint>
#include <memory>
#include <vector>

namespace vk {

class DescriptorPool;
class DescriptorSetLayout;
class ImageView;

class DescriptorSetMemoryOwners
{
public:
	DescriptorSetMemoryOwners(DescriptorSetLayout *layout);
	void prepareForSampling();
	void contentsChanged();
	void storeMetaData(uint32_t bindingNumber, uint32_t arrayElement, ImageView *imageView, bool readOnly);
	void copyTo(uint32_t srcBinding, uint32_t srcArrayElement, DescriptorSetMemoryOwners &dstSet, uint32_t dstBinding, uint32_t dstArrayElement, uint32_t descriptorCount);

private:
	void increment(uint32_t &bindingNumber, uint32_t &arrayElement) const;

	struct MemoryOwner
	{
		ImageView *view = nullptr;  // Pointer to the view which owns the memory used by the descriptor set
		bool readOnly = false;      // Whether the descriptor set can be used to write to the memory or only read from it
	};

	std::vector<std::vector<MemoryOwner> > memoryOwners;
};

struct alignas(16) DescriptorSetHeader
{
	DescriptorSetLayout *layout;
	DescriptorPool *pool;
};

class alignas(16) DescriptorSet
{
public:
	static inline DescriptorSet *Cast(VkDescriptorSet object)
	{
		return static_cast<DescriptorSet *>(static_cast<void *>(object));
	}

	void init();
	void prepareForSampling();
	void contentsChanged();
	void storeMetaData(uint32_t bindingNumber, uint32_t arrayElement, ImageView *imageView, bool readOnly);
	void copyMetadata(uint32_t srcBinding, uint32_t srcArrayElement, DescriptorSet *dstSet, uint32_t dstBinding, uint32_t dstArrayElement, uint32_t descriptorCount);

	using Array = std::array<DescriptorSet *, vk::MAX_BOUND_DESCRIPTOR_SETS>;
	using Bindings = std::array<uint8_t *, vk::MAX_BOUND_DESCRIPTOR_SETS>;
	using DynamicOffsets = std::array<uint32_t, vk::MAX_DESCRIPTOR_SET_COMBINED_BUFFERS_DYNAMIC>;

	DescriptorSetHeader header;
	alignas(16) uint8_t data[1];
};

inline DescriptorSet *Cast(VkDescriptorSet object)
{
	return DescriptorSet::Cast(object);
}

}  // namespace vk

#endif  // VK_DESCRIPTOR_SET_HPP_
