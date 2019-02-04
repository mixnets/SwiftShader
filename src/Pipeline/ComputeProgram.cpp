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

#include "ComputeProgram.hpp"

#include "Vulkan/VkDebug.hpp"

namespace sw
{
	ComputeProgram::ComputeProgram(SpirvShader const *spirvShader, VkPipelineLayout pipelineLayout)
		: data(Arg<0>())
		, shader(spirvShader)
		, pipelineLayout(pipelineLayout)

	{
	}

	ComputeProgram::~ComputeProgram()
	{
	}

	void ComputeProgram::generate()
	{
		shader->emitProlog(&routine);
		emit();
		shader->emitEpilog(&routine);
	}

	void ComputeProgram::emit()
	{
		static_assert(sizeof(VkDescriptorSet) == sizeof(void*));
		Pointer<Pointer<Byte>> setsArray = data + OFFSET(Data, descriptorSets[0]);
		For(Int i = 0, i < vk::MAX_BOUND_DESCRIPTOR_SETS, i++)
		{
			routine.descriptorSets[i] = setsArray[i];
		}

		auto &modes = shader->getModes();

		Int4 numWorkgroups = *Pointer<Int4>(data + OFFSET(Data, numWorkgroups));
		Int4 workgroupID = *Pointer<Int4>(data + OFFSET(Data, workgroupID));
		Int4 workgroupSize = Int4(modes.LocalSizeX, modes.LocalSizeY, modes.LocalSizeZ, 0);

		setInputBuiltin(spv::BuiltInNumWorkgroups, [&](const SpirvShader::BuiltinMapping& builtin, Array<Float4>& value)
		{
			for (uint32_t component = 0; component < builtin.SizeInComponents; component++)
			{
				value[builtin.FirstComponent + component] =
					As<Float4>(Int4(Extract(numWorkgroups, component)));
			}
		});

		setInputBuiltin(spv::BuiltInWorkgroupSize, [&](const SpirvShader::BuiltinMapping& builtin, Array<Float4>& value)
		{
			for (uint32_t component = 0; component < builtin.SizeInComponents; component++)
			{
				value[builtin.FirstComponent + component] =
					As<Float4>(Int4(Extract(workgroupSize, component)));
			}
		});

		// Total number of invocations required to execute this workgroup.
		const int numInvocations = modes.LocalSizeX * modes.LocalSizeY * modes.LocalSizeZ;

		enum { XXXX, YYYY, ZZZZ };

		For(Int invocationIndex = 0, invocationIndex < numInvocations, invocationIndex += SpirvShader::NumLanes)
		{
			Int4 localInvocationIndex = Int4(invocationIndex) + Int4(0, 1, 2, 3);

			Array<Int4> localInvocationID(3);
			{
				Int4 idx = localInvocationIndex;
				localInvocationID[ZZZZ] = idx / Int4(modes.LocalSizeX * modes.LocalSizeY);
				idx -= localInvocationID[ZZZZ] * Int4(modes.LocalSizeX * modes.LocalSizeY); // modulo
				localInvocationID[YYYY] = idx / Int4(modes.LocalSizeX);
				idx -= localInvocationID[YYYY] * Int4(modes.LocalSizeX); // modulo
				localInvocationID[XXXX] = idx;
			}

			setInputBuiltin(spv::BuiltInLocalInvocationIndex, [&](const SpirvShader::BuiltinMapping& builtin, Array<Float4>& value)
			{
				ASSERT(builtin.SizeInComponents == 1);
				value[builtin.FirstComponent] = As<Float4>(localInvocationIndex);
			});

			setInputBuiltin(spv::BuiltInLocalInvocationId, [&](const SpirvShader::BuiltinMapping& builtin, Array<Float4>& value)
			{
				for (uint32_t component = 0; component < builtin.SizeInComponents; component++)
				{
					value[builtin.FirstComponent + component] = As<Float4>(localInvocationID[component]);
				}
			});

			setInputBuiltin(spv::BuiltInGlobalInvocationId, [&](const SpirvShader::BuiltinMapping& builtin, Array<Float4>& value)
			{
				for (uint32_t component = 0; component < builtin.SizeInComponents; component++)
				{
					Int4 globalInvocationID =
						Int4(Extract(workgroupID, component)) *
						Int4(Extract(workgroupSize, component)) +
						localInvocationID[component];
					value[builtin.FirstComponent + component] = As<Float4>(globalInvocationID);

					RR_WATCH(component, globalInvocationID);
				}
			});

			// TODO: Disable lanes where (invocationIDs >= numInvocations)
			// Int4 enabledLanes = invocationIDs < Int4(numInvocations);

			// Process numLanes of the workgroup.
			shader->emit(&routine, vk::Cast(pipelineLayout));
		}
	}

	void ComputeProgram::setInputBuiltin(spv::BuiltIn id, std::function<void(const SpirvShader::BuiltinMapping& builtin, Array<Float4>& value)> cb)
	{
		auto it = shader->inputBuiltins.find(id);
		if (it != shader->inputBuiltins.end())
		{
			const auto& builtin = it->second;
			auto &value = routine.getValue(builtin.Id);
			cb(builtin, value);
		}
	}

	void ComputeProgram::run(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ,
		uint32_t descriptorSetCount, VkDescriptorSet* descriptorSets)
	{
		typedef void (FUNC)(void*);
		auto routine = (*this)("ComputeRoutine");
		auto runWorkgroup = reinterpret_cast<FUNC*>(routine->getEntry());

		Data data;
		for (uint32_t i = 0U; i < descriptorSetCount; i++)
		{
			data.descriptorSets[i] = (i < vk::MAX_BOUND_DESCRIPTOR_SETS) ? descriptorSets[i] : nullptr;
		}

		data.numWorkgroups[0] = groupCountX;
		data.numWorkgroups[1] = groupCountY;
		data.numWorkgroups[2] = groupCountZ;
		data.numWorkgroups[3] = 0;

		// TODO: Split across threads.

		for (uint32_t groupZ = 0; groupZ < groupCountZ; groupZ++)
		{
			data.workgroupID[2] = groupZ;
			for (uint32_t groupY = 0; groupY < groupCountY; groupY++)
			{
				data.workgroupID[1] = groupY;
				for (uint32_t groupX = 0; groupX < groupCountX; groupX++)
				{
					data.workgroupID[0] = groupX;
					runWorkgroup(&data);
				}
			}
		}
	}
}
