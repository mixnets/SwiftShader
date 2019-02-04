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
		, spirvShader(spirvShader)
		, pipelineLayout(pipelineLayout)

	{
	}

	ComputeProgram::~ComputeProgram()
	{
	}

	void ComputeProgram::generate()
	{
		spirvShader->emitProlog(&routine);
		setup();
		spirvShader->emit(&routine, vk::Cast(pipelineLayout));
		spirvShader->emitProlog(&routine);
	}

	void ComputeProgram::setup()
	{
		static_assert(sizeof(VkDescriptorSet) == sizeof(void*));
		Pointer<Pointer<Byte>> setsArray = data + OFFSET(Data, descriptorSets[0]);
		For(Int i = 0, i < vk::MAX_BOUND_DESCRIPTOR_SETS, i++)
		{
			routine.descriptorSets[i] = setsArray[i];
		}
	}
}
