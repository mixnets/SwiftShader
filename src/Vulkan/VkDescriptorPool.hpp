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

class DescriptorPool : public Object<DescriptorPool, VkDescriptorPool>
{
public:
	struct Pool
	{
		VkDescriptorType type = VK_DESCRIPTOR_TYPE_SAMPLER;
		uint32_t count = 0;
	};

	DescriptorPool(const VkDescriptorPoolCreateInfo* pCreateInfo, void* mem);
	~DescriptorPool() = delete;
	void destroy(const VkAllocationCallbacks* pAllocator);

	static size_t ComputeRequiredAllocationSize(const VkDescriptorPoolCreateInfo* pCreateInfo);

	VkResult allocate(uint32_t descriptorSetCount, const VkDescriptorSetLayout* pSetLayouts, VkDescriptorSet* pDescriptorSets);
	void free(uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets);
	void reset();

private:
	size_t getFreeSetIndex() const;

	Pool* pools = nullptr;
	DescriptorSet* sets = nullptr;
	bool* usedSets = nullptr;

	uint32_t maxSets = 0;
	uint32_t numSets = 0;
};

static inline DescriptorPool* Cast(VkDescriptorPool object)
{
	return reinterpret_cast<DescriptorPool*>(object);
}

} // namespace vk

#endif // VK_DESCRIPTOR_POOL_HPP_
