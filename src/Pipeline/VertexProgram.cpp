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

#include "VertexProgram.hpp"

#include "SamplerCore.hpp"
#include "Device/Renderer.hpp"
#include "Device/Vertex.hpp"
#include "System/Half.hpp"
#include "Vulkan/VkDebug.hpp"

#include "Vulkan/VkPipelineLayout.hpp"

namespace sw
{
	VertexProgram::VertexProgram(
			const VertexProcessor::State &state,
			vk::PipelineLayout const *pipelineLayout,
			SpirvShader const *spirvShader,
			const vk::DescriptorSet::Bindings &descriptorSets)
		: VertexRoutine(state, pipelineLayout, spirvShader),
		  descriptorSets(descriptorSets)
	{
		routine.setImmutableInputBuiltins(spirvShader);

		routine.setInputBuiltin(spirvShader, spv::BuiltInViewIndex, [&](const SpirvShader::BuiltinMapping& builtin, Array<SIMD::Float>& value)
		{
			assert(builtin.SizeInComponents == 1);
			value[builtin.FirstComponent] = As<Float4>(Int4((*Pointer<Int>(data + OFFSET(DrawData, viewID)))));
		});

		routine.setInputBuiltin(spirvShader, spv::BuiltInInstanceIndex, [&](const SpirvShader::BuiltinMapping& builtin, Array<SIMD::Float>& value)
		{
			// TODO: we could do better here; we know InstanceIndex is uniform across all lanes
			assert(builtin.SizeInComponents == 1);
			value[builtin.FirstComponent] = As<Float4>(Int4((*Pointer<Int>(data + OFFSET(DrawData, instanceID)))));
		});

		routine.setInputBuiltin(spirvShader, spv::BuiltInSubgroupSize, [&](const SpirvShader::BuiltinMapping& builtin, Array<SIMD::Float>& value)
		{
			ASSERT(builtin.SizeInComponents == 1);
			value[builtin.FirstComponent] = As<SIMD::Float>(SIMD::Int(SIMD::Width));
		});

		routine.descriptorSets = data + OFFSET(DrawData, descriptorSets);
		routine.descriptorDynamicOffsets = data + OFFSET(DrawData, descriptorDynamicOffsets);
		routine.pushConstants = data + OFFSET(DrawData, pushConstants);
		routine.constants = *Pointer<Pointer<Byte>>(data + OFFSET(DrawData, constants));
	}

	VertexProgram::~VertexProgram()
	{
	}

	void VertexProgram::program(Pointer<UInt> &batch, Pointer<UInt>& tagCache)
	{
		Int4 indices;
		indices = Insert(indices, As<Int>(batch[0]), 0);
		indices = Insert(indices, As<Int>(batch[1]), 1);
		indices = Insert(indices, As<Int>(batch[2]), 2);
		indices = Insert(indices, As<Int>(batch[3]), 3);

		auto it = spirvShader->inputBuiltins.find(spv::BuiltInVertexIndex);
		if (it != spirvShader->inputBuiltins.end())
		{
			assert(it->second.SizeInComponents == 1);

			routine.getVariable(it->second.Id)[it->second.FirstComponent] =
					As<Float4>(indices + Int4(*Pointer<Int>(data + OFFSET(DrawData, baseVertex))));
		}

		// Check if any of the indices are already cached
		Int4 cacheIndices = indices & Int4(VertexCache::TAG_MASK);
		cacheIndices = Insert(cacheIndices, As<Int>(tagCache[cacheIndices.x]), 0);
		cacheIndices = Insert(cacheIndices, As<Int>(tagCache[cacheIndices.y]), 1);
		cacheIndices = Insert(cacheIndices, As<Int>(tagCache[cacheIndices.z]), 2);
		cacheIndices = Insert(cacheIndices, As<Int>(tagCache[cacheIndices.w]), 3);
		Int4 storesAndAtomicsMask = CmpNEQ(indices, cacheIndices);

		// Prevent duplicate vertices from using atomics multiple times
		Int4 batchDuplicates = CmpNEQ(indices.xxyy, indices.yzzz); // Compares [0,1], [0,2], [1, 2]
		storesAndAtomicsMask.y = Int(storesAndAtomicsMask.y) & batchDuplicates.x;
		storesAndAtomicsMask.z = Int(storesAndAtomicsMask.z) & batchDuplicates.y & batchDuplicates.z;
		batchDuplicates = CmpNEQ(indices.xyzz, indices.wwww); // Compares [0,3], [1,3], [2, 3]
		storesAndAtomicsMask.w = Int(storesAndAtomicsMask.w) & batchDuplicates.x & batchDuplicates.y & batchDuplicates.z;

		auto activeLaneMask = SIMD::Int(0xFFFFFFFF);
		spirvShader->emit(&routine, activeLaneMask, storesAndAtomicsMask, descriptorSets);

		spirvShader->emitEpilog(&routine);
	}
}
