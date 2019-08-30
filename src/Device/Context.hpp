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

#ifndef sw_Context_hpp
#define sw_Context_hpp

#include "Vulkan/VkConfig.h"
#include "Vulkan/VkDescriptorSet.hpp"
#include "Config.hpp"
#include "Stream.hpp"

#include "System/Hash.hpp"
#include "System/Types.hpp"

namespace vk
{
	class ImageView;
	class PipelineLayout;
}

namespace sw
{
	class SpirvShader;

	struct PushConstantStorage
	{
		unsigned char data[vk::MAX_PUSH_CONSTANT_SIZE];
	};

	struct BlendState
	{
		bool alphaBlendEnable = false;
		VkBlendFactor sourceBlendFactor = VK_BLEND_FACTOR_ONE;
		VkBlendFactor destBlendFactor = VK_BLEND_FACTOR_ZERO;
		VkBlendOp blendOperation = VK_BLEND_OP_ADD;
		VkBlendFactor sourceBlendFactorAlpha = VK_BLEND_FACTOR_ONE;
		VkBlendFactor destBlendFactorAlpha = VK_BLEND_FACTOR_ZERO;
		VkBlendOp blendOperationAlpha = VK_BLEND_OP_ADD;

		SW_DECLARE_COMPARABLE(BlendState,
				alphaBlendEnable, sourceBlendFactor, destBlendFactor,
				blendOperation, sourceBlendFactorAlpha, destBlendFactorAlpha,
				blendOperationAlpha);
	};

	class Context
	{
	public:
		Context();

		void init();

		bool isDrawPoint() const;
		bool isDrawLine() const;
		bool isDrawTriangle() const;

		bool depthWriteActive() const;
		bool depthBufferActive() const;
		bool stencilActive() const;

		bool allTargetsColorClamp() const;

		void setBlendState(int index, BlendState state);
		BlendState getBlendState(int index) const;

		VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;

		bool stencilEnable = false;
		VkStencilOpState frontStencil = {};
		VkStencilOpState backStencil = {};

		// Pixel processor states
		VkCullModeFlags cullMode = VK_CULL_MODE_FRONT_BIT;
		VkFrontFace frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

		float depthBias = 0.0f;
		float slopeDepthBias = 0.0f;

		VkFormat renderTargetInternalFormat(int index) const;
		int colorWriteActive(int index) const;

		vk::DescriptorSet::Bindings descriptorSets = {};
		vk::DescriptorSet::DynamicOffsets descriptorDynamicOffsets = {};
		Stream input[MAX_INTERFACE_COMPONENTS / 4];

		std::array<vk::ImageView*, RENDERTARGETS> renderTarget = {};
		vk::ImageView *depthBuffer = nullptr;
		vk::ImageView *stencilBuffer = nullptr;

		vk::PipelineLayout const *pipelineLayout = nullptr;

		// Shaders
		const SpirvShader *pixelShader = nullptr;
		const SpirvShader *vertexShader = nullptr;

		bool occlusionEnabled = false;

		// Pixel processor states
		bool rasterizerDiscard = false;
		bool depthBoundsTestEnable = false;
		bool depthBufferEnable = false;
		VkCompareOp depthCompareMode = VK_COMPARE_OP_LESS;
		bool depthWriteEnable = false;

		float lineWidth = 1.0f;

		std::array<int, RENDERTARGETS> colorWriteMask;   // RGBA
		unsigned int sampleMask = 0xFFFFFFFF;
		unsigned int multiSampleMask = 0;
		int sampleCount = 0;
		bool alphaToCoverage = false;

	private:
		bool colorWriteActive() const;
		bool colorUsed() const;

		bool alphaBlendActive(int index) const;
		VkBlendFactor sourceBlendFactor(int index) const;
		VkBlendFactor destBlendFactor(int index) const;
		VkBlendOp blendOperation(int index) const;

		VkBlendFactor sourceBlendFactorAlpha(int index) const;
		VkBlendFactor destBlendFactorAlpha(int index) const;
		VkBlendOp blendOperationAlpha(int index) const;

		BlendState blendState[RENDERTARGETS];
	};
}

#endif   // sw_Context_hpp
