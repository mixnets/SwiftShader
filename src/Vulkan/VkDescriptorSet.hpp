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

#include "marl/mutex.h"

namespace vk {

class DescriptorSetLayout;
class ImageView;

class DescriptorSetMetaData
{
public:
	DescriptorSetMetaData(DescriptorSetLayout *layout);
	void increment(uint32_t &bindingNumber, uint32_t &arrayElement) const;
	void prepareForSampling();
	void contentsChanged();
	void storeMetaData(uint32_t bindingNumber, uint32_t arrayElement, ImageView *imageView, bool readOnly);
	void copyTo(uint32_t descriptorCount, uint32_t srcBinding, uint32_t srcArrayElement, uint32_t dstBinding, uint32_t dstArrayElement, DescriptorSetMetaData *dstSet);

private:
	struct MetaDatum
	{
		ImageView *view = nullptr;
		bool readOnly = false;
	};

	marl::mutex mutex;
	std::vector<std::vector<MetaDatum> > data GUARDED_BY(mutex);
};

struct alignas(16) DescriptorSetHeader
{
	DescriptorSetLayout *layout;
	DescriptorSetMetaData *metadata;
};

class alignas(16) DescriptorSet
{
public:
	static inline DescriptorSet *Cast(VkDescriptorSet object)
	{
		return static_cast<DescriptorSet *>(static_cast<void *>(object));
	}

	void init();
	void destroyMetadata();
	void increment(uint32_t &bindingNumber, uint32_t &arrayElement);
	void prepareForSampling() const;
	void contentsChanged() const;
	void storeMetaData(uint32_t bindingNumber, uint32_t arrayElement, ImageView *imageView, bool readOnly);
	void copyMetadata(uint32_t descriptorCount, uint32_t srcBinding, uint32_t srcArrayElement, uint32_t dstBinding, uint32_t dstArrayElement, DescriptorSet *dstSet) const;

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
