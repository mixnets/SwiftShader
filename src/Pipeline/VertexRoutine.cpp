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

#include <Device/Vertex.hpp>
#include "VertexRoutine.hpp"

#include "Constants.hpp"
#include "Device/Vertex.hpp"
#include "Device/Renderer.hpp"
#include "System/Half.hpp"
#include "Vulkan/VkDebug.hpp"
#include "SpirvShader.hpp"

namespace sw
{
	VertexRoutine::VertexRoutine(
			const VertexProcessor::State &state,
			vk::PipelineLayout const *pipelineLayout,
			SpirvShader const *spirvShader)
		: routine(pipelineLayout),
		  state(state),
		  spirvShader(spirvShader)
	{
	  	spirvShader->emitProlog(&routine);
	}

	VertexRoutine::~VertexRoutine()
	{
	}

	void VertexRoutine::generate()
	{
		Pointer<Byte> cache = task + OFFSET(VertexTask,vertexCache);
		Pointer<Byte> vertexCache = cache + OFFSET(VertexCache,vertex);
		Pointer<Int> tagCache = cache + OFFSET(VertexCache,tag);

		Int vertexCount = *Pointer<Int>(task + OFFSET(VertexTask,vertexCount));

		constants = *Pointer<Pointer<Byte>>(data + OFFSET(DrawData,constants));

		struct Pending
		{
			SIMD::Int inputIndices = SIMD::Int(0);
			SIMD::Int vertexOffsets = SIMD::Int(0);
			SIMD::Int activeLaneMask = SIMD::Int(0);
			UInt count = 0;
		} pending;

		// flush() computes the vertices contained in pending, writing the
		// result to the output VertexRoutinePrototype::vertex buffer, and
		// making a copy into the cache. flush() resets pending before
		// returning.
		auto flush = [&]()
		{
			readInput(pending.inputIndices, pending.activeLaneMask);
			program(pending.inputIndices, pending.activeLaneMask);
			computeClipFlags();
			writeVertex(vertex, pending.vertexOffsets, pending.activeLaneMask);

			auto tagIndices = pending.inputIndices & SIMD::Int(VertexCache::TagMask);
			auto tagOffsets = tagIndices * SIMD::Int(sizeof(VertexCache::tag[0]));
			rr::Scatter(tagCache, pending.inputIndices, tagOffsets, pending.activeLaneMask, sizeof(VertexCache::tag[0]));
			auto cacheOffsets = tagIndices * SIMD::Int(sizeof(Vertex));
			copyVertex(
				vertexCache, cacheOffsets,
				vertex, pending.vertexOffsets,
				pending.activeLaneMask);

			pending = {};
		};

		Int vertexOffset = 0;

		Do
		{
			Int index = *Pointer<UInt>(batch);
			Int cacheIndex = index & VertexCache::TagMask;

			If(tagCache[cacheIndex] == index)
			{
				// Cache hit
				copyVertex(vertex + vertexOffset, vertexCache + cacheIndex * sizeof(Vertex));
			}
			Else
			{
				// Cache miss. Add to pending.
				pending.inputIndices = Insert(pending.inputIndices.xxyz, index, 0);
				pending.vertexOffsets = Insert(pending.vertexOffsets.xxyz, vertexOffset, 0);
				pending.activeLaneMask = Insert(pending.activeLaneMask.xxyz, Int(-1), 0);
				pending.count++;
			}

			vertexOffset += sizeof(Vertex);
			batch += sizeof(unsigned int);
			vertexCount--;

			If(pending.count == SIMD::Width || (pending.count > 0 && vertexCount == 0))
			{
				flush();
			}
		}
		Until(vertexCount == 0)

		Return();
	}

	void VertexRoutine::readInput(RValue<SIMD::Int> indices, RValue<SIMD::Int> activeLaneMask)
	{
		for(int i = 0; i < MAX_INTERFACE_COMPONENTS; i += 4)
		{
			if (spirvShader->inputs[i].Type != SpirvShader::ATTRIBTYPE_UNUSED ||
				spirvShader->inputs[i + 1].Type != SpirvShader::ATTRIBTYPE_UNUSED ||
				spirvShader->inputs[i + 2].Type != SpirvShader::ATTRIBTYPE_UNUSED ||
				spirvShader->inputs[i + 3].Type != SpirvShader::ATTRIBTYPE_UNUSED)
			{

				Pointer<Byte> input = *Pointer<Pointer<Byte>>(data + OFFSET(DrawData, input) + sizeof(void *) * (i/4));
				Int stride = *Pointer<Int>(data + OFFSET(DrawData, stride) + sizeof(unsigned int) * (i/4));

				auto value = readStream(input, indices * SIMD::Int(stride), activeLaneMask, state.input[i/4]);
				routine.inputs[i] = value.x;
				routine.inputs[i+1] = value.y;
				routine.inputs[i+2] = value.z;
				routine.inputs[i+3] = value.w;
			}
		}
	}

	void VertexRoutine::computeClipFlags()
	{
		auto it = spirvShader->outputBuiltins.find(spv::BuiltInPosition);
		assert(it != spirvShader->outputBuiltins.end());
		assert(it->second.SizeInComponents == 4);
		auto &pos = routine.getVariable(it->second.Id);
		auto posX = pos[it->second.FirstComponent];
		auto posY = pos[it->second.FirstComponent + 1];
		auto posZ = pos[it->second.FirstComponent + 2];
		auto posW = pos[it->second.FirstComponent + 3];

		Int4 maxX = CmpLT(posW, posX);
		Int4 maxY = CmpLT(posW, posY);
		Int4 maxZ = CmpLT(posW, posZ);
		Int4 minX = CmpNLE(-posW, posX);
		Int4 minY = CmpNLE(-posW, posY);
		Int4 minZ = CmpNLE(Float4(0.0f), posZ);

		clipFlags = *Pointer<Int>(constants + OFFSET(Constants,maxX) + SignMask(maxX) * 4);   // FIXME: Array indexing
		clipFlags |= *Pointer<Int>(constants + OFFSET(Constants,maxY) + SignMask(maxY) * 4);
		clipFlags |= *Pointer<Int>(constants + OFFSET(Constants,maxZ) + SignMask(maxZ) * 4);
		clipFlags |= *Pointer<Int>(constants + OFFSET(Constants,minX) + SignMask(minX) * 4);
		clipFlags |= *Pointer<Int>(constants + OFFSET(Constants,minY) + SignMask(minY) * 4);
		clipFlags |= *Pointer<Int>(constants + OFFSET(Constants,minZ) + SignMask(minZ) * 4);

		Int4 finiteX = CmpLE(Abs(posX), *Pointer<Float4>(constants + OFFSET(Constants,maxPos)));
		Int4 finiteY = CmpLE(Abs(posY), *Pointer<Float4>(constants + OFFSET(Constants,maxPos)));
		Int4 finiteZ = CmpLE(Abs(posZ), *Pointer<Float4>(constants + OFFSET(Constants,maxPos)));

		Int4 finiteXYZ = finiteX & finiteY & finiteZ;
		clipFlags |= *Pointer<Int>(constants + OFFSET(Constants,fini) + SignMask(finiteXYZ) * 4);
	}

	Vector4f VertexRoutine::readStream(RValue<Pointer<Byte>> buffer, RValue<SIMD::Int> offsets, RValue<SIMD::Int> activeLaneMask, const Stream &stream)
	{
		Vector4f v;

		Pointer<Byte> source0 = buffer + Extract(offsets, 0);
		Pointer<Byte> source1 = buffer + Extract(offsets, 1);
		Pointer<Byte> source2 = buffer + Extract(offsets, 2);
		Pointer<Byte> source3 = buffer + Extract(offsets, 3);

		bool isNativeFloatAttrib = (stream.attribType == SpirvShader::ATTRIBTYPE_FLOAT) || stream.normalized;

		switch(stream.type)
		{
		case STREAMTYPE_FLOAT:
			for (int i = 0; i < stream.count; i++)
			{
				switch(stream.attribType)
				{
				case SpirvShader::ATTRIBTYPE_FLOAT:
					v[i] = rr::Gather(Pointer<Float>(buffer + i*4), offsets, activeLaneMask, 4);
					break;
				case SpirvShader::ATTRIBTYPE_INT:
					v[i] = As<SIMD::Float>(rr::Gather(Pointer<Int>(buffer + i*4), offsets, activeLaneMask, 4));
					break;
				case SpirvShader::ATTRIBTYPE_UINT:
					v[i] = As<SIMD::Float>(rr::Gather(Pointer<UInt>(buffer + i*4), offsets, activeLaneMask, 4));
					break;
				default:
					UNREACHABLE("stream.attribType: %d", int(stream.attribType));
					break;
				}
			}
			break;
		case STREAMTYPE_BYTE:
			for (int i = 0; i < stream.count; i++)
			{
				if(isNativeFloatAttrib) // Stream: UByte, Shader attrib: Float
				{
					v[i] = Float4(rr::Gather(Pointer<Byte>(buffer + i), offsets, activeLaneMask, 1));

					if(stream.normalized)
					{
						v[i] *= *Pointer<Float4>(constants + OFFSET(Constants,unscaleByte));
					}
				}
				else // Stream: UByte, Shader attrib: Int / UInt
				{
					v[i] = As<Float4>(Int4(rr::Gather(Pointer<Byte>(buffer + i), offsets, activeLaneMask, 1)));
				}
			}
			break;
		case STREAMTYPE_SBYTE:
			for (int i = 0; i < stream.count; i++)
			{
				if(isNativeFloatAttrib) // Stream: SByte, Shader attrib: Float
				{
					v[i] = Float4(rr::Gather(Pointer<SByte>(buffer + i), offsets, activeLaneMask, 1));

					if(stream.normalized)
					{
						v[i] *= *Pointer<Float4>(constants + OFFSET(Constants,unscaleSByte));
					}
				}
				else // Stream: SByte, Shader attrib: Int / UInt
				{
					v[i] = As<Float4>(Int4(rr::Gather(Pointer<SByte>(buffer + i), offsets, activeLaneMask, 1)));
				}
			}
			break;
		case STREAMTYPE_COLOR:
			for (int i = 0; i < stream.count; i++)
			{
				static const int rgSwap[] = {2, 1, 0, 3}; // Swap red and blue
				v[i] = Float4(rr::Gather(Pointer<Byte>(buffer + rgSwap[i]), offsets, activeLaneMask, 1)) *
						*Pointer<Float4>(constants + OFFSET(Constants,unscaleByte));
			}
			break;
		case STREAMTYPE_SHORT:
			for (int i = 0; i < stream.count; i++)
			{
				if(isNativeFloatAttrib) // Stream: Int, Shader attrib: Float
				{
					v[i] = Float4(rr::Gather(Pointer<Short>(buffer + i*2), offsets, activeLaneMask, 4));

					if(stream.normalized)
					{
						v[i] *= *Pointer<Float4>(constants + OFFSET(Constants,unscaleShort));
					}
				}
				else // Stream: Short, Shader attrib: Int/UInt, no type conversion
				{
					v[i] = As<Float4>(Int4(rr::Gather(Pointer<Short>(buffer + i*2), offsets, activeLaneMask, 2)));
				}
			}
			break;
		case STREAMTYPE_USHORT:
			for (int i = 0; i < stream.count; i++)
			{
				if(isNativeFloatAttrib) // Stream: Int, Shader attrib: Float
				{
					v[i] = Float4(rr::Gather(Pointer<UShort>(buffer + i*2), offsets, activeLaneMask, 4));

					if(stream.normalized)
					{
						v[i] *= *Pointer<Float4>(constants + OFFSET(Constants,unscaleUShort));
					}
				}
				else // Stream: UShort, Shader attrib: Int/UInt, no type conversion
				{
					v[i] = As<Float4>(Int4(rr::Gather(Pointer<UShort>(buffer + i*2), offsets, activeLaneMask, 2)));
				}
			}
			break;
		case STREAMTYPE_INT:
			for (int i = 0; i < stream.count; i++)
			{
				if(isNativeFloatAttrib) // Stream: Int, Shader attrib: Float
				{
					v[i] = Float4(rr::Gather(Pointer<Int>(buffer + i*4), offsets, activeLaneMask, 4));

					if(stream.normalized)
					{
						v[i] *= *Pointer<Float4>(constants + OFFSET(Constants, unscaleInt));
					}
				}
				else // Stream: Int, Shader attrib: Int/UInt, no type conversion
				{
					v[i] = rr::Gather(Pointer<Float>(buffer + i*4), offsets, activeLaneMask, 4);
				}
			}
			break;
		case STREAMTYPE_UINT:
			for (int i = 0; i < stream.count; i++)
			{
				if(isNativeFloatAttrib) // Stream: UInt, Shader attrib: Float
				{
					v[i] = Float4(rr::Gather(Pointer<UInt>(buffer + i*4), offsets, activeLaneMask, 4));

					if(stream.normalized)
					{
						v[i] *= *Pointer<Float4>(constants + OFFSET(Constants, unscaleUInt));
					}
				}
				else // Stream: UInt, Shader attrib: Int/UInt, no type conversion
				{
					v[i] = rr::Gather(Pointer<Float>(buffer + i*4), offsets, activeLaneMask, 4);
				}
			}
			break;
		case STREAMTYPE_HALF:
			for (int i = 0; i < stream.count; i++)
			{
				auto s = rr::Gather(Pointer<UShort>(buffer + i*2), offsets, activeLaneMask, 2);
				v[i] = rr::Gather(Pointer<Float>(constants + OFFSET(Constants,half2float)), SIMD::Int(s) * SIMD::Int(4), activeLaneMask, 4);
			}
			break;
		case STREAMTYPE_2_10_10_10_INT:
			{
				Int4 src = rr::Gather(Pointer<Int>(buffer), offsets, activeLaneMask, 4);

				v.x = Float4((src << 22) >> 22);
				v.y = Float4((src << 12) >> 22);
				v.z = Float4((src << 02) >> 22);
				v.w = Float4(src >> 30);

				if(stream.normalized)
				{
					v.x = Max(v.x * Float4(1.0f / 0x1FF), Float4(-1.0f));
					v.y = Max(v.y * Float4(1.0f / 0x1FF), Float4(-1.0f));
					v.z = Max(v.z * Float4(1.0f / 0x1FF), Float4(-1.0f));
					v.w = Max(v.w, Float4(-1.0f));
				}
			}
			break;
		case STREAMTYPE_2_10_10_10_UINT:
			{
				Int4 src = rr::Gather(Pointer<Int>(buffer), offsets, activeLaneMask, 4);

				v.x = Float4(src & Int4(0x3FF));
				v.y = Float4((src >> 10) & Int4(0x3FF));
				v.z = Float4((src >> 20) & Int4(0x3FF));
				v.w = Float4((src >> 30) & Int4(0x3));

				if(stream.normalized)
				{
					v.x *= Float4(1.0f / 0x3FF);
					v.y *= Float4(1.0f / 0x3FF);
					v.z *= Float4(1.0f / 0x3FF);
					v.w *= Float4(1.0f / 0x3);
				}
			}
			break;
		default:
			UNIMPLEMENTED("stream.type: %d", int(stream.type));
		}

		if(stream.count < 1) v.x = Float4(0.0f);
		if(stream.count < 2) v.y = Float4(0.0f);
		if(stream.count < 3) v.z = Float4(0.0f);
		if(stream.count < 4) v.w = isNativeFloatAttrib ? As<Float4>(Float4(1.0f)) : As<Float4>(Int4(1));

		return v;
	}

	void VertexRoutine::writeVertex(RValue<Pointer<Byte>> base, RValue<SIMD::Int> offsets, RValue<SIMD::Int> activeLaneMask)
	{
		for (int i = 0; i < MAX_INTERFACE_COMPONENTS; i++)
		{
			if (spirvShader->outputs[i].Type != SpirvShader::ATTRIBTYPE_UNUSED)
			{
				rr::Scatter(Pointer<Float>(base + OFFSET(Vertex,v[i])), routine.outputs[i], offsets, activeLaneMask, 16);
			}
		}

		auto cf = (SIMD::Int(clipFlags) >> SIMD::Int(0, 8, 16, 24)) & SIMD::Int(0x0000000FF);
		rr::Scatter(Pointer<Int>(base + OFFSET(Vertex,clipFlags)), cf, offsets, activeLaneMask, 4);

		// Viewport transform
		auto it = spirvShader->outputBuiltins.find(spv::BuiltInPosition);
		assert(it != spirvShader->outputBuiltins.end());
		assert(it->second.SizeInComponents == 4);
		auto &pos = routine.getVariable(it->second.Id);
		Float4 posX = pos[it->second.FirstComponent];
		Float4 posY = pos[it->second.FirstComponent + 1];
		Float4 posZ = pos[it->second.FirstComponent + 2];
		Float4 posW = pos[it->second.FirstComponent + 3];

		// Write the builtin pos into the vertex; it's not going to be consumed by the FS, but may need to reproject if we have to clip.
		rr::Scatter(Pointer<Float>(base + OFFSET(Vertex,builtins.position.x)), posX, offsets, activeLaneMask, 4);
		rr::Scatter(Pointer<Float>(base + OFFSET(Vertex,builtins.position.y)), posY, offsets, activeLaneMask, 4);
		rr::Scatter(Pointer<Float>(base + OFFSET(Vertex,builtins.position.z)), posZ, offsets, activeLaneMask, 4);
		rr::Scatter(Pointer<Float>(base + OFFSET(Vertex,builtins.position.w)), posW, offsets, activeLaneMask, 4);

		Float4 w = As<Float4>(As<Int4>(posW) | (As<Int4>(CmpEQ(posW, Float4(0.0f))) & As<Int4>(Float4(1.0f))));
		Float4 rhw = Float4(1.0f) / w;

		auto projX = As<Float4>(RoundInt(*Pointer<Float4>(data + OFFSET(DrawData,X0x16)) + posX * rhw * *Pointer<Float4>(data + OFFSET(DrawData,Wx16))));
		auto projY = As<Float4>(RoundInt(*Pointer<Float4>(data + OFFSET(DrawData,Y0x16)) + posY * rhw * *Pointer<Float4>(data + OFFSET(DrawData,Hx16))));
		auto projZ = posZ * rhw;
		auto projW = rhw;

		rr::Scatter(Pointer<Float>(base + OFFSET(Vertex,projected.x)), projX, offsets, activeLaneMask, 4);
		rr::Scatter(Pointer<Float>(base + OFFSET(Vertex,projected.y)), projY, offsets, activeLaneMask, 4);
		rr::Scatter(Pointer<Float>(base + OFFSET(Vertex,projected.z)), projZ, offsets, activeLaneMask, 4);
		rr::Scatter(Pointer<Float>(base + OFFSET(Vertex,projected.w)), projW, offsets, activeLaneMask, 4);

		it = spirvShader->outputBuiltins.find(spv::BuiltInPointSize);
		if (it != spirvShader->outputBuiltins.end())
		{
			assert(it->second.SizeInComponents == 1);
			auto psize = routine.getVariable(it->second.Id)[it->second.FirstComponent];
			rr::Scatter(Pointer<Float>(base + OFFSET(Vertex,builtins.pointSize)), psize, offsets, activeLaneMask, 4);
		}
	}

	template <typename COPY16>
	void VertexRoutine::copyVertex(COPY16 copy16)
	{
		for(int i = 0; i < MAX_INTERFACE_COMPONENTS; i++)
		{
			if(spirvShader->outputs[i].Type != SpirvShader::ATTRIBTYPE_UNUSED)
			{
				copy16(OFFSET(Vertex, v[i]));
			}
		}

		copy16(OFFSET(Vertex, projected.x));
		copy16(OFFSET(Vertex, projected.y));
		copy16(OFFSET(Vertex, projected.z));
		copy16(OFFSET(Vertex, projected.w));

		copy16(OFFSET(Vertex, clipFlags));

		copy16(OFFSET(Vertex, builtins.position.x));
		copy16(OFFSET(Vertex, builtins.position.y));
		copy16(OFFSET(Vertex, builtins.position.z));
		copy16(OFFSET(Vertex, builtins.position.w));

		copy16(OFFSET(Vertex, builtins.pointSize));
	}

	void VertexRoutine::copyVertex(
			RValue<Pointer<Byte>> dstBase,
			RValue<SIMD::Int> dstOffsets,
			RValue<Pointer<Byte>> srcBase,
			RValue<SIMD::Int> srcOffsets,
			RValue<SIMD::Int> activeLaneMask)
	{
		copyVertex([&] (int offset)
		{
			auto v = rr::Gather(Pointer<Int>(srcBase + offset), srcOffsets, activeLaneMask, 4);
			rr::Scatter(Pointer<Int>(dstBase + offset), v, dstOffsets, activeLaneMask, 4);
		});
	}

	void VertexRoutine::copyVertex(RValue<Pointer<Byte>> dst, RValue<Pointer<Byte>> src)
	{
		copyVertex([&] (int offset)
		{
			*Pointer<SIMD::Int>(dst + offset) = *Pointer<SIMD::Int>(src + offset);
		});
	}
}
