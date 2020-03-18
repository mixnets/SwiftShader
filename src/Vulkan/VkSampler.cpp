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
//#include <mutex>
//#include <vector>

namespace vk {

SamplerState::SamplerState(const VkSamplerCreateInfo *pCreateInfo, const vk::SamplerYcbcrConversion *ycbcrConversion)
    : Memset(this, 0), magFilter(pCreateInfo->magFilter)
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
//    , ycbcrConversion(ycbcrConversion)
{
	if(ycbcrConversion)
	{
		ycbcrModel = ycbcrConversion->ycbcrModel;
		studioSwing = (ycbcrConversion->ycbcrRange == VK_SAMPLER_YCBCR_RANGE_ITU_NARROW);
		swappedChroma = (ycbcrConversion->components.r != VK_COMPONENT_SWIZZLE_R);
	}
}

bool SamplerState::Compare::operator()(const SamplerState &a, const SamplerState &b) const
{
	//	static_assert(sw::is_memcmparable<Sampler>::value, "Cannot memcmp Sampler");
	return ::memcmp(&a, &b, sizeof(SamplerState)) < 0;

	/*auto less[](const Sampler &a, const Sampler &b) = {
		if(a.magFilter < b.magFilter) return true;
	    if(a.minFilter < b.minFilter) return true;
	    if(a.mipmapMode < b.mipmapMode) return true;

        return a.swappedChroma && !b.swappedChroma;
    }

    ASSERT(!less(a, a));
    ASSERT(!less(b, b));
    ASSERT(!less(a, b) || !less(b, a));

    return less(a, b);*/
}

Sampler::Sampler(const VkSamplerCreateInfo *pCreateInfo, void *mem, Device *device, const vk::SamplerYcbcrConversion *ycbcrConversion)
    : device(device)
	, state(std::make_shared<SamplerState>(pCreateInfo, ycbcrConversion))
{
	//static std::vector<Param> cache;

	//Param p;
	//memset(&p, 0, sizeof(Param));
//	p.magFilter = magFilter;
//	p.minFilter = minFilter;
//	p.mipmapMode = mipmapMode;
//	p.addressModeU = addressModeU;
//	p.addressModeV = addressModeV;
//	p.addressModeW = addressModeW;
//
//	for(size_t i = 0; i < cache.size(); i++)
//	{
//		if(memcmp(&cache[i], &p, sizeof(Param)) == 0)
//		{
//			id = i + 1;
//			goto done;
//		}
//	}
//
//	cache.push_back(p);
//	id = cache.size();
//
//done:;

    

    {
        auto *samplerIndexer = device->getSamplerIndexer();
	 //   std::lock_guard<std::mutex> lock(samplerIndexer->getMutex());

        id = samplerIndexer->index(state.get());
    }
}

Sampler::~Sampler()
{
	//auto *samplerIndexer = device->getSamplerIndexer();
	//std::lock_guard<std::mutex> lock(samplerIndexer->getMutex());

	auto *samplerIndexer = device->getSamplerIndexer();
	samplerIndexer->remove(state.get());
}

}  // namespace vk
