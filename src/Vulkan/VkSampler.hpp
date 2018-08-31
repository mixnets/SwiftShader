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

#ifndef VK_SAMPLER_HPP_
#define VK_SAMPLER_HPP_

#include "VkDevice.hpp"

namespace vk {

VkClass(Sampler) {
public:
	constexpr static VkSystemAllocationScope kAllocationScope = VK_SYSTEM_ALLOCATION_SCOPE_OBJECT;

	Sampler(VkDevice pDevice, const VkSamplerCreateInfo* pCreateInfo) :
		device(Cast(pDevice)),
		magFilter(pCreateInfo->magFilter),
		minFilter(pCreateInfo->minFilter),
		mipmapMode(pCreateInfo->mipmapMode),
		addressModeU(pCreateInfo->addressModeU),
		addressModeV(pCreateInfo->addressModeV),
		addressModeW(pCreateInfo->addressModeW),
		mipLodBias(pCreateInfo->mipLodBias),
		anisotropyEnable(pCreateInfo->anisotropyEnable),
		maxAnisotropy(pCreateInfo->maxAnisotropy),
		compareEnable(pCreateInfo->compareEnable),
		compareOp(pCreateInfo->compareOp),
		minLod(pCreateInfo->minLod),
		maxLod(pCreateInfo->maxLod),
		borderColor(pCreateInfo->borderColor),
		unnormalizedCoordinates(pCreateInfo->unnormalizedCoordinates) {}

	~Sampler() = delete;

private:
	Device*                 device;
	VkFilter                magFilter;
	VkFilter                minFilter;
	VkSamplerMipmapMode     mipmapMode;
	VkSamplerAddressMode    addressModeU;
	VkSamplerAddressMode    addressModeV;
	VkSamplerAddressMode    addressModeW;
	float                   mipLodBias;
	VkBool32                anisotropyEnable;
	float                   maxAnisotropy;
	VkBool32                compareEnable;
	VkCompareOp             compareOp;
	float                   minLod;
	float                   maxLod;
	VkBorderColor           borderColor;
	VkBool32                unnormalizedCoordinates;
};

} // namespace vk

#endif // VK_SAMPLER_HPP_