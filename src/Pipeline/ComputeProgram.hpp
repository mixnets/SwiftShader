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

namespace sw
{
	using namespace rr;

	class ComputeProgram : public Function<Void(Pointer<Byte>)>
	{
	public:
		ComputeProgram(/*const PixelProcessor::State &state,*/ SpirvShader const *spirvShader);

		virtual ~ComputeProgram();

	    void generate();

		struct Data
		{
			VkDescriptorSet descriptorSets[vk::MAX_BOUND_DESCRIPTOR_SETS];
		};

	protected:
		void setup();

		Pointer<Byte> data; // argument 0

		SpirvRoutine routine;
		SpirvShader const * const spirvShader;
	};
}

#endif   // sw_ComputeProgram_hpp
