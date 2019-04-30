// Copyright 2016 The SwiftShader Authors. All Rights Reserved.
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

#include "Sampler.hpp"

namespace sw
{
	FilterType Sampler::maximumTextureFilterQuality = FILTER_LINEAR;
	MipmapType Sampler::maximumMipmapFilterQuality = MIPMAP_POINT;

	Sampler::State::State()
	{
		memset(this, 0, sizeof(State));
	}

	Sampler::Sampler()
	{
		// FIXME: Mipmap::init
		static const unsigned int zero = 0x00FF00FF;

		for(int level = 0; level < MIPMAP_LEVELS; level++)
		{
			Mipmap &mipmap = texture.mipmap[level];

			memset(&mipmap, 0, sizeof(Mipmap));

			for(int face = 0; face < 6; face++)
			{
				mipmap.buffer[face] = &zero;
			}
		}

		textureFormat = VK_FORMAT_UNDEFINED;
		textureType = TEXTURE_NULL;

		textureFilter = FILTER_LINEAR;
		addressingModeU = ADDRESSING_WRAP;
		addressingModeV = ADDRESSING_WRAP;
		addressingModeW = ADDRESSING_WRAP;
		mipmapFilterState = MIPMAP_NONE;
		sRGB = false;
		gather = false;
		highPrecisionFiltering = false;
		border = 0;

		swizzle = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };

		compare = COMPARE_BYPASS;

		exp2LOD = 1.0f;
	}

	Sampler::~Sampler()
	{
	}

	Sampler::State Sampler::samplerState() const
	{
		State state;

		if(textureType != TEXTURE_NULL)
		{
			//state.textureType = textureType;
			//state.textureFormat = textureFormat;
			//state.textureFilter = getTextureFilter();
			//state.addressingModeU = getAddressingModeU();
			//state.addressingModeV = getAddressingModeV();
			//state.addressingModeW = getAddressingModeW();
			//state.mipmapFilter = mipmapFilter();
			//state.sRGB = (sRGB && textureFormat.isSRGBreadable()) || textureFormat.isSRGBformat();
			//state.swizzle = swizzle;
			//state.highPrecisionFiltering = highPrecisionFiltering;
			//state.compare = getCompareFunc();

			#if PERF_PROFILE
				state.compressedFormat = Surface::isCompressed(externalTextureFormat);
			#endif
		}

		return state;
	}
}
