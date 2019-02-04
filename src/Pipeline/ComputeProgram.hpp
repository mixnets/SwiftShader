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

#ifndef sw_ComputeProgram_hpp
#define sw_ComputeProgram_hpp

#include "SpirvShader.hpp"

#include "Reactor/Reactor.hpp"

#include <functional>

namespace sw
{
	using namespace rr;

	class ComputeProgram : public Function<Void(Pointer<Byte>)>
	{
	public:
		ComputeProgram(SpirvShader const *spirvShader, VkPipelineLayout pipelineLayout);

		virtual ~ComputeProgram();

	    void generate();

		void run(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ,
			uint32_t descriptorSetCount, VkDescriptorSet* descriptorSets);

	protected:
		void emit();

		void setInputBuiltin(spv::BuiltIn id, std::function<void(const SpirvShader::BuiltinMapping& builtin, Array<Float4>& value)> cb);

		Pointer<Byte> data; // argument 0

		struct Data
		{
			VkDescriptorSet descriptorSets[vk::MAX_BOUND_DESCRIPTOR_SETS];
			uint4 numWorkgroups;
			uint4 workgroupID;
		};

		SpirvRoutine routine;
		SpirvShader const * const shader;
		VkPipelineLayout const pipelineLayout;
	};
}

#endif   // sw_ComputeProgram_hpp
