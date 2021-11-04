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

#include "SpirvShader.hpp"

#include "SamplerCore.hpp"
#include "Device/Config.hpp"
#include "Pipeline/Constants.hpp"
#include "System/Debug.hpp"
#include "System/Math.hpp"
#include "Vulkan/VkDescriptorSetLayout.hpp"
#include "Vulkan/VkDevice.hpp"
#include "Vulkan/VkImageView.hpp"
#include "Vulkan/VkSampler.hpp"

#include <spirv/unified1/spirv.hpp>

#include <climits>
#include <mutex>

namespace sw {

SpirvShader::ImageSampler *SpirvShader::getImageSampler(const sw::Constants *constants, uint32_t inst, uint32_t samplerId, uint32_t imageViewId)
{
	const vk::Device *device = constants->device;

	ImageInstruction instruction(inst);
	ASSERT(imageViewId != 0 && (samplerId != 0 || instruction.samplerMethod == Fetch || instruction.samplerMethod == Write));
	ASSERT(device);

	vk::Device::SamplingRoutineCache::Key key = { inst, samplerId, imageViewId };

	auto createSamplingRoutine = [&device](const vk::Device::SamplingRoutineCache::Key &key) {
		ImageInstruction instruction(key.instruction);
		const vk::Identifier::State imageViewState = vk::Identifier(key.imageView).getState();
		const vk::SamplerState *vkSamplerState = (key.sampler != 0) ? device->findSampler(key.sampler) : nullptr;

		auto type = imageViewState.imageViewType;
		auto samplerMethod = static_cast<SamplerMethod>(instruction.samplerMethod);

		Sampler samplerState = {};
		samplerState.textureType = type;
		samplerState.textureFormat = imageViewState.format;

		samplerState.addressingModeU = convertAddressingMode(0, vkSamplerState, type);
		samplerState.addressingModeV = convertAddressingMode(1, vkSamplerState, type);
		samplerState.addressingModeW = convertAddressingMode(2, vkSamplerState, type);

		samplerState.mipmapFilter = convertMipmapMode(vkSamplerState);
		samplerState.swizzle = imageViewState.mapping;
		samplerState.gatherComponent = instruction.gatherComponent;

		if(vkSamplerState)
		{
			samplerState.textureFilter = convertFilterMode(vkSamplerState, type, samplerMethod);
			samplerState.border = vkSamplerState->borderColor;
			samplerState.customBorder = vkSamplerState->customBorderColor;

			samplerState.mipmapFilter = convertMipmapMode(vkSamplerState);
			samplerState.highPrecisionFiltering = (vkSamplerState->filteringPrecision == VK_SAMPLER_FILTERING_PRECISION_MODE_HIGH_GOOGLE);

			samplerState.compareEnable = (vkSamplerState->compareEnable != VK_FALSE);
			samplerState.compareOp = vkSamplerState->compareOp;
			samplerState.unnormalizedCoordinates = (vkSamplerState->unnormalizedCoordinates != VK_FALSE);

			samplerState.ycbcrModel = vkSamplerState->ycbcrModel;
			samplerState.studioSwing = vkSamplerState->studioSwing;
			samplerState.swappedChroma = vkSamplerState->swappedChroma;

			samplerState.mipLodBias = vkSamplerState->mipLodBias;
			samplerState.maxAnisotropy = vkSamplerState->maxAnisotropy;
			samplerState.minLod = vkSamplerState->minLod;
			samplerState.maxLod = vkSamplerState->maxLod;

			// If there's a single mip level and filtering doesn't depend on the LOD level,
			// the sampler will need to compute the LOD to produce the proper result.
			// Otherwise, it can be ignored.
			// We can skip the LOD computation for all modes, except LOD query,
			// where we have to return the proper value even if nothing else requires it.
			if(imageViewState.singleMipLevel &&
			   (samplerState.textureFilter != FILTER_MIN_POINT_MAG_LINEAR) &&
			   (samplerState.textureFilter != FILTER_MIN_LINEAR_MAG_POINT) &&
			   (samplerMethod != Query))
			{
				samplerState.minLod = 0.0f;
				samplerState.maxLod = 0.0f;
			}
		}
		else if(samplerMethod == Fetch)
		{
			// OpImageFetch does not take a sampler descriptor, but for VK_EXT_image_robustness
			// requires replacing invalid texels with zero.
			// TODO(b/162327166): Only perform bounds checks when VK_EXT_image_robustness is enabled.
			samplerState.border = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;

			// If there's a single mip level we can skip LOD computation.
			if(imageViewState.singleMipLevel)
			{
				samplerState.minLod = 0.0f;
				samplerState.maxLod = 0.0f;
			}
		}
		else if(samplerMethod == Write)
		{
			//////////////////////////////////

			return emitWriteRoutine(instruction, samplerState);
		}
		else
			ASSERT(false);

		return emitSamplerRoutine(instruction, samplerState);
	};

	vk::Device::SamplingRoutineCache *cache = device->getSamplingRoutineCache();
	auto routine = cache->getOrCreate(key, createSamplingRoutine);

	return (ImageSampler *)(routine->getEntry());
}

/*static int boo()
{
    return 1;
}*/

static int uv2()
{
	return 1;
}

//static SIMD::Pointer GetTexelAddress(EmitState const *state, Pointer<Byte> imageBase, Int imageSizeInBytes, Operand const &coordinate, Type const &imageType, Pointer<Byte> descriptor, int texelSize, Object::ID sampleId, bool useStencilAspect, OutOfBoundsBehavior outOfBoundsBehavior)
//static SIMD::Pointer GetTexelAddress(Pointer<Byte> imageBase, Int imageSizeInBytes, const Operand &coordinate, const Type &imageType, Pointer<Byte> descriptor, int texelSize, Object::ID sampleId, bool useStencilAspect, OutOfBoundsBehavior outOfBoundsBehavior)
static SIMD::Pointer GetTexelAddress(const Sampler &samplerState, Pointer<Byte> descriptor, SIMD::Float coordinate[4], int texelSize, bool useStencilAspect, OutOfBoundsBehavior outOfBoundsBehavior)
{
	//auto routine = state->routine;
	bool isArrayed = samplerState.isArrayed();
	/////auto dim = spv::Dim2D;  //////////////////// static_cast<spv::Dim>(imageType.definition.word(3));
	int dims = samplerState.dims();

	SIMD::Int u = As<SIMD::Int>(coordinate[0]);
	SIMD::Int v = SIMD::Int(0);

	//if(coordinate.componentCount > 1)
	//{
	v = As<SIMD::Int>(coordinate[1]);  ////////////////////v = coordinate.Int(1);
	//}

	Call(uv2);

	//if(dim == spv::DimSubpassData)
	//{
	//	u += routine->windowSpacePosition[0];
	//	v += routine->windowSpacePosition[1];
	//}

	///////////////////////////////SIMD::Int rowPitch = *Pointer<Int>(descriptor + (useStencilAspect
	auto rowPitch = SIMD::Int(*Pointer<Int>(descriptor + (useStencilAspect
	                                                          ? OFFSET(vk::StorageImageDescriptor, stencilRowPitchBytes)
	                                                          : OFFSET(vk::StorageImageDescriptor, rowPitchBytes))));
	auto slicePitch = SIMD::Int(
	    *Pointer<Int>(descriptor + (useStencilAspect
	                                    ? OFFSET(vk::StorageImageDescriptor, stencilSlicePitchBytes)
	                                    : OFFSET(vk::StorageImageDescriptor, slicePitchBytes))));
	auto samplePitch = SIMD::Int(
	    *Pointer<Int>(descriptor + (useStencilAspect
	                                    ? OFFSET(vk::StorageImageDescriptor, stencilSamplePitchBytes)
	                                    : OFFSET(vk::StorageImageDescriptor, samplePitchBytes))));

	SIMD::Int ptrOffset = u * SIMD::Int(texelSize);

	if(dims > 1)
	{
		ptrOffset += v * rowPitch;
	}

	SIMD::Int w = 0;
	if((dims > 2) || isArrayed)
	{
		if(dims > 2)
		{
			w += As<SIMD::Int>(coordinate[2]);  /////////// coordinate.Int(2);
		}

		if(isArrayed)
		{
			w += As<SIMD::Int>(coordinate[dims]);  //////////////////// coordinate.Int(dims);
		}

		ptrOffset += w * slicePitch;
	}

	//if(dim == spv::DimSubpassData)
	//{
	//	// Multiview input attachment access is to the layer corresponding to the current view
	//	///////////////////////////////////////////////ptrOffset += SIMD::Int(routine->viewID) * slicePitch;
	//}

	//SIMD::Int n = 0;
	//if(sampleId.value())
	//{
	//	Operand sample(this, state, sampleId);
	//	if(!sample.isConstantZero())
	//	{
	//		n = sample.Int(0);
	//		ptrOffset += n * samplePitch;
	//	}
	//}

	// If the out-of-bounds behavior is set to nullify, then each coordinate must be tested individually.
	// Other out-of-bounds behaviors work properly by just comparing the offset against the total size.
	if(outOfBoundsBehavior == OutOfBoundsBehavior::Nullify)
	{
		SIMD::UInt width = *Pointer<UInt>(descriptor + OFFSET(vk::StorageImageDescriptor, width));
		SIMD::Int oobMask = As<SIMD::Int>(CmpNLT(As<SIMD::UInt>(u), width));

		if(dims > 1)
		{
			SIMD::UInt height = *Pointer<UInt>(descriptor + OFFSET(vk::StorageImageDescriptor, height));
			oobMask |= As<SIMD::Int>(CmpNLT(As<SIMD::UInt>(v), height));
		}

		if((dims > 2) || isArrayed)
		{
			UInt depth = *Pointer<UInt>(descriptor + OFFSET(vk::StorageImageDescriptor, depth));
			if(samplerState.isCube()) { depth *= 6; }
			oobMask |= As<SIMD::Int>(CmpNLT(As<SIMD::UInt>(w), SIMD::UInt(depth)));
		}

		//if(sampleId.value())
		//{
		//	Operand sample(this, state, sampleId);
		//	if(!sample.isConstantZero())
		//	{
		//		SIMD::UInt sampleCount = *Pointer<UInt>(descriptor + OFFSET(vk::StorageImageDescriptor, sampleCount));
		//		oobMask |= As<SIMD::Int>(CmpNLT(As<SIMD::UInt>(n), sampleCount));
		//	}
		//}

		constexpr int32_t OOB_OFFSET = 0x7FFFFFFF - 16;  // SIMD pointer offsets are signed 32-bit, so this is the largest offset (for 16-byte texels).
		static_assert(OOB_OFFSET >= vk::MAX_MEMORY_ALLOCATION_SIZE, "the largest offset must be guaranteed to be out-of-bounds");

		ptrOffset = (ptrOffset & ~oobMask) | (oobMask & SIMD::Int(OOB_OFFSET));  // oob ? OOB_OFFSET : ptrOffset  // TODO: IfThenElse()
	}

	Pointer<Byte> imageBase = *Pointer<Pointer<Byte>>(descriptor + (useStencilAspect
	                                                                    ? OFFSET(vk::StorageImageDescriptor, stencilPtr)
	                                                                    : OFFSET(vk::StorageImageDescriptor, ptr)));

	Int imageSizeInBytes = *Pointer<Int>(descriptor + OFFSET(vk::StorageImageDescriptor, sizeInBytes));

	return SIMD::Pointer(imageBase, imageSizeInBytes, ptrOffset);
}

std::shared_ptr<rr::Routine> SpirvShader::emitWriteRoutine(ImageInstruction instruction, const Sampler &samplerState)
{
	// TODO(b/129523279): Hold a separate mutex lock for the sampler being built.
	rr::Function<Void(Pointer<Byte>, Pointer<SIMD::Float>, Pointer<SIMD::Float>, Pointer<Byte>)> function;
	{
		Pointer<Byte> texture = function.Arg<0>();
		Pointer<SIMD::Float> in = function.Arg<1>();
		Pointer<SIMD::Float> out = function.Arg<2>();
		Pointer<Byte> constants = function.Arg<3>();

		SIMD::Float uvwa[4];
		//SIMD::Float dRef;
		//SIMD::Float lodOrBias;  // Explicit level-of-detail, or bias added to the implicit level-of-detail (depending on samplerMethod).
		//Vector4f dsx;
		//Vector4f dsy;
		//Vector4i offset;
		SIMD::Int sampleId;
		///////////////SamplerFunction samplerFunction = instruction.getSamplerFunction();

		uint32_t i = 0;
		for(; i < instruction.coordinates; i++)
		{
			uvwa[i] = in[i];
		}

		//if(instruction.isDref())/////////////////////////////////////////////////////////////////////
		//{
		//	dRef = in[i];
		//	i++;
		//}

		//if(instruction.samplerMethod == Lod || instruction.samplerMethod == Bias || instruction.samplerMethod == Fetch)
		//{
		//	lodOrBias = in[i];
		//	i++;
		//}
		//else if(instruction.samplerMethod == Grad)
		//{
		//	for(uint32_t j = 0; j < instruction.grad; j++, i++)
		//	{
		//		dsx[j] = in[i];
		//	}

		//	for(uint32_t j = 0; j < instruction.grad; j++, i++)
		//	{
		//		dsy[j] = in[i];
		//	}
		//}

		//for(uint32_t j = 0; j < instruction.offset; j++, i++)
		//{
		//	offset[j] = As<SIMD::Int>(in[i]);
		//}

		if(instruction.sample)
		{
			sampleId = As<SIMD::Int>(in[i]);
		}

		///////////////////////////////////////////////////////////////////////////////////////

		//auto imageId = Object::ID(insn.word(1));
		//auto &image = getObject(imageId);
		//auto &imageType = getType(image);

		//ASSERT(imageType.definition.opcode() == spv::OpTypeImage);

		//Object::ID sampleId = 0;

		//if(insn.wordCount() > 4)
		//{
		//	int operand = 5;
		//	uint32_t imageOperands = insn.word(4);
		//	if(imageOperands & spv::ImageOperandsSampleMask)
		//	{
		//		sampleId = insn.word(operand++);
		//		imageOperands &= ~spv::ImageOperandsSampleMask;
		//	}
		//	// TODO(b/174475384)
		//	if(imageOperands & spv::ImageOperandsZeroExtendMask)
		//	{
		//		imageOperands &= ~spv::ImageOperandsZeroExtendMask;
		//	}
		//	else if(imageOperands & spv::ImageOperandsSignExtendMask)
		//	{
		//		imageOperands &= ~spv::ImageOperandsSignExtendMask;
		//	}

		//	// Should be no remaining image operands.
		//	if(imageOperands != 0)
		//	{
		//		UNSUPPORTED("Image operands 0x%08X", (int)imageOperands);
		//	}
		//}

		//auto coordinate = Operand(this, state, insn.word(2));
		//auto texel = Operand(this, state, insn.word(3));

		Pointer<Byte> descriptor = texture;  //////////////// state->getPointer(imageId).base;
		//Pointer<Byte> imageBase = *Pointer<Pointer<Byte>>(binding + OFFSET(vk::StorageImageDescriptor, ptr));
		//auto imageSizeInBytes = *Pointer<Int>(binding + OFFSET(vk::StorageImageDescriptor, sizeInBytes));

		Intermediate tt(4);  ////////////////////////////////////////////////
		tt.move(0, out[0]);
		tt.move(1, out[1]);
		tt.move(2, out[2]);
		tt.move(3, out[3]);
		Operand texel(tt);

		//Call(boo);

		SIMD::Int packed[4];
		int texelSize = 0;
		///////Int format = *Pointer<Int>(descriptor + OFFSET(vk::StorageImageDescriptor, format));  ////////////////////////////////////// static_cast<spv::ImageFormat>(imageType.definition.word(8));
		vk::Format format = samplerState.textureFormat;
		switch(format)
		{
		case VK_FORMAT_R32G32B32A32_SFLOAT:
		case VK_FORMAT_R32G32B32A32_SINT:
		case VK_FORMAT_R32G32B32A32_UINT:
			texelSize = 16;
			packed[0] = texel.Int(0);
			packed[1] = texel.Int(1);
			packed[2] = texel.Int(2);
			packed[3] = texel.Int(3);
			break;
		case VK_FORMAT_R32_SFLOAT:
		case VK_FORMAT_R32_SINT:
		case VK_FORMAT_R32_UINT:
			texelSize = 4;
			packed[0] = texel.Int(0);
			break;
		case VK_FORMAT_R8G8B8A8_UNORM:
			texelSize = 4;
			packed[0] = (SIMD::UInt(Round(Min(Max(texel.Float(0), SIMD::Float(0.0f)), SIMD::Float(1.0f)) * SIMD::Float(255.0f)))) |
			            ((SIMD::UInt(Round(Min(Max(texel.Float(1), SIMD::Float(0.0f)), SIMD::Float(1.0f)) * SIMD::Float(255.0f)))) << 8) |
			            ((SIMD::UInt(Round(Min(Max(texel.Float(2), SIMD::Float(0.0f)), SIMD::Float(1.0f)) * SIMD::Float(255.0f)))) << 16) |
			            ((SIMD::UInt(Round(Min(Max(texel.Float(3), SIMD::Float(0.0f)), SIMD::Float(1.0f)) * SIMD::Float(255.0f)))) << 24);
			break;
		case VK_FORMAT_R8G8B8A8_SNORM:
			texelSize = 4;
			packed[0] = (SIMD::Int(Round(Min(Max(texel.Float(0), SIMD::Float(-1.0f)), SIMD::Float(1.0f)) * SIMD::Float(127.0f))) &
			             SIMD::Int(0xFF)) |
			            ((SIMD::Int(Round(Min(Max(texel.Float(1), SIMD::Float(-1.0f)), SIMD::Float(1.0f)) * SIMD::Float(127.0f))) &
			              SIMD::Int(0xFF))
			             << 8) |
			            ((SIMD::Int(Round(Min(Max(texel.Float(2), SIMD::Float(-1.0f)), SIMD::Float(1.0f)) * SIMD::Float(127.0f))) &
			              SIMD::Int(0xFF))
			             << 16) |
			            ((SIMD::Int(Round(Min(Max(texel.Float(3), SIMD::Float(-1.0f)), SIMD::Float(1.0f)) * SIMD::Float(127.0f))) &
			              SIMD::Int(0xFF))
			             << 24);
			break;
		case VK_FORMAT_R8G8B8A8_SINT:
		case VK_FORMAT_R8G8B8A8_UINT:
			texelSize = 4;
			packed[0] = (SIMD::UInt(texel.UInt(0) & SIMD::UInt(0xff))) |
			            (SIMD::UInt(texel.UInt(1) & SIMD::UInt(0xff)) << 8) |
			            (SIMD::UInt(texel.UInt(2) & SIMD::UInt(0xff)) << 16) |
			            (SIMD::UInt(texel.UInt(3) & SIMD::UInt(0xff)) << 24);
			break;
		case VK_FORMAT_R16G16B16A16_SFLOAT:
			texelSize = 8;
			packed[0] = floatToHalfBits(texel.UInt(0), false) | floatToHalfBits(texel.UInt(1), true);
			packed[1] = floatToHalfBits(texel.UInt(2), false) | floatToHalfBits(texel.UInt(3), true);
			break;
		case VK_FORMAT_R16G16B16A16_SINT:
		case VK_FORMAT_R16G16B16A16_UINT:
			texelSize = 8;
			packed[0] = SIMD::UInt(texel.UInt(0) & SIMD::UInt(0xFFFF)) | (SIMD::UInt(texel.UInt(1) & SIMD::UInt(0xFFFF)) << 16);
			packed[1] = SIMD::UInt(texel.UInt(2) & SIMD::UInt(0xFFFF)) | (SIMD::UInt(texel.UInt(3) & SIMD::UInt(0xFFFF)) << 16);
			break;
		case VK_FORMAT_R32G32_SFLOAT:
		case VK_FORMAT_R32G32_SINT:
		case VK_FORMAT_R32G32_UINT:
			texelSize = 8;
			packed[0] = texel.Int(0);
			packed[1] = texel.Int(1);
			break;
		case VK_FORMAT_R16G16_SFLOAT:
			texelSize = 4;
			packed[0] = floatToHalfBits(texel.UInt(0), false) | floatToHalfBits(texel.UInt(1), true);
			break;
		case VK_FORMAT_R16G16_SINT:
		case VK_FORMAT_R16G16_UINT:
			texelSize = 4;
			packed[0] = SIMD::UInt(texel.UInt(0) & SIMD::UInt(0xFFFF)) | (SIMD::UInt(texel.UInt(1) & SIMD::UInt(0xFFFF)) << 16);
			break;
		case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
			texelSize = 4;
			// Truncates instead of rounding. See b/147900455
			packed[0] = ((floatToHalfBits(As<SIMD::UInt>(Max(texel.Float(0), SIMD::Float(0.0f))), false) & SIMD::UInt(0x7FF0)) >> 4) |
			            ((floatToHalfBits(As<SIMD::UInt>(Max(texel.Float(1), SIMD::Float(0.0f))), false) & SIMD::UInt(0x7FF0)) << 7) |
			            ((floatToHalfBits(As<SIMD::UInt>(Max(texel.Float(2), SIMD::Float(0.0f))), false) & SIMD::UInt(0x7FE0)) << 17);
			break;
		case VK_FORMAT_R16_SFLOAT:
			texelSize = 2;
			packed[0] = floatToHalfBits(texel.UInt(0), false);
			break;
		case VK_FORMAT_R16G16B16A16_UNORM:
			texelSize = 8;
			packed[0] = SIMD::UInt(Round(Min(Max(texel.Float(0), SIMD::Float(0.0f)), SIMD::Float(1.0f)) * SIMD::Float(0xFFFF))) |
			            (SIMD::UInt(Round(Min(Max(texel.Float(1), SIMD::Float(0.0f)), SIMD::Float(1.0f)) * SIMD::Float(0xFFFF))) << 16);
			packed[1] = SIMD::UInt(Round(Min(Max(texel.Float(2), SIMD::Float(0.0f)), SIMD::Float(1.0f)) * SIMD::Float(0xFFFF))) |
			            (SIMD::UInt(Round(Min(Max(texel.Float(3), SIMD::Float(0.0f)), SIMD::Float(1.0f)) * SIMD::Float(0xFFFF))) << 16);
			break;
		case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
			texelSize = 4;
			packed[0] = (SIMD::UInt(Round(Min(Max(texel.Float(0), SIMD::Float(0.0f)), SIMD::Float(1.0f)) * SIMD::Float(0x3FF)))) |
			            ((SIMD::UInt(Round(Min(Max(texel.Float(1), SIMD::Float(0.0f)), SIMD::Float(1.0f)) * SIMD::Float(0x3FF)))) << 10) |
			            ((SIMD::UInt(Round(Min(Max(texel.Float(2), SIMD::Float(0.0f)), SIMD::Float(1.0f)) * SIMD::Float(0x3FF)))) << 20) |
			            ((SIMD::UInt(Round(Min(Max(texel.Float(3), SIMD::Float(0.0f)), SIMD::Float(1.0f)) * SIMD::Float(0x3)))) << 30);
			break;
		case VK_FORMAT_R16G16_UNORM:
			texelSize = 4;
			packed[0] = SIMD::UInt(Round(Min(Max(texel.Float(0), SIMD::Float(0.0f)), SIMD::Float(1.0f)) * SIMD::Float(0xFFFF))) |
			            (SIMD::UInt(Round(Min(Max(texel.Float(1), SIMD::Float(0.0f)), SIMD::Float(1.0f)) * SIMD::Float(0xFFFF))) << 16);
			break;
		case VK_FORMAT_R8G8_UNORM:
			texelSize = 2;
			packed[0] = SIMD::UInt(Round(Min(Max(texel.Float(0), SIMD::Float(0.0f)), SIMD::Float(1.0f)) * SIMD::Float(0xFF))) |
			            (SIMD::UInt(Round(Min(Max(texel.Float(1), SIMD::Float(0.0f)), SIMD::Float(1.0f)) * SIMD::Float(0xFF))) << 8);
			break;
		case VK_FORMAT_R16_UNORM:
			texelSize = 2;
			packed[0] = SIMD::UInt(Round(Min(Max(texel.Float(0), SIMD::Float(0.0f)), SIMD::Float(1.0f)) * SIMD::Float(0xFFFF)));
			break;
		case VK_FORMAT_R8_UNORM:
			texelSize = 1;
			packed[0] = SIMD::UInt(Round(Min(Max(texel.Float(0), SIMD::Float(0.0f)), SIMD::Float(1.0f)) * SIMD::Float(0xFF)));
			break;
		case VK_FORMAT_R16G16B16A16_SNORM:
			texelSize = 8;
			packed[0] = (SIMD::Int(Round(Min(Max(texel.Float(0), SIMD::Float(-1.0f)), SIMD::Float(1.0f)) * SIMD::Float(0x7FFF))) & SIMD::Int(0xFFFF)) |
			            (SIMD::Int(Round(Min(Max(texel.Float(1), SIMD::Float(-1.0f)), SIMD::Float(1.0f)) * SIMD::Float(0x7FFF))) << 16);
			packed[1] = (SIMD::Int(Round(Min(Max(texel.Float(2), SIMD::Float(-1.0f)), SIMD::Float(1.0f)) * SIMD::Float(0x7FFF))) & SIMD::Int(0xFFFF)) |
			            (SIMD::Int(Round(Min(Max(texel.Float(3), SIMD::Float(-1.0f)), SIMD::Float(1.0f)) * SIMD::Float(0x7FFF))) << 16);
			break;
		case VK_FORMAT_R16G16_SNORM:
			texelSize = 4;
			packed[0] = (SIMD::Int(Round(Min(Max(texel.Float(0), SIMD::Float(-1.0f)), SIMD::Float(1.0f)) * SIMD::Float(0x7FFF))) & SIMD::Int(0xFFFF)) |
			            (SIMD::Int(Round(Min(Max(texel.Float(1), SIMD::Float(-1.0f)), SIMD::Float(1.0f)) * SIMD::Float(0x7FFF))) << 16);
			break;
		case VK_FORMAT_R8G8_SNORM:
			texelSize = 2;
			packed[0] = (SIMD::Int(Round(Min(Max(texel.Float(0), SIMD::Float(-1.0f)), SIMD::Float(1.0f)) * SIMD::Float(0x7F))) & SIMD::Int(0xFF)) |
			            (SIMD::Int(Round(Min(Max(texel.Float(1), SIMD::Float(-1.0f)), SIMD::Float(1.0f)) * SIMD::Float(0x7F))) << 8);
			break;
		case VK_FORMAT_R16_SNORM:
			texelSize = 2;
			packed[0] = SIMD::Int(Round(Min(Max(texel.Float(0), SIMD::Float(-1.0f)), SIMD::Float(1.0f)) * SIMD::Float(0x7FFF)));
			break;
		case VK_FORMAT_R8_SNORM:
			texelSize = 1;
			packed[0] = SIMD::Int(Round(Min(Max(texel.Float(0), SIMD::Float(-1.0f)), SIMD::Float(1.0f)) * SIMD::Float(0x7F)));
			break;
		case VK_FORMAT_R8G8_SINT:
		case VK_FORMAT_R8G8_UINT:
			texelSize = 2;
			packed[0] = SIMD::UInt(texel.UInt(0) & SIMD::UInt(0xFF)) | (SIMD::UInt(texel.UInt(1) & SIMD::UInt(0xFF)) << 8);
			break;
		case VK_FORMAT_R16_SINT:
		case VK_FORMAT_R16_UINT:
			texelSize = 2;
			packed[0] = SIMD::UInt(texel.UInt(0) & SIMD::UInt(0xFFFF));
			break;
		case VK_FORMAT_R8_SINT:
		case VK_FORMAT_R8_UINT:
			texelSize = 1;
			packed[0] = SIMD::UInt(texel.UInt(0) & SIMD::UInt(0xFF));
			break;
		case VK_FORMAT_A2B10G10R10_UINT_PACK32:
			texelSize = 4;
			packed[0] = (SIMD::UInt(texel.UInt(0) & SIMD::UInt(0x3FF))) |
			            (SIMD::UInt(texel.UInt(1) & SIMD::UInt(0x3FF)) << 10) |
			            (SIMD::UInt(texel.UInt(2) & SIMD::UInt(0x3FF)) << 20) |
			            (SIMD::UInt(texel.UInt(3) & SIMD::UInt(0x3)) << 30);
			break;
		default:
			UNSUPPORTED("VkFormat %d", int(format));
			break;
		}

		// "The integer texel coordinates are validated according to the same rules as for texel input coordinate
		//  validation. If the texel fails integer texel coordinate validation, then the write has no effect."
		// - https://www.khronos.org/registry/vulkan/specs/1.2/html/chap16.html#textures-output-coordinate-validation
		auto robustness = OutOfBoundsBehavior::Nullify;

		//SIMD::Pointer texelPtr = GetTexelAddress(state, imageBase, imageSizeInBytes, coordinate, imageType, binding, texelSize, sampleId, false, robustness);
		SIMD::Pointer texelPtr = sw::/***********/ GetTexelAddress(samplerState, descriptor, uvwa, texelSize, false, robustness);

		SIMD::Int mask = As<SIMD::Int>(out[4]);

		// Scatter packed texel data.
		// TODO(b/160531165): Provide scatter abstractions for various element sizes.
		if(texelSize == 4 || texelSize == 8 || texelSize == 16)
		{
			for(auto i = 0; i < texelSize / 4; i++)
			{
				texelPtr.Store(packed[i], robustness, mask);  ////////////////
				texelPtr += sizeof(float);
			}
		}
		else if(texelSize == 2)
		{
			SIMD::Int offsets = texelPtr.offsets();

			for(int i = 0; i < SIMD::Width; i++)
			{
				If(Extract(mask, i) != 0)
				{
					*Pointer<Short>(texelPtr.base + Extract(offsets, i)) = Short(Extract(packed[0], i));
				}
			}
		}
		else if(texelSize == 1)
		{
			SIMD::Int offsets = texelPtr.offsets();

			for(int i = 0; i < SIMD::Width; i++)
			{
				If(Extract(mask, i) != 0)
				{
					*Pointer<Byte>(texelPtr.base + Extract(offsets, i)) = Byte(Extract(packed[0], i));
				}
			}
		}
		else
			UNREACHABLE("texelSize: %d", int(texelSize));

		/////////////////////////////////////////////////////////////////////////////////
	}

	return function("sampler");
}

std::shared_ptr<rr::Routine> SpirvShader::emitSamplerRoutine(ImageInstruction instruction, const Sampler &samplerState)
{
	// TODO(b/129523279): Hold a separate mutex lock for the sampler being built.
	rr::Function<Void(Pointer<Byte>, Pointer<SIMD::Float>, Pointer<SIMD::Float>, Pointer<Byte>)> function;
	{
		Pointer<Byte> texture = function.Arg<0>();
		Pointer<SIMD::Float> in = function.Arg<1>();
		Pointer<SIMD::Float> out = function.Arg<2>();
		Pointer<Byte> constants = function.Arg<3>();

		SIMD::Float uvwa[4];
		SIMD::Float dRef;
		SIMD::Float lodOrBias;  // Explicit level-of-detail, or bias added to the implicit level-of-detail (depending on samplerMethod).
		Vector4f dsx;
		Vector4f dsy;
		Vector4i offset;
		SIMD::Int sampleId;
		SamplerFunction samplerFunction = instruction.getSamplerFunction();

		uint32_t i = 0;
		for(; i < instruction.coordinates; i++)
		{
			uvwa[i] = in[i];
		}

		if(instruction.isDref())
		{
			dRef = in[i];
			i++;
		}

		if(instruction.samplerMethod == Lod || instruction.samplerMethod == Bias || instruction.samplerMethod == Fetch)
		{
			lodOrBias = in[i];
			i++;
		}
		else if(instruction.samplerMethod == Grad)
		{
			for(uint32_t j = 0; j < instruction.grad; j++, i++)
			{
				dsx[j] = in[i];
			}

			for(uint32_t j = 0; j < instruction.grad; j++, i++)
			{
				dsy[j] = in[i];
			}
		}

		for(uint32_t j = 0; j < instruction.offset; j++, i++)
		{
			offset[j] = As<SIMD::Int>(in[i]);
		}

		if(instruction.sample)
		{
			sampleId = As<SIMD::Int>(in[i]);
		}

		SamplerCore s(constants, samplerState);

		// For explicit-lod instructions the LOD can be different per SIMD lane. SamplerCore currently assumes
		// a single LOD per four elements, so we sample the image again for each LOD separately.
		// TODO(b/133868964) Pass down 4 component lodOrBias, dsx, and dsy to sampleTexture
		if(samplerFunction.method == Lod || samplerFunction.method == Grad ||
		   samplerFunction.method == Bias || samplerFunction.method == Fetch)
		{
			// Only perform per-lane sampling if LOD diverges or we're doing Grad sampling.
			Bool perLaneSampling = samplerFunction.method == Grad || lodOrBias.x != lodOrBias.y ||
			                       lodOrBias.x != lodOrBias.z || lodOrBias.x != lodOrBias.w;
			auto lod = Pointer<Float>(&lodOrBias);
			Int i = 0;
			Do
			{
				SIMD::Float dPdx;
				SIMD::Float dPdy;
				dPdx.x = Pointer<Float>(&dsx.x)[i];
				dPdx.y = Pointer<Float>(&dsx.y)[i];
				dPdx.z = Pointer<Float>(&dsx.z)[i];

				dPdy.x = Pointer<Float>(&dsy.x)[i];
				dPdy.y = Pointer<Float>(&dsy.y)[i];
				dPdy.z = Pointer<Float>(&dsy.z)[i];

				Vector4f sample = s.sampleTexture(texture, uvwa, dRef, lod[i], dPdx, dPdy, offset, sampleId, samplerFunction);

				If(perLaneSampling)
				{
					Pointer<Float> rgba = out;
					rgba[0 * SIMD::Width + i] = Pointer<Float>(&sample.x)[i];
					rgba[1 * SIMD::Width + i] = Pointer<Float>(&sample.y)[i];
					rgba[2 * SIMD::Width + i] = Pointer<Float>(&sample.z)[i];
					rgba[3 * SIMD::Width + i] = Pointer<Float>(&sample.w)[i];
					i++;
				}
				Else
				{
					Pointer<SIMD::Float> rgba = out;
					rgba[0] = sample.x;
					rgba[1] = sample.y;
					rgba[2] = sample.z;
					rgba[3] = sample.w;
					i = SIMD::Width;
				}
			}
			Until(i == SIMD::Width);
		}
		else
		{
			Vector4f sample = s.sampleTexture(texture, uvwa, dRef, lodOrBias.x, (dsx.x), (dsy.x), offset, sampleId, samplerFunction);

			Pointer<SIMD::Float> rgba = out;
			rgba[0] = sample.x;
			rgba[1] = sample.y;
			rgba[2] = sample.z;
			rgba[3] = sample.w;
		}
	}

	return function("sampler");
}

sw::FilterType SpirvShader::convertFilterMode(const vk::SamplerState *samplerState, VkImageViewType imageViewType, SamplerMethod samplerMethod)
{
	if(samplerMethod == Gather)
	{
		return FILTER_GATHER;
	}

	if(samplerMethod == Fetch)
	{
		return FILTER_POINT;
	}

	if(samplerState->anisotropyEnable != VK_FALSE)
	{
		if(imageViewType == VK_IMAGE_VIEW_TYPE_2D || imageViewType == VK_IMAGE_VIEW_TYPE_2D_ARRAY)
		{
			if(samplerMethod != Lod)  // TODO(b/162926129): Support anisotropic filtering with explicit LOD.
			{
				return FILTER_ANISOTROPIC;
			}
		}
	}

	switch(samplerState->magFilter)
	{
	case VK_FILTER_NEAREST:
		switch(samplerState->minFilter)
		{
		case VK_FILTER_NEAREST: return FILTER_POINT;
		case VK_FILTER_LINEAR: return FILTER_MIN_LINEAR_MAG_POINT;
		default:
			UNSUPPORTED("minFilter %d", samplerState->minFilter);
			return FILTER_POINT;
		}
		break;
	case VK_FILTER_LINEAR:
		switch(samplerState->minFilter)
		{
		case VK_FILTER_NEAREST: return FILTER_MIN_POINT_MAG_LINEAR;
		case VK_FILTER_LINEAR: return FILTER_LINEAR;
		default:
			UNSUPPORTED("minFilter %d", samplerState->minFilter);
			return FILTER_POINT;
		}
		break;
	default:
		break;
	}

	UNSUPPORTED("magFilter %d", samplerState->magFilter);
	return FILTER_POINT;
}

sw::MipmapType SpirvShader::convertMipmapMode(const vk::SamplerState *samplerState)
{
	if(!samplerState)
	{
		return MIPMAP_POINT;  // Samplerless operations (OpImageFetch) can take an integer Lod operand./////////////////////////
	}

	if(samplerState->ycbcrModel != VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY)
	{
		// TODO(b/151263485): Check image view level count instead.
		return MIPMAP_NONE;
	}

	switch(samplerState->mipmapMode)
	{
	case VK_SAMPLER_MIPMAP_MODE_NEAREST: return MIPMAP_POINT;
	case VK_SAMPLER_MIPMAP_MODE_LINEAR: return MIPMAP_LINEAR;
	default:
		UNSUPPORTED("mipmapMode %d", samplerState->mipmapMode);
		return MIPMAP_POINT;
	}
}

sw::AddressingMode SpirvShader::convertAddressingMode(int coordinateIndex, const vk::SamplerState *samplerState, VkImageViewType imageViewType)
{
	switch(imageViewType)
	{
	case VK_IMAGE_VIEW_TYPE_1D:
	case VK_IMAGE_VIEW_TYPE_1D_ARRAY:
		if(coordinateIndex >= 1)
		{
			return ADDRESSING_UNUSED;
		}
		break;
	case VK_IMAGE_VIEW_TYPE_2D:
	case VK_IMAGE_VIEW_TYPE_2D_ARRAY:
		if(coordinateIndex == 2)
		{
			return ADDRESSING_UNUSED;
		}
		break;

	case VK_IMAGE_VIEW_TYPE_3D:
		break;

	case VK_IMAGE_VIEW_TYPE_CUBE:
	case VK_IMAGE_VIEW_TYPE_CUBE_ARRAY:
		if(coordinateIndex <= 1)  // Cube faces themselves are addressed as 2D images.
		{
			// Vulkan 1.1 spec:
			// "Cube images ignore the wrap modes specified in the sampler. Instead, if VK_FILTER_NEAREST is used within a mip level then
			//  VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE is used, and if VK_FILTER_LINEAR is used within a mip level then sampling at the edges
			//  is performed as described earlier in the Cube map edge handling section."
			// This corresponds with our 'SEAMLESS' addressing mode.
			return ADDRESSING_SEAMLESS;
		}
		else  // coordinateIndex == 2
		{
			// The cube face is an index into 2D array layers.
			return ADDRESSING_CUBEFACE;
		}
		break;

	default:
		UNSUPPORTED("imageViewType %d", imageViewType);
		return ADDRESSING_WRAP;
	}

	if(!samplerState)
	{
		// OpImageFetch does not take a sampler descriptor, but still needs a valid
		// addressing mode that prevents out-of-bounds accesses:
		// "The value returned by a read of an invalid texel is undefined, unless that
		//  read operation is from a buffer resource and the robustBufferAccess feature
		//  is enabled. In that case, an invalid texel is replaced as described by the
		//  robustBufferAccess feature." - Vulkan 1.1

		// VK_EXT_image_robustness requires nullifying out-of-bounds accesses.
		// ADDRESSING_BORDER causes texel replacement to be performed.
		// TODO(b/162327166): Only perform bounds checks when VK_EXT_image_robustness is enabled.
		return ADDRESSING_BORDER;
	}

	VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	switch(coordinateIndex)
	{
	case 0: addressMode = samplerState->addressModeU; break;
	case 1: addressMode = samplerState->addressModeV; break;
	case 2: addressMode = samplerState->addressModeW; break;
	default: UNSUPPORTED("coordinateIndex: %d", coordinateIndex);
	}

	switch(addressMode)
	{
	case VK_SAMPLER_ADDRESS_MODE_REPEAT: return ADDRESSING_WRAP;
	case VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT: return ADDRESSING_MIRROR;
	case VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE: return ADDRESSING_CLAMP;
	case VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER: return ADDRESSING_BORDER;
	case VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE: return ADDRESSING_MIRRORONCE;
	default:
		UNSUPPORTED("addressMode %d", addressMode);
		return ADDRESSING_WRAP;
	}
}

}  // namespace sw
