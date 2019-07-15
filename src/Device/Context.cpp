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

#include "Context.hpp"

#include "Primitive.hpp"
#include "System/Memory.hpp"
#include "Vulkan/VkDebug.hpp"
#include "Vulkan/VkImageView.hpp"
#include "Pipeline/SpirvShader.hpp"

#include <string.h>

namespace sw
{
	Context::Context()
	{
		init();
	}

	bool Context::isDrawPoint() const
	{
		switch(topology)
		{
		case VK_PRIMITIVE_TOPOLOGY_POINT_LIST:
			return true;
		case VK_PRIMITIVE_TOPOLOGY_LINE_LIST:
		case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP:
		case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST:
		case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP:
		case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN:
			break;
		default:
			UNIMPLEMENTED("topology %d", int(topology));
		}
		return false;
	}

	bool Context::isDrawLine() const
	{
		switch(topology)
		{
		case VK_PRIMITIVE_TOPOLOGY_LINE_LIST:
		case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP:
			return true;
		case VK_PRIMITIVE_TOPOLOGY_POINT_LIST:
		case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST:
		case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP:
		case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN:
			break;
		default:
			UNIMPLEMENTED("topology %d", int(topology));
		}
		return false;
	}

	bool Context::isDrawTriangle() const
	{
		switch(topology)
		{
		case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST:
		case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP:
		case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN:
			return true;
		case VK_PRIMITIVE_TOPOLOGY_POINT_LIST:
		case VK_PRIMITIVE_TOPOLOGY_LINE_LIST:
		case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP:
			break;
		default:
			UNIMPLEMENTED("topology %d", int(topology));
		}
		return false;
	}

	void Context::init()
	{
		for(int i = 0; i < RENDERTARGETS; ++i)
		{
			renderTarget[i] = nullptr;

			alphaBlendEnable[i] = false;

			sourceBlendFactorState[i] = VK_BLEND_FACTOR_ONE;
			destBlendFactorState[i] = VK_BLEND_FACTOR_ZERO;
			blendOperationState[i] = VK_BLEND_OP_ADD;

			sourceBlendFactorStateAlpha[i] = VK_BLEND_FACTOR_ONE;
			destBlendFactorStateAlpha[i] = VK_BLEND_FACTOR_ZERO;
			blendOperationStateAlpha[i] = VK_BLEND_OP_ADD;
		}
		depthBuffer = nullptr;
		stencilBuffer = nullptr;

		stencilEnable = false;
		frontStencil = {};
		backStencil = {};

		rasterizerDiscard = false;

		depthCompareMode = VK_COMPARE_OP_LESS;
		depthBoundsTestEnable = false;
		depthBufferEnable = false;
		depthWriteEnable = false;

		cullMode = VK_CULL_MODE_FRONT_BIT;
		frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

		depthBias = 0.0f;
		slopeDepthBias = 0.0f;

		for(int i = 0; i < RENDERTARGETS; i++)
		{
			colorWriteMask[i] = 0x0000000F;
		}

		pipelineLayout = nullptr;

		pixelShader = nullptr;
		vertexShader = nullptr;

		occlusionEnabled = false;

		lineWidth = 1.0f;

		sampleMask = 0xFFFFFFFF;
		alphaToCoverage = false;
	}

	bool Context::depthWriteActive() const
	{
		if(!depthBufferActive()) return false;

		return depthWriteEnable;
	}

	bool Context::depthBufferActive() const
	{
		return depthBuffer && depthBufferEnable;
	}

	bool Context::stencilActive() const
	{
		return stencilBuffer && stencilEnable;
	}

	bool Context::alphaBlendActive(int index) const
	{
		if(!alphaBlendEnable[index])
		{
			return false;
		}

		if(!colorUsed())
		{
			return false;
		}

		bool colorBlend = !(blendOperation(index) == VK_BLEND_OP_SRC_EXT && sourceBlendFactor(index) == VK_BLEND_FACTOR_ONE);
		bool alphaBlend = !(blendOperationAlpha(index) == VK_BLEND_OP_SRC_EXT && sourceBlendFactorAlpha(index) == VK_BLEND_FACTOR_ONE);

		return colorBlend || alphaBlend;
	}

	VkBlendFactor Context::sourceBlendFactor(int index) const
	{
		if(!alphaBlendEnable[index]) return VK_BLEND_FACTOR_ONE;

		switch(blendOperationState[index])
		{
		case VK_BLEND_OP_ADD:
		case VK_BLEND_OP_SUBTRACT:
		case VK_BLEND_OP_REVERSE_SUBTRACT:
			return sourceBlendFactorState[index];
		case VK_BLEND_OP_MIN:
			return VK_BLEND_FACTOR_ONE;
		case VK_BLEND_OP_MAX:
			return VK_BLEND_FACTOR_ONE;
		default:
			ASSERT(false);
		}

		return sourceBlendFactorState[index];
	}

	VkBlendFactor Context::destBlendFactor(int index) const
	{
		if(!alphaBlendEnable[index]) return VK_BLEND_FACTOR_ONE;

		switch(blendOperationState[index])
		{
		case VK_BLEND_OP_ADD:
		case VK_BLEND_OP_SUBTRACT:
		case VK_BLEND_OP_REVERSE_SUBTRACT:
			return destBlendFactorState[index];
		case VK_BLEND_OP_MIN:
			return VK_BLEND_FACTOR_ONE;
		case VK_BLEND_OP_MAX:
			return VK_BLEND_FACTOR_ONE;
		default:
			ASSERT(false);
		}

		return destBlendFactorState[index];
	}

	bool Context::allTargetsColorClamp() const
	{
		// TODO: remove all of this and support VkPhysicalDeviceFeatures::independentBlend instead
		for (int i = 0; i < RENDERTARGETS; i++)
		{
			if (renderTarget[i] && renderTarget[i]->getFormat().isFloatFormat())
			{
				return false;
			}
		}

		return true;
	}

	VkBlendOp Context::blendOperation(int index) const
	{
		if(!alphaBlendEnable[index]) return VK_BLEND_OP_SRC_EXT;

		switch(blendOperationState[index])
		{
		case VK_BLEND_OP_ADD:
			if(sourceBlendFactor(index) == VK_BLEND_FACTOR_ZERO)
			{
				if(destBlendFactor(index) == VK_BLEND_FACTOR_ZERO)
				{
					return VK_BLEND_OP_ZERO_EXT;
				}
				else
				{
					return VK_BLEND_OP_DST_EXT;
				}
			}
			else if(sourceBlendFactor(index) == VK_BLEND_FACTOR_ONE)
			{
				if(destBlendFactor(index) == VK_BLEND_FACTOR_ZERO)
				{
					return VK_BLEND_OP_SRC_EXT;
				}
				else
				{
					return VK_BLEND_OP_ADD;
				}
			}
			else
			{
				if(destBlendFactor(index) == VK_BLEND_FACTOR_ZERO)
				{
					return VK_BLEND_OP_SRC_EXT;
				}
				else
				{
					return VK_BLEND_OP_ADD;
				}
			}
		case VK_BLEND_OP_SUBTRACT:
			if(sourceBlendFactor(index) == VK_BLEND_FACTOR_ZERO && allTargetsColorClamp())
			{
				return VK_BLEND_OP_ZERO_EXT;   // Negative, clamped to zero
			}
			else if(sourceBlendFactor(index) == VK_BLEND_FACTOR_ONE)
			{
				if(destBlendFactor(index) == VK_BLEND_FACTOR_ZERO)
				{
					return VK_BLEND_OP_SRC_EXT;
				}
				else
				{
					return VK_BLEND_OP_SUBTRACT;
				}
			}
			else
			{
				if(destBlendFactor(index) == VK_BLEND_FACTOR_ZERO)
				{
					return VK_BLEND_OP_SRC_EXT;
				}
				else
				{
					return VK_BLEND_OP_SUBTRACT;
				}
			}
		case VK_BLEND_OP_REVERSE_SUBTRACT:
			if(sourceBlendFactor(index) == VK_BLEND_FACTOR_ZERO)
			{
				if(destBlendFactor(index) == VK_BLEND_FACTOR_ZERO)
				{
					return VK_BLEND_OP_ZERO_EXT;
				}
				else
				{
					return VK_BLEND_OP_DST_EXT;
				}
			}
			else if(sourceBlendFactor(index) == VK_BLEND_FACTOR_ONE)
			{
				if(destBlendFactor(index) == VK_BLEND_FACTOR_ZERO && allTargetsColorClamp())
				{
					return VK_BLEND_OP_ZERO_EXT;   // Negative, clamped to zero
				}
				else
				{
					return VK_BLEND_OP_REVERSE_SUBTRACT;
				}
			}
			else
			{
				if(destBlendFactor(index) == VK_BLEND_FACTOR_ZERO && allTargetsColorClamp())
				{
					return VK_BLEND_OP_ZERO_EXT;   // Negative, clamped to zero
				}
				else
				{
					return VK_BLEND_OP_REVERSE_SUBTRACT;
				}
			}
		case VK_BLEND_OP_MIN:
			return VK_BLEND_OP_MIN;
		case VK_BLEND_OP_MAX:
			return VK_BLEND_OP_MAX;
		default:
			ASSERT(false);
		}

		return blendOperationState[index];
	}

	VkBlendFactor Context::sourceBlendFactorAlpha(int index) const
	{
		switch (blendOperationStateAlpha[index])
		{
		case VK_BLEND_OP_ADD:
		case VK_BLEND_OP_SUBTRACT:
		case VK_BLEND_OP_REVERSE_SUBTRACT:
			return sourceBlendFactorStateAlpha[index];
		case VK_BLEND_OP_MIN:
			return VK_BLEND_FACTOR_ONE;
		case VK_BLEND_OP_MAX:
			return VK_BLEND_FACTOR_ONE;
		default:
			ASSERT(false);
		}

		return sourceBlendFactorStateAlpha[index];
	}

	VkBlendFactor Context::destBlendFactorAlpha(int index) const
	{
		switch (blendOperationStateAlpha[index])
		{
		case VK_BLEND_OP_ADD:
		case VK_BLEND_OP_SUBTRACT:
		case VK_BLEND_OP_REVERSE_SUBTRACT:
			return destBlendFactorStateAlpha[index];
		case VK_BLEND_OP_MIN:
			return VK_BLEND_FACTOR_ONE;
		case VK_BLEND_OP_MAX:
			return VK_BLEND_FACTOR_ONE;
		default:
			ASSERT(false);
		}

		return destBlendFactorStateAlpha[index];
	}

	VkBlendOp Context::blendOperationAlpha(int index) const
	{
		switch (blendOperationStateAlpha[index])
		{
		case VK_BLEND_OP_ADD:
			if (sourceBlendFactorAlpha(index) == VK_BLEND_FACTOR_ZERO)
			{
				if (destBlendFactorAlpha(index) == VK_BLEND_FACTOR_ZERO)
				{
					return VK_BLEND_OP_ZERO_EXT;
				}
				else
				{
					return VK_BLEND_OP_DST_EXT;
				}
			}
			else if (sourceBlendFactorAlpha(index) == VK_BLEND_FACTOR_ONE)
			{
				if (destBlendFactorAlpha(index) == VK_BLEND_FACTOR_ZERO)
				{
					return VK_BLEND_OP_SRC_EXT;
				}
				else
				{
					return VK_BLEND_OP_ADD;
				}
			}
			else
			{
				if (destBlendFactorAlpha(index) == VK_BLEND_FACTOR_ZERO)
				{
					return VK_BLEND_OP_SRC_EXT;
				}
				else
				{
					return VK_BLEND_OP_ADD;
				}
			}
		case VK_BLEND_OP_SUBTRACT:
			if (sourceBlendFactorAlpha(index) == VK_BLEND_FACTOR_ZERO && allTargetsColorClamp())
			{
				return VK_BLEND_OP_ZERO_EXT;   // Negative, clamped to zero
			}
			else if (sourceBlendFactorAlpha(index) == VK_BLEND_FACTOR_ONE)
			{
				if (destBlendFactorAlpha(index) == VK_BLEND_FACTOR_ZERO)
				{
					return VK_BLEND_OP_SRC_EXT;
				}
				else
				{
					return VK_BLEND_OP_SUBTRACT;
				}
			}
			else
			{
				if (destBlendFactorAlpha(index) == VK_BLEND_FACTOR_ZERO)
				{
					return VK_BLEND_OP_SRC_EXT;
				}
				else
				{
					return VK_BLEND_OP_SUBTRACT;
				}
			}
		case VK_BLEND_OP_REVERSE_SUBTRACT:
			if (sourceBlendFactorAlpha(index) == VK_BLEND_FACTOR_ZERO)
			{
				if (destBlendFactorAlpha(index) == VK_BLEND_FACTOR_ZERO)
				{
					return VK_BLEND_OP_ZERO_EXT;
				}
				else
				{
					return VK_BLEND_OP_DST_EXT;
				}
			}
			else if (sourceBlendFactorAlpha(index) == VK_BLEND_FACTOR_ONE)
			{
				if (destBlendFactorAlpha(index) == VK_BLEND_FACTOR_ZERO && allTargetsColorClamp())
				{
					return VK_BLEND_OP_ZERO_EXT;   // Negative, clamped to zero
				}
				else
				{
					return VK_BLEND_OP_REVERSE_SUBTRACT;
				}
			}
			else
			{
				if (destBlendFactorAlpha(index) == VK_BLEND_FACTOR_ZERO && allTargetsColorClamp())
				{
					return VK_BLEND_OP_ZERO_EXT;   // Negative, clamped to zero
				}
				else
				{
					return VK_BLEND_OP_REVERSE_SUBTRACT;
				}
			}
		case VK_BLEND_OP_MIN:
			return VK_BLEND_OP_MIN;
		case VK_BLEND_OP_MAX:
			return VK_BLEND_OP_MAX;
		default:
			ASSERT(false);
		}

		return blendOperationStateAlpha[index];
	}

	VkFormat Context::renderTargetInternalFormat(int index) const
	{
		if(renderTarget[index])
		{
			return renderTarget[index]->getFormat();
		}
		else
		{
			return VK_FORMAT_UNDEFINED;
		}
	}

	bool Context::colorWriteActive() const
	{
		for (int i = 0; i < RENDERTARGETS; i++)
		{
			if (colorWriteActive(i))
			{
				return true;
			}
		}

		return false;
	}

	int Context::colorWriteActive(int index) const
	{
		if(!renderTarget[index] || renderTarget[index]->getFormat() == VK_FORMAT_UNDEFINED)
		{
			return 0;
		}

		if(blendOperation(index) == VK_BLEND_OP_DST_EXT && destBlendFactor(index) == VK_BLEND_FACTOR_ONE &&
		   (blendOperationAlpha(index) == VK_BLEND_OP_DST_EXT && destBlendFactorAlpha(index) == VK_BLEND_FACTOR_ONE))
		{
			return 0;
		}

		return colorWriteMask[index];
	}

	bool Context::colorUsed() const
	{
		return colorWriteActive() || (pixelShader && pixelShader->getModes().ContainsKill);
	}
}
