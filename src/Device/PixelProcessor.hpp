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

#ifndef sw_PixelProcessor_hpp
#define sw_PixelProcessor_hpp

#include "Color.hpp"
#include "Context.hpp"
#include "RoutineCache.hpp"

#include <System/Hash.hpp>

namespace sw
{
	class PixelShader;
	class Rasterizer;
	struct Texture;
	struct DrawData;
	struct Primitive;

	struct ComparableVkStencilOpState : public VkStencilOpState
	{
		inline ComparableVkStencilOpState& operator = (const VkStencilOpState& rhs)
		{
			*static_cast<VkStencilOpState*>(this) = rhs;
			return *this;
		}

		SW_DECLARE_COMPARABLE(ComparableVkStencilOpState,
			failOp, passOp, depthFailOp, compareOp, compareMask, writeMask, reference);
	};

	class PixelProcessor
	{
	public:
		struct States
		{
			uint64_t shaderID = 0;

			VkCompareOp depthCompareMode = VK_COMPARE_OP_NEVER;
			bool depthWriteEnable = false;
			bool quadLayoutDepthBuffer = false;

			bool stencilActive = 0;
			ComparableVkStencilOpState frontStencil = {};
			ComparableVkStencilOpState backStencil = {};

			bool depthTestActive = false;
			bool occlusionEnabled = false;
			bool perspective = false;
			bool depthClamp = false;

			std::array<BlendState, RENDERTARGETS> blendState = {};

			unsigned int colorWriteMask = 0;
			std::array<VkFormat, RENDERTARGETS> targetFormat = {};
			unsigned int multiSample = 0;
			unsigned int multiSampleMask = 0;
			bool alphaToCoverage = false;
			bool centroid = false;
			VkFrontFace frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
			VkFormat depthFormat = VK_FORMAT_UNDEFINED;

			SW_DECLARE_COMPARABLE(States,
				shaderID, depthCompareMode, depthWriteEnable,
				quadLayoutDepthBuffer, stencilActive, frontStencil, backStencil,
				depthTestActive, occlusionEnabled, perspective, depthClamp,
				blendState, colorWriteMask, targetFormat, multiSample,
				multiSampleMask, alphaToCoverage, centroid, frontFace,
				depthFormat);
		};

		struct State : States
		{
			bool operator==(const State &state) const;

			int colorWriteActive(int index) const
			{
				return (colorWriteMask >> (index * 4)) & 0xF;
			}

			uint32_t hash;
		};

		struct Stencil
		{
			int64_t testMaskQ;
			int64_t referenceMaskedQ;
			int64_t referenceMaskedSignedQ;
			int64_t writeMaskQ;
			int64_t invWriteMaskQ;
			int64_t referenceQ;

			void set(int reference, int testMask, int writeMask)
			{
				referenceQ = replicate(reference);
				testMaskQ = replicate(testMask);
				writeMaskQ = replicate(writeMask);
				invWriteMaskQ = ~writeMaskQ;
				referenceMaskedQ = referenceQ & testMaskQ;
				referenceMaskedSignedQ = replicate(((reference & testMask) + 0x80) & 0xFF);
			}

			static int64_t replicate(int b)
			{
				int64_t w = b & 0xFF;

				return (w << 0) | (w << 8) | (w << 16) | (w << 24) | (w << 32) | (w << 40) | (w << 48) | (w << 56);
			}
		};

		struct Factor
		{
			word4 alphaReference4;

			word4 blendConstant4W[4];
			float4 blendConstant4F[4];
			word4 invBlendConstant4W[4];
			float4 invBlendConstant4F[4];
		};

	public:
		typedef void (*RoutinePointer)(const Primitive *primitive, int count, int cluster, int clusterCount, DrawData *draw);

		PixelProcessor();

		virtual ~PixelProcessor();

		void setBlendConstant(const Color<float> &blendConstant);

	protected:
		const State update(const Context* context) const;
		std::shared_ptr<Routine> routine(const State &state, vk::PipelineLayout const *pipelineLayout,
		                                 SpirvShader const *pixelShader, const vk::DescriptorSet::Bindings &descriptorSets);
		void setRoutineCacheSize(int routineCacheSize);

		// Other semi-constants
		Factor factor;

	private:
		RoutineCache<State> *routineCache;
	};
}

#endif   // sw_PixelProcessor_hpp
