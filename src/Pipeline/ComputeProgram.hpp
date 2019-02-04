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

namespace vk
{
	class PipelineLayout;
} // namespace vk

namespace sw
{
	using namespace rr;

	class DescriptorSetsLayout;

	class ComputeProgram : public Function<Void(Pointer<Byte>)>
	{
	public:
		ComputeProgram(SpirvShader const *spirvShader, vk::PipelineLayout const *pipelineLayout);

		virtual ~ComputeProgram();

	    void generate();

		static void run(
			Routine *routine,
			uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ,
			uint32_t numDescriptorSets, void** descriptorSets);

	protected:
		void emit();

		void setInputBuiltin(spv::BuiltIn id, std::function<void(const SpirvShader::BuiltinMapping& builtin, Array<Float4>& value)> cb);

		Pointer<Byte> data; // argument 0

		struct Data
		{
			void** descriptorSets;
			uint4 numWorkgroups;
			uint4 workgroupID;
		};

		SpirvRoutine routine;
		SpirvShader const * const shader;
		vk::PipelineLayout const * const pipelineLayout;
	};
}

#endif   // sw_ComputeProgram_hpp
