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

VkClass(Sampler) {
public:
	Sampler(VkDevice pDevice,
	        VkFilter pMagFilter,
	        VkFilter pMinFilter,
	        VkSamplerMipmapMode pMipmapMode,
	        VkSamplerAddressMode pAddressModeU,
	        VkSamplerAddressMode pAddressModeV,
	        VkSamplerAddressMode pAddressModeW,
	        float pMipLodBias,
	        VkBool32 pAnisotropyEnable,
	        float pMaxAnisotropy,
	        VkBool32 pCompareEnable,
	        VkCompareOp pCompareOp,
	        float pMinLod,
	        float pMaxLod,
	        VkBorderColor pBorderColor,
	        VkBool32 pUnnormalizedCoordinates) :
		device(Cast(pDevice)),
		magFilter(pMagFilter),
		minFilter(pMinFilter),
		mipmapMode(pMipmapMode),
		addressModeU(pAddressModeU),
		addressModeV(pAddressModeV),
		addressModeW(pAddressModeW),
		mipLodBias(pMipLodBias),
		anisotropyEnable(pAnisotropyEnable),
		maxAnisotropy(pMaxAnisotropy),
		compareEnable(pCompareEnable),
		compareOp(pCompareOp),
		minLod(pMinLod),
		maxLod(pMaxLod),
		borderColor(pBorderColor),
		unnormalizedCoordinates(pUnnormalizedCoordinates) {}

	static VkSystemAllocationScope getSystemAllocationScope() { return VK_SYSTEM_ALLOCATION_SCOPE_OBJECT; }

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

#endif // VK_SAMPLER_HPP_