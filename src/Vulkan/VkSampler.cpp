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

#include "VkSampler.hpp"

#include "VkDevice.hpp"
#include "Device/LRUCache.hpp"
#include <cstring>

namespace vk {

SamplerState::SamplerState(const VkSamplerCreateInfo *pCreateInfo, const vk::SamplerYcbcrConversion *ycbcrConversion)
    : Memset(this, 0)
    , magFilter(pCreateInfo->magFilter)
    , minFilter(pCreateInfo->minFilter)
    , mipmapMode(pCreateInfo->mipmapMode)
    , addressModeU(pCreateInfo->addressModeU)
    , addressModeV(pCreateInfo->addressModeV)
    , addressModeW(pCreateInfo->addressModeW)
    , mipLodBias(pCreateInfo->mipLodBias)
    , anisotropyEnable(pCreateInfo->anisotropyEnable)
    , maxAnisotropy(pCreateInfo->maxAnisotropy)
    , compareEnable(pCreateInfo->compareEnable)
    , compareOp(pCreateInfo->compareOp)
    , minLod(ClampLod(pCreateInfo->minLod))
    , maxLod(ClampLod(pCreateInfo->maxLod))
    , borderColor(pCreateInfo->borderColor)
    , unnormalizedCoordinates(pCreateInfo->unnormalizedCoordinates)
{

	if(ycbcrConversion)
	{
		ycbcrModel = ycbcrConversion->ycbcrModel;
		studioSwing = (ycbcrConversion->ycbcrRange == VK_SAMPLER_YCBCR_RANGE_ITU_NARROW);
		swappedChroma = (ycbcrConversion->components.r != VK_COMPONENT_SWIZZLE_R);
	}
}

//bool SamplerState::Compare::operator()(const SamplerState &a, const SamplerState &b) const
//{
//	static_assert(sw::is_memcmparable<Sampler>::value, "Cannot memcmp Sampler");
//	return ::memcmp(&a, &b, sizeof(SamplerState)) < 0;
//}

Sampler::Sampler(const VkSamplerCreateInfo *pCreateInfo, void *mem, Device *device, const vk::SamplerYcbcrConversion *ycbcrConversion)
    : SamplerState(pCreateInfo, ycbcrConversion)
    , id(device->getSamplerIndexer()->index(this))
    , device(device)
{
	//	auto *samplerIndexer = device->getSamplerIndexer();
	//	id = samplerIndexer->index(this);
}

Sampler::~Sampler()
{
	auto *samplerIndexer = device->getSamplerIndexer();
	samplerIndexer->remove(this);
}

}  // namespace vk
