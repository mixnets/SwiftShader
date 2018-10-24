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

#include "VkPipeline.hpp"
#include "VkPipelineLayout.hpp"
#include "VkRenderPass.hpp"
#include "VkShaderModule.hpp"

namespace
{

sw::DrawType Convert(VkPrimitiveTopology topology)
{
	switch(topology)
	{
	case VK_PRIMITIVE_TOPOLOGY_POINT_LIST:
		return sw::DRAW_POINTLIST;
	case VK_PRIMITIVE_TOPOLOGY_LINE_LIST:
		return sw::DRAW_LINELIST;
	case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP:
		return sw::DRAW_LINESTRIP;
	case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST:
		return sw::DRAW_TRIANGLELIST;
	case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP:
		return sw::DRAW_TRIANGLESTRIP;
	case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN:
		return sw::DRAW_TRIANGLEFAN;
	case VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY:
	case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY:
	case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY:
	case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY:
		// geometry shader specific
		ASSERT(false);
		break;
	case VK_PRIMITIVE_TOPOLOGY_PATCH_LIST:
		// tesselation shader specific
		ASSERT(false);
		break;
	default:
		UNIMPLEMENTED();
	}

	return sw::DRAW_TRIANGLELIST;
}

sw::Rect Convert(const VkRect2D& rect)
{
	return sw::Rect(rect.offset.x, rect.offset.y, rect.offset.x + rect.extent.width, rect.offset.y + rect.extent.height);
}

sw::Viewport Convert(const VkViewport& viewport)
{
	return { viewport.x, viewport.y, viewport.width, viewport.height, viewport.minDepth, viewport.maxDepth };
}

sw::StencilOperation Convert(VkStencilOp stencilOp)
{
	switch(stencilOp)
	{
	case VK_STENCIL_OP_KEEP:
		return sw::OPERATION_KEEP;
	case VK_STENCIL_OP_ZERO:
		return sw::OPERATION_ZERO;
	case VK_STENCIL_OP_REPLACE:
		return sw::OPERATION_REPLACE;
	case VK_STENCIL_OP_INCREMENT_AND_CLAMP:
		return sw::OPERATION_INCRSAT;
	case VK_STENCIL_OP_DECREMENT_AND_CLAMP:
		return sw::OPERATION_DECRSAT;
	case VK_STENCIL_OP_INVERT:
		return sw::OPERATION_INVERT;
	case VK_STENCIL_OP_INCREMENT_AND_WRAP:
		return sw::OPERATION_INCR;
	case VK_STENCIL_OP_DECREMENT_AND_WRAP:
		return sw::OPERATION_DECR;
	default:
		UNIMPLEMENTED();
	}

	return sw::OPERATION_KEEP;
}

sw::StencilCompareMode ConvertStencil(VkCompareOp compareOp)
{
	switch(compareOp)
	{
	case VK_COMPARE_OP_NEVER:
		return sw::STENCIL_NEVER;
	case VK_COMPARE_OP_LESS:
		return sw::STENCIL_LESS;
	case VK_COMPARE_OP_EQUAL:
		return sw::STENCIL_EQUAL;
	case VK_COMPARE_OP_LESS_OR_EQUAL:
		return sw::STENCIL_LESSEQUAL;
	case VK_COMPARE_OP_GREATER:
		return sw::STENCIL_GREATER;
	case VK_COMPARE_OP_NOT_EQUAL:
		return sw::STENCIL_NOTEQUAL;
	case VK_COMPARE_OP_GREATER_OR_EQUAL:
		return sw::STENCIL_GREATEREQUAL;
	case VK_COMPARE_OP_ALWAYS:
		return sw::STENCIL_ALWAYS;
	default:
		UNIMPLEMENTED();
	}

	return sw::STENCIL_NEVER;
}

sw::DepthCompareMode ConvertDepth(VkCompareOp compareOp)
{
	switch(compareOp)
	{
	case VK_COMPARE_OP_NEVER:
		return sw::DEPTH_NEVER;
	case VK_COMPARE_OP_LESS:
		return sw::DEPTH_LESS;
	case VK_COMPARE_OP_EQUAL:
		return sw::DEPTH_EQUAL;
	case VK_COMPARE_OP_LESS_OR_EQUAL:
		return sw::DEPTH_LESSEQUAL;
	case VK_COMPARE_OP_GREATER:
		return sw::DEPTH_GREATER;
	case VK_COMPARE_OP_NOT_EQUAL:
		return sw::DEPTH_NOTEQUAL;
	case VK_COMPARE_OP_GREATER_OR_EQUAL:
		return sw::DEPTH_GREATEREQUAL;
	case VK_COMPARE_OP_ALWAYS:
		return sw::DEPTH_ALWAYS;
	default:
		UNIMPLEMENTED();
	}

	return sw::DEPTH_NEVER;
}

sw::LogicalOperation Convert(VkLogicOp logicOp)
{
	switch(logicOp)
	{
	case VK_LOGIC_OP_CLEAR:
		return sw::LOGICALOP_CLEAR;
	case VK_LOGIC_OP_AND:
		return sw::LOGICALOP_AND;
	case VK_LOGIC_OP_AND_REVERSE:
		return sw::LOGICALOP_AND_REVERSE;
	case VK_LOGIC_OP_COPY:
		return sw::LOGICALOP_COPY;
	case VK_LOGIC_OP_AND_INVERTED:
		return sw::LOGICALOP_AND_INVERTED;
	case VK_LOGIC_OP_NO_OP:
		return sw::LOGICALOP_NOOP;
	case VK_LOGIC_OP_XOR:
		return sw::LOGICALOP_XOR;
	case VK_LOGIC_OP_OR:
		return sw::LOGICALOP_OR;
	case VK_LOGIC_OP_NOR:
		return sw::LOGICALOP_NOR;
	case VK_LOGIC_OP_EQUIVALENT:
		return sw::LOGICALOP_EQUIV;
	case VK_LOGIC_OP_INVERT:
		return sw::LOGICALOP_INVERT;
	case VK_LOGIC_OP_OR_REVERSE:
		return sw::LOGICALOP_OR_REVERSE;
	case VK_LOGIC_OP_COPY_INVERTED:
		return sw::LOGICALOP_COPY_INVERTED;
	case VK_LOGIC_OP_OR_INVERTED:
		return sw::LOGICALOP_OR_INVERTED;
	case VK_LOGIC_OP_NAND:
		return sw::LOGICALOP_NAND;
	case VK_LOGIC_OP_SET:
		return sw::LOGICALOP_SET;
	default:
		UNIMPLEMENTED();
	}

	return sw::LOGICALOP_CLEAR;
}

sw::BlendFactor Convert(VkBlendFactor blendFactor)
{
	switch(blendFactor)
	{
	case VK_BLEND_FACTOR_ZERO:
		return sw::BLEND_ZERO;
	case VK_BLEND_FACTOR_ONE:
		return sw::BLEND_ONE;
	case VK_BLEND_FACTOR_SRC_COLOR:
		return sw::BLEND_SOURCE;
	case VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR:
		return sw::BLEND_INVSOURCE;
	case VK_BLEND_FACTOR_DST_COLOR:
		return sw::BLEND_DEST;
	case VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR:
		return sw::BLEND_INVDEST;
	case VK_BLEND_FACTOR_SRC_ALPHA:
		return sw::BLEND_SOURCEALPHA;
	case VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA:
		return sw::BLEND_INVSOURCEALPHA;
	case VK_BLEND_FACTOR_DST_ALPHA:
		return sw::BLEND_DESTALPHA;
	case VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA:
		return sw::BLEND_INVDESTALPHA;
	case VK_BLEND_FACTOR_CONSTANT_COLOR:
		return sw::BLEND_CONSTANT;
	case VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR:
		return sw::BLEND_INVCONSTANT;
	case VK_BLEND_FACTOR_CONSTANT_ALPHA:
		return sw::BLEND_CONSTANTALPHA;
	case VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA:
		return sw::BLEND_INVCONSTANTALPHA;
	case VK_BLEND_FACTOR_SRC_ALPHA_SATURATE:
		return sw::BLEND_SRCALPHASAT;
	case VK_BLEND_FACTOR_SRC1_COLOR:
	case VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR:
	case VK_BLEND_FACTOR_SRC1_ALPHA:
	case VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA:
	default:
		UNIMPLEMENTED();
	}

	return sw::BLEND_ZERO;
}

sw::BlendOperation Convert(VkBlendOp blendOp)
{
	switch(blendOp)
	{
	case VK_BLEND_OP_ADD:
		return sw::BLENDOP_ADD;
	case VK_BLEND_OP_SUBTRACT:
		return sw::BLENDOP_SUB;
	case VK_BLEND_OP_REVERSE_SUBTRACT:
		return sw::BLENDOP_INVSUB;
	case VK_BLEND_OP_MIN:
		return sw::BLENDOP_MIN;
	case VK_BLEND_OP_MAX:
		return sw::BLENDOP_MAX;
	case VK_BLEND_OP_ZERO_EXT:
		return sw::BLENDOP_NULL;
	case VK_BLEND_OP_SRC_EXT:
		return sw::BLENDOP_SOURCE;
	case VK_BLEND_OP_DST_EXT:
		return sw::BLENDOP_DEST;
	case VK_BLEND_OP_SRC_OVER_EXT:
	case VK_BLEND_OP_DST_OVER_EXT:
	case VK_BLEND_OP_SRC_IN_EXT:
	case VK_BLEND_OP_DST_IN_EXT:
	case VK_BLEND_OP_SRC_OUT_EXT:
	case VK_BLEND_OP_DST_OUT_EXT:
	case VK_BLEND_OP_SRC_ATOP_EXT:
	case VK_BLEND_OP_DST_ATOP_EXT:
	case VK_BLEND_OP_XOR_EXT:
	case VK_BLEND_OP_MULTIPLY_EXT:
	case VK_BLEND_OP_SCREEN_EXT:
	case VK_BLEND_OP_OVERLAY_EXT:
	case VK_BLEND_OP_DARKEN_EXT:
	case VK_BLEND_OP_LIGHTEN_EXT:
	case VK_BLEND_OP_COLORDODGE_EXT:
	case VK_BLEND_OP_COLORBURN_EXT:
	case VK_BLEND_OP_HARDLIGHT_EXT:
	case VK_BLEND_OP_SOFTLIGHT_EXT:
	case VK_BLEND_OP_DIFFERENCE_EXT:
	case VK_BLEND_OP_EXCLUSION_EXT:
	case VK_BLEND_OP_INVERT_EXT:
	case VK_BLEND_OP_INVERT_RGB_EXT:
	case VK_BLEND_OP_LINEARDODGE_EXT:
	case VK_BLEND_OP_LINEARBURN_EXT:
	case VK_BLEND_OP_VIVIDLIGHT_EXT:
	case VK_BLEND_OP_LINEARLIGHT_EXT:
	case VK_BLEND_OP_PINLIGHT_EXT:
	case VK_BLEND_OP_HARDMIX_EXT:
	case VK_BLEND_OP_HSL_HUE_EXT:
	case VK_BLEND_OP_HSL_SATURATION_EXT:
	case VK_BLEND_OP_HSL_COLOR_EXT:
	case VK_BLEND_OP_HSL_LUMINOSITY_EXT:
	case VK_BLEND_OP_PLUS_EXT:
	case VK_BLEND_OP_PLUS_CLAMPED_EXT:
	case VK_BLEND_OP_PLUS_CLAMPED_ALPHA_EXT:
	case VK_BLEND_OP_PLUS_DARKER_EXT:
	case VK_BLEND_OP_MINUS_EXT:
	case VK_BLEND_OP_MINUS_CLAMPED_EXT:
	case VK_BLEND_OP_CONTRAST_EXT:
	case VK_BLEND_OP_INVERT_OVG_EXT:
	case VK_BLEND_OP_RED_EXT:
	case VK_BLEND_OP_GREEN_EXT:
	case VK_BLEND_OP_BLUE_EXT:
	default:
		UNIMPLEMENTED();
	}

	return sw::BLENDOP_NULL;
}

sw::StreamType getStreamType(VkFormat format)
{
	switch(format)
	{
	case VK_FORMAT_R8_UNORM:
	case VK_FORMAT_R8G8_UNORM:
	case VK_FORMAT_R8G8B8A8_UNORM:
	case VK_FORMAT_R8_UINT:
	case VK_FORMAT_R8G8_UINT:
	case VK_FORMAT_R8G8B8A8_UINT:
	case VK_FORMAT_B8G8R8A8_UNORM:
	case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
	case VK_FORMAT_A8B8G8R8_UINT_PACK32:
		return sw::STREAMTYPE_BYTE;
	case VK_FORMAT_R8_SNORM:
	case VK_FORMAT_R8_SINT:
	case VK_FORMAT_R8G8_SNORM:
	case VK_FORMAT_R8G8_SINT:
	case VK_FORMAT_R8G8B8A8_SNORM:
	case VK_FORMAT_R8G8B8A8_SINT:
	case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
	case VK_FORMAT_A8B8G8R8_SINT_PACK32:
		return sw::STREAMTYPE_SBYTE;
	case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
		return sw::STREAMTYPE_2_10_10_10_UINT;
	case VK_FORMAT_R16_UNORM:
	case VK_FORMAT_R16_UINT:
	case VK_FORMAT_R16G16_UNORM:
	case VK_FORMAT_R16G16_UINT:
	case VK_FORMAT_R16G16B16A16_UNORM:
	case VK_FORMAT_R16G16B16A16_UINT:
		return sw::STREAMTYPE_USHORT;
	case VK_FORMAT_R16_SNORM:
	case VK_FORMAT_R16_SINT:
	case VK_FORMAT_R16G16_SNORM:
	case VK_FORMAT_R16G16_SINT:
	case VK_FORMAT_R16G16B16A16_SNORM:
	case VK_FORMAT_R16G16B16A16_SINT:
		return sw::STREAMTYPE_SHORT;
	case VK_FORMAT_R16_SFLOAT:
	case VK_FORMAT_R16G16_SFLOAT:
	case VK_FORMAT_R16G16B16A16_SFLOAT:
		return sw::STREAMTYPE_HALF;
	case VK_FORMAT_R32_UINT:
	case VK_FORMAT_R32G32_UINT:
	case VK_FORMAT_R32G32B32_UINT:
	case VK_FORMAT_R32G32B32A32_UINT:
		return sw::STREAMTYPE_UINT;
	case VK_FORMAT_R32_SINT:
	case VK_FORMAT_R32G32_SINT:
	case VK_FORMAT_R32G32B32_SINT:
	case VK_FORMAT_R32G32B32A32_SINT:
		return sw::STREAMTYPE_INT;
	case VK_FORMAT_R32_SFLOAT:
	case VK_FORMAT_R32G32_SFLOAT:
	case VK_FORMAT_R32G32B32_SFLOAT:
	case VK_FORMAT_R32G32B32A32_SFLOAT:
		return sw::STREAMTYPE_FLOAT;
	default:
		UNIMPLEMENTED();
	}

	return sw::STREAMTYPE_BYTE;
}

uint32_t getNumberOfChannels(VkFormat format)
{
	switch(format)
	{
	case VK_FORMAT_R8_UNORM:
	case VK_FORMAT_R8_SNORM:
	case VK_FORMAT_R8_UINT:
	case VK_FORMAT_R8_SINT:
	case VK_FORMAT_R16_UNORM:
	case VK_FORMAT_R16_SNORM:
	case VK_FORMAT_R16_UINT:
	case VK_FORMAT_R16_SINT:
	case VK_FORMAT_R16_SFLOAT:
	case VK_FORMAT_R32_UINT:
	case VK_FORMAT_R32_SINT:
	case VK_FORMAT_R32_SFLOAT:
		return 1;
	case VK_FORMAT_R8G8_UNORM:
	case VK_FORMAT_R8G8_SNORM:
	case VK_FORMAT_R8G8_UINT:
	case VK_FORMAT_R8G8_SINT:
	case VK_FORMAT_R16G16_UNORM:
	case VK_FORMAT_R16G16_SNORM:
	case VK_FORMAT_R16G16_UINT:
	case VK_FORMAT_R16G16_SINT:
	case VK_FORMAT_R16G16_SFLOAT:
	case VK_FORMAT_R32G32_UINT:
	case VK_FORMAT_R32G32_SINT:
	case VK_FORMAT_R32G32_SFLOAT:
		return 2;
	case VK_FORMAT_R32G32B32_UINT:
	case VK_FORMAT_R32G32B32_SINT:
	case VK_FORMAT_R32G32B32_SFLOAT:
		return 3;
	case VK_FORMAT_R8G8B8A8_UNORM:
	case VK_FORMAT_R8G8B8A8_SNORM:
	case VK_FORMAT_R8G8B8A8_UINT:
	case VK_FORMAT_R8G8B8A8_SINT:
	case VK_FORMAT_B8G8R8A8_UNORM:
	case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
	case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
	case VK_FORMAT_A8B8G8R8_UINT_PACK32:
	case VK_FORMAT_A8B8G8R8_SINT_PACK32:
	case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
	case VK_FORMAT_R16G16B16A16_UNORM:
	case VK_FORMAT_R16G16B16A16_SNORM:
	case VK_FORMAT_R16G16B16A16_UINT:
	case VK_FORMAT_R16G16B16A16_SINT:
	case VK_FORMAT_R16G16B16A16_SFLOAT:
	case VK_FORMAT_R32G32B32A32_UINT:
	case VK_FORMAT_R32G32B32A32_SINT:
	case VK_FORMAT_R32G32B32A32_SFLOAT:
		return 4;
	default:
		UNIMPLEMENTED();
	}

	return 0;
}

bool isNormalized(VkFormat format)
{
	switch(format)
	{
	case VK_FORMAT_R8_UNORM:
	case VK_FORMAT_R8_SNORM:
	case VK_FORMAT_R8G8_UNORM:
	case VK_FORMAT_R8G8_SNORM:
	case VK_FORMAT_R8G8B8A8_UNORM:
	case VK_FORMAT_R8G8B8A8_SNORM:
	case VK_FORMAT_B8G8R8A8_UNORM:
	case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
	case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
	case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
	case VK_FORMAT_R16_UNORM:
	case VK_FORMAT_R16_SNORM:
	case VK_FORMAT_R16G16_UNORM:
	case VK_FORMAT_R16G16_SNORM:
	case VK_FORMAT_R16G16B16A16_UNORM:
	case VK_FORMAT_R16G16B16A16_SNORM:
		return true;
	case VK_FORMAT_R8_UINT:
	case VK_FORMAT_R8_SINT:
	case VK_FORMAT_R8G8_UINT:
	case VK_FORMAT_R8G8_SINT:
	case VK_FORMAT_R8G8B8A8_UINT:
	case VK_FORMAT_R8G8B8A8_SINT:
	case VK_FORMAT_A8B8G8R8_UINT_PACK32:
	case VK_FORMAT_A8B8G8R8_SINT_PACK32:
	case VK_FORMAT_R16_UINT:
	case VK_FORMAT_R16_SINT:
	case VK_FORMAT_R16_SFLOAT:
	case VK_FORMAT_R16G16_UINT:
	case VK_FORMAT_R16G16_SINT:
	case VK_FORMAT_R16G16_SFLOAT:
	case VK_FORMAT_R16G16B16A16_UINT:
	case VK_FORMAT_R16G16B16A16_SINT:
	case VK_FORMAT_R16G16B16A16_SFLOAT:
	case VK_FORMAT_R32_UINT:
	case VK_FORMAT_R32_SINT:
	case VK_FORMAT_R32_SFLOAT:
	case VK_FORMAT_R32G32_UINT:
	case VK_FORMAT_R32G32_SINT:
	case VK_FORMAT_R32G32_SFLOAT:
	case VK_FORMAT_R32G32B32_UINT:
	case VK_FORMAT_R32G32B32_SINT:
	case VK_FORMAT_R32G32B32_SFLOAT:
	case VK_FORMAT_R32G32B32A32_UINT:
	case VK_FORMAT_R32G32B32A32_SINT:
	case VK_FORMAT_R32G32B32A32_SFLOAT:
		return false;
	default:
		UNIMPLEMENTED();
	}

	return true;
}

}

namespace vk
{

Pipeline::Pipeline(const VkGraphicsPipelineCreateInfo* pCreateInfo, void* mem)
{
	if((pCreateInfo->flags != 0) ||
	   (pCreateInfo->stageCount != 2) ||
	   (pCreateInfo->pTessellationState != nullptr) ||
	   (pCreateInfo->pDynamicState != nullptr) ||
	   (pCreateInfo->subpass != 0) ||
	   (pCreateInfo->basePipelineHandle != VK_NULL_HANDLE) ||
	   (pCreateInfo->basePipelineIndex != 0))
	{
		UNIMPLEMENTED();
	}

	const auto& vertexStage = pCreateInfo->pStages[0];
	if((vertexStage.stage != VK_SHADER_STAGE_VERTEX_BIT) ||
	   (vertexStage.flags != 0) ||
	   (vertexStage.pSpecializationInfo != nullptr))
	{
		UNIMPLEMENTED();
	}

	vertexModule = vertexStage.module;

	const auto& fragmentStage = pCreateInfo->pStages[1];
	if((fragmentStage.stage != VK_SHADER_STAGE_FRAGMENT_BIT) ||
	   (fragmentStage.flags != 0) ||
	   (fragmentStage.pSpecializationInfo != nullptr))
	{
		UNIMPLEMENTED();
	}

	fragmentModule = fragmentStage.module;

	auto vertexInputState = pCreateInfo->pVertexInputState;
	if(vertexInputState->flags != 0)
	{
		UNIMPLEMENTED();
	}

	for(uint32_t i = 0; i < vertexInputState->vertexBindingDescriptionCount; i++)
	{
		const auto* vertexBindingDescription = vertexInputState->pVertexBindingDescriptions;
		context.input[vertexBindingDescription->binding].stride = vertexBindingDescription->stride;
		if(vertexBindingDescription->inputRate != VK_VERTEX_INPUT_RATE_VERTEX)
		{
			UNIMPLEMENTED();
		}
	}

	for(uint32_t i = 0; i < vertexInputState->vertexAttributeDescriptionCount; i++)
	{
		const auto* vertexAttributeDescriptions = vertexInputState->pVertexAttributeDescriptions;
		auto& input = context.input[vertexAttributeDescriptions->binding];
		input.count = getNumberOfChannels(vertexAttributeDescriptions->format);
		input.type = getStreamType(vertexAttributeDescriptions->format);
		input.normalized = isNormalized(vertexAttributeDescriptions->format);

		if(vertexAttributeDescriptions->location != vertexAttributeDescriptions->binding)
		{
			UNIMPLEMENTED();
		}
		if(vertexAttributeDescriptions->offset != 0)
		{
			UNIMPLEMENTED();
		}
	}

	auto assemblyState = pCreateInfo->pInputAssemblyState;
	if((assemblyState->flags != 0) ||
	   (assemblyState->primitiveRestartEnable != 0))
	{
		UNIMPLEMENTED();
	}

	context.drawType = Convert(assemblyState->topology);

	auto viewportState = pCreateInfo->pViewportState;
	if((viewportState->flags != 0) ||
	   (viewportState->viewportCount != 1) ||
	   (viewportState->scissorCount	!= 1))
	{
		UNIMPLEMENTED();
	}

	scissor = Convert(viewportState->pScissors[0]);
	viewport = Convert(viewportState->pViewports[0]);

	auto rasterizationState = pCreateInfo->pRasterizationState;
	if((rasterizationState->flags != 0) ||
	   (rasterizationState->depthClampEnable != 0) ||
	   (rasterizationState->polygonMode != VK_POLYGON_MODE_FILL))
	{
		UNIMPLEMENTED();
	}

	context.rasterizerDiscard = rasterizationState->rasterizerDiscardEnable;
	context.frontFacingCCW = rasterizationState->frontFace == VK_FRONT_FACE_COUNTER_CLOCKWISE;
	context.depthBias = (rasterizationState->depthBiasEnable ? rasterizationState->depthBiasConstantFactor : 0.0f);
	context.slopeDepthBias = (rasterizationState->depthBiasEnable ? rasterizationState->depthBiasSlopeFactor : 0.0f);

	auto multisampleState = pCreateInfo->pMultisampleState;
	if((multisampleState->flags != 0) ||
	   (multisampleState->rasterizationSamples != VK_SAMPLE_COUNT_1_BIT) ||
	   (multisampleState->sampleShadingEnable != 0) ||
	   (multisampleState->pSampleMask != nullptr) ||
	   (multisampleState->alphaToCoverageEnable != 0) ||
	   (multisampleState->alphaToOneEnable != 0))
	{
		UNIMPLEMENTED();
	}

	auto depthStencilState = pCreateInfo->pDepthStencilState;
	if((depthStencilState->flags != 0) ||
	   (depthStencilState->depthBoundsTestEnable != 0) ||
	   (depthStencilState->minDepthBounds != 0.0f) ||
	   (depthStencilState->maxDepthBounds != 1.0f))
	{
		UNIMPLEMENTED();
	}

	context.depthBufferEnable = depthStencilState->depthTestEnable;
	context.depthWriteEnable = depthStencilState->depthWriteEnable;
	context.depthCompareMode = ConvertDepth(depthStencilState->depthCompareOp);

	context.stencilEnable = context.twoSidedStencil = depthStencilState->stencilTestEnable;
	if(context.stencilEnable)
	{
		context.stencilMask = depthStencilState->front.compareMask;
		context.stencilCompareMode = ConvertStencil(depthStencilState->front.compareOp);
		context.stencilZFailOperation = Convert(depthStencilState->front.depthFailOp);
		context.stencilFailOperation = Convert(depthStencilState->front.failOp);
		context.stencilPassOperation = Convert(depthStencilState->front.passOp);
		context.stencilReference = depthStencilState->front.reference;
		context.stencilWriteMask = depthStencilState->front.writeMask;

		context.stencilMaskCCW = depthStencilState->back.compareMask;
		context.stencilCompareModeCCW = ConvertStencil(depthStencilState->back.compareOp);
		context.stencilZFailOperationCCW = Convert(depthStencilState->back.depthFailOp);
		context.stencilFailOperationCCW = Convert(depthStencilState->back.failOp);
		context.stencilPassOperationCCW = Convert(depthStencilState->back.passOp);
		context.stencilReferenceCCW = depthStencilState->back.reference;
		context.stencilWriteMaskCCW = depthStencilState->back.writeMask;
	}

	auto colorBlendState = pCreateInfo->pColorBlendState;
	if((colorBlendState->flags != 0) ||
	   ((colorBlendState->logicOpEnable != 0) &&
	    (colorBlendState->attachmentCount > 1)))
	{
		UNIMPLEMENTED();
	}

	context.colorLogicOpEnabled = colorBlendState->logicOpEnable;
	context.logicalOperation = Convert(colorBlendState->logicOp);
	blendConstants.r = colorBlendState->blendConstants[0];
	blendConstants.g = colorBlendState->blendConstants[1];
	blendConstants.b = colorBlendState->blendConstants[2];
	blendConstants.a = colorBlendState->blendConstants[3];

	if(colorBlendState->attachmentCount == 1)
	{
		const VkPipelineColorBlendAttachmentState& attachment = colorBlendState->pAttachments[0];
		if(attachment.colorWriteMask != (VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT))
		{
			UNIMPLEMENTED();
		}

		context.alphaBlendEnable = attachment.blendEnable;
		context.separateAlphaBlendEnable = (attachment.alphaBlendOp != attachment.colorBlendOp) ||
		                                   (attachment.dstAlphaBlendFactor != attachment.dstColorBlendFactor) ||
		                                   (attachment.srcAlphaBlendFactor != attachment.srcColorBlendFactor);
		context.blendOperationStateAlpha = Convert(attachment.alphaBlendOp);
		context.blendOperationState = Convert(attachment.colorBlendOp);
		context.destBlendFactorStateAlpha = Convert(attachment.dstAlphaBlendFactor);
		context.destBlendFactorState = Convert(attachment.dstColorBlendFactor);
		context.sourceBlendFactorStateAlpha = Convert(attachment.srcAlphaBlendFactor);
		context.sourceBlendFactorState = Convert(attachment.srcColorBlendFactor);
	}

	auto layout = Cast(pCreateInfo->layout);
	if((layout->getSetLayoutCount() != 0) ||
	   (layout->getPushConstantRangeCount() != 0))
	{
		UNIMPLEMENTED();
	}

	auto renderPass = Cast(pCreateInfo->renderPass);
	if((renderPass->getDependencyCount() != 0) ||
	   (renderPass->getAttachmentCount() != 1) ||
	   (renderPass->getSubpassCount() != 1))
	{
		UNIMPLEMENTED();
	}

	VkAttachmentDescription attachment = renderPass->getAttachment(0);
	if((attachment.flags != 0) ||
	   (attachment.format != VK_FORMAT_R8G8B8A8_UNORM) ||
	   (attachment.samples != VK_SAMPLE_COUNT_1_BIT) ||
	   (attachment.loadOp != VK_ATTACHMENT_LOAD_OP_CLEAR) ||
	   (attachment.storeOp != VK_ATTACHMENT_STORE_OP_STORE) ||
	   (attachment.stencilLoadOp != VK_ATTACHMENT_LOAD_OP_DONT_CARE) ||
	   (attachment.stencilStoreOp != VK_ATTACHMENT_STORE_OP_DONT_CARE) ||
	   !((attachment.initialLayout == VK_IMAGE_LAYOUT_UNDEFINED) ||
	     (attachment.initialLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)) ||
	   (attachment.finalLayout != VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL))
	{
		UNIMPLEMENTED();
	}

	VkSubpassDescription subpass = renderPass->getSubpass(0);
	if((subpass.flags != 0) ||
	   (subpass.pipelineBindPoint != VK_PIPELINE_BIND_POINT_GRAPHICS) ||
	   (subpass.inputAttachmentCount != 0) ||
	   (subpass.colorAttachmentCount != 1) ||
	   !((subpass.pResolveAttachments == nullptr) ||
	     (subpass.pResolveAttachments->attachment == VK_ATTACHMENT_UNUSED)) ||
	   (subpass.pDepthStencilAttachment != nullptr) ||
	   (subpass.preserveAttachmentCount != 0))
	{
		UNIMPLEMENTED();
	}

	VkAttachmentReference colorAttachment = subpass.pColorAttachments[0];
	if((colorAttachment.attachment != 0) ||
	   (colorAttachment.layout != VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL))
	{
		UNIMPLEMENTED();
	}
}

Pipeline::Pipeline(const VkComputePipelineCreateInfo* pCreateInfo, void* mem)
{
	UNIMPLEMENTED();
}

void Pipeline::destroy(const VkAllocationCallbacks* pAllocator)
{
}

size_t Pipeline::ComputeRequiredAllocationSize(const VkGraphicsPipelineCreateInfo* pCreateInfo)
{
	return 0;
}

size_t Pipeline::ComputeRequiredAllocationSize(const VkComputePipelineCreateInfo* pCreateInfo)
{
	return 0;
}

void Pipeline::bindDescriptorSets(VkPipelineLayout layout, uint32_t firstSet,
	uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets,
	uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets)
{
	/*for(uint32_t i = 0; i < descriptorSetCount; i++)
	{
		uint32_t setIndex = i + firstSet;
		pDescriptorSets[i];
	}*/
}

uint32_t Pipeline::computePrimitiveCount(uint32_t vertexCount) const
{
	switch(context.drawType)
	{
	case sw::DRAW_POINTLIST:
		return vertexCount;
	case sw::DRAW_LINELIST:
		return vertexCount / 2;
	case sw::DRAW_LINESTRIP:
		return vertexCount - 1;
	case sw::DRAW_TRIANGLELIST:
		return vertexCount / 3;
	case sw::DRAW_TRIANGLESTRIP:
		return vertexCount - 2;
	case sw::DRAW_TRIANGLEFAN:
		return vertexCount - 2;
	default:
		UNIMPLEMENTED();
	}

	return 0;
}

const sw::Context& Pipeline::getContext() const
{
	return context;
}

const sw::Rect& Pipeline::getScissor() const
{
	return scissor;
}

const sw::Viewport& Pipeline::getViewport() const
{
	return viewport;
}

const sw::Color<float>& Pipeline::getBlendConstants() const
{
	return blendConstants;
}

}