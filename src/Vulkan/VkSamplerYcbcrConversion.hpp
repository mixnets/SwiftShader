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

#ifndef VK_SAMPLER_YCBCR_CONVERSION_HPP_
#define VK_SAMPLER_YCBCR_CONVERSION_HPP_

#include "VkObject.hpp"

namespace vk
{
	class SamplerYcbcrConversion : public Object<SamplerYcbcrConversion, VkSamplerYcbcrConversion>
	{
	public:
		SamplerYcbcrConversion(const VkSamplerYcbcrConversionCreateInfo* pCreateInfo, void* mem) :
			format(pCreateInfo->format),
			ycbcrModel(pCreateInfo->ycbcrModel),
			ycbcrRange(pCreateInfo->ycbcrRange),
			components(pCreateInfo->components),
			xChromaOffset(pCreateInfo->xChromaOffset),
			yChromaOffset(pCreateInfo->yChromaOffset),
			chromaFilter(pCreateInfo->chromaFilter),
			forceExplicitReconstruction(pCreateInfo->forceExplicitReconstruction)
		{
		}

		~SamplerYcbcrConversion() = delete;

		static size_t ComputeRequiredAllocationSize(const VkSamplerYcbcrConversionCreateInfo* pCreateInfo)
		{
			return 0;
		}

	private:
		VkFormat                         format = VK_FORMAT_UNDEFINED;
		VkSamplerYcbcrModelConversion    ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY;
		VkSamplerYcbcrRange              ycbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_FULL;
		VkComponentMapping               components =
		{
			VK_COMPONENT_SWIZZLE_R,
			VK_COMPONENT_SWIZZLE_G,
			VK_COMPONENT_SWIZZLE_B,
			VK_COMPONENT_SWIZZLE_A,
		};
		VkChromaLocation                 xChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
		VkChromaLocation                 yChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
		VkFilter                         chromaFilter = VK_FILTER_NEAREST;
		VkBool32                         forceExplicitReconstruction = VK_FALSE;
	};

	static inline SamplerYcbcrConversion* Cast(VkSamplerYcbcrConversion object)
	{
		return reinterpret_cast<SamplerYcbcrConversion*>(object);
	}

} // namespace vk

#endif // VK_SAMPLER_YCBCR_CONVERSION_HPP_