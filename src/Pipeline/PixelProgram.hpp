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

#ifndef sw_PixelProgram_hpp
#define sw_PixelProgram_hpp

#include "PixelRoutine.hpp"
#include "Device/Config.hpp"

namespace sw {

class PixelProgram : public PixelRoutine
{
public:
	PixelProgram(
	    const PixelProcessor::State &state,
	    vk::PipelineLayout const *pipelineLayout,
	    SpirvShader const *spirvShader,
	    const vk::DescriptorSet::Bindings &descriptorSets)
	    : PixelRoutine(state, pipelineLayout, spirvShader, descriptorSets)
	{
	}

	virtual ~PixelProgram() {}

protected:
	virtual void setBuiltins(Int &x, Int &y, Float4 (&z)[MAX_SAMPLES], Float4 &w, Int cMask[MAX_SAMPLES]);
	virtual void applyShader(Int cMask[MAX_SAMPLES], Int sMask[MAX_SAMPLES], Int zMask[MAX_SAMPLES]);
	virtual Bool alphaTest(Int cMask[MAX_SAMPLES]);
	virtual void rasterOperation(Pointer<Byte> cBuffer[RENDERTARGETS], Int &x, Int sMask[MAX_SAMPLES], Int zMask[MAX_SAMPLES], Int cMask[MAX_SAMPLES]);

private:
	// Color outputs
	Vector4f c[RENDERTARGETS];

	// Raster operations
	void clampColor(Vector4f oC[RENDERTARGETS]);

	Int4 maskAny(Int cMask[MAX_SAMPLES]) const;
	Int4 maskAny(Int cMask[MAX_SAMPLES], Int sMask[MAX_SAMPLES], Int zMask[MAX_SAMPLES]) const;
	Float4 linearToSRGB(const Float4 &x);
};

}  // namespace sw

#endif
