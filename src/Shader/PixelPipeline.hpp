// SwiftShader Software Renderer
//
// Copyright(c) 2015 Google Inc.
//
// All rights reserved. No part of this software may be copied, distributed, transmitted,
// transcribed, stored in a retrieval system, translated into any human or computer
// language by any means, or disclosed to third parties without the explicit written
// agreement of Google Inc. Without such an agreement, no rights or licenses, express
// or implied, including but not limited to any patent rights, are granted to you.
//

#ifndef sw_PixelPipeline_hpp
#define sw_PixelPipeline_hpp

#include "PixelRoutine.hpp"

namespace sw
{
	class PixelPipeline : public PixelRoutine
	{
	public:
		PixelPipeline(const PixelProcessor::State &state, const PixelShader *shader) : PixelRoutine(state, shader) {}
	};
}

#endif