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

#include <vector>

namespace vk {

struct Param
{
	VkFilter magFilter = VK_FILTER_NEAREST;
	VkFilter minFilter = VK_FILTER_NEAREST;
	VkSamplerMipmapMode mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	VkSamplerAddressMode addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	VkSamplerAddressMode addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	VkSamplerAddressMode addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	float mipLodBias = 0.0f;
	VkBool32 anisotropyEnable = VK_FALSE;
	float maxAnisotropy = 0.0f;
	VkBool32 compareEnable = VK_FALSE;
	VkCompareOp compareOp = VK_COMPARE_OP_NEVER;
	float minLod = 0.0f;
	float maxLod = 0.0f;
	VkBorderColor borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
	VkBool32 unnormalizedCoordinates = VK_FALSE;
};

Sampler::Sampler(const VkSamplerCreateInfo *pCreateInfo, void *mem, const vk::SamplerYcbcrConversion *ycbcrConversion)
    : magFilter(pCreateInfo->magFilter)
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
    , ycbcrConversion(ycbcrConversion)
{
	static std::vector<Param> cache;

	Param p;
	memset(&p, 0, sizeof(Param));
	p.magFilter = magFilter;
	p.minFilter = minFilter;
	p.mipmapMode = mipmapMode;
	p.addressModeU = addressModeU;
	p.addressModeV = addressModeV;
	p.addressModeW = addressModeW;

	for(size_t i = 0; i < cache.size(); i++)
	{
		if(memcmp(&cache[i], &p, sizeof(Param)) == 0)
		{
			id = i + 1;
			goto done;
		}
	}

	cache.push_back(p);
	id = cache.size();

done:;
}

}  // namespace vk
