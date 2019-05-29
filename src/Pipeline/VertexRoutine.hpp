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

#ifndef sw_VertexRoutine_hpp
#define sw_VertexRoutine_hpp

#include "Device/Color.hpp"
#include "Device/VertexProcessor.hpp"
#include "ShaderCore.hpp"
#include "SpirvShader.hpp"

namespace vk
{
	class PipelineLayout;
} // namespace vk

namespace sw
{
	class VertexRoutinePrototype : public Function<Void(Pointer<Byte>, Pointer<Byte>, Pointer<Byte>, Pointer<Byte>)>
	{
	public:
		VertexRoutinePrototype() : vertex(Arg<0>()), batch(Arg<1>()), task(Arg<2>()), data(Arg<3>()) {}
		virtual ~VertexRoutinePrototype() {}

	protected:
		Pointer<Byte> vertex;
		Pointer<Byte> batch;
		Pointer<Byte> task;
		Pointer<Byte> data;
	};

	class VertexRoutine : public VertexRoutinePrototype
	{
	public:
		VertexRoutine(
			const VertexProcessor::State &state,
			vk::PipelineLayout const *pipelineLayout,
			SpirvShader const *spirvShader);
		virtual ~VertexRoutine();

		void generate();

	protected:
		Pointer<Byte> constants;

		Int clipFlags;

		SpirvRoutine routine;

		const VertexProcessor::State &state;
		SpirvShader const * const spirvShader;

	private:
		virtual void program(RValue<SIMD::Int> indices, RValue<SIMD::Int> activeLaneMask) = 0;

		typedef VertexProcessor::State::Input Stream;

		Vector4f readStream(RValue<Pointer<Byte>> buffer, RValue<SIMD::Int> offsets, RValue<SIMD::Int> activeLaneMask, const Stream &stream);
		void readInput(RValue<SIMD::Int> indices, RValue<SIMD::Int> activeLaneMask);
		void computeClipFlags();
		void writeVertex(RValue<Pointer<Byte>> base, RValue<SIMD::Int> offsets, RValue<SIMD::Int> activeLaneMask);

		// Internal helper used for copying vertices.
		// F is a function with the signature: void(int field_offset_in_bytes)
		// F will be called for every 4-byte field of the Vertex structure.
		template <typename F> void visitVertexFields(F);

		// copyVertex copies a single vertex from src to dst.
		void copyVertex(RValue<Pointer<Byte>> dst, RValue<Pointer<Byte>> src);

		// copyVertices copies up to a SIMD::Width number of vertices from
		// [srcBase + srcOffsets] to [dstBase + dstOffsets]. The copy will be
		// masked by activeLaneMask.
		void copyVertices(
			RValue<Pointer<Byte>> dstBase,
			RValue<SIMD::Int> dstOffsets,
			RValue<Pointer<Byte>> srcBase,
			RValue<SIMD::Int> srcOffsets,
			RValue<SIMD::Int> activeLaneMask);
	};
}

#endif   // sw_VertexRoutine_hpp
