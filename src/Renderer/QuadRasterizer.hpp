// SwiftShader Software Renderer
//
// Copyright(c) 2005-2011 TransGaming Inc.
//
// All rights reserved. No part of this software may be copied, distributed, transmitted,
// transcribed, stored in a retrieval system, translated into any human or computer
// language by any means, or disclosed to third parties without the explicit written
// agreement of TransGaming Inc. Without such an agreement, no rights or licenses, express
// or implied, including but not limited to any patent rights, are granted to you.
//

#ifndef sw_QuadRasterizer_hpp
#define sw_QuadRasterizer_hpp

#include "Rasterizer.hpp"
#include "ShaderCore.hpp"
#include "PixelShader.hpp"

#include "Types.hpp"

namespace sw
{
	class QuadRasterizer : public Rasterizer
	{
	public:
		QuadRasterizer(const PixelProcessor::State &state, const PixelShader *shader);

		virtual ~QuadRasterizer();

	protected:
		struct Registers
		{
			Registers(const PixelShader *shader);

			Pointer<Byte> constants;

			Pointer<Byte> primitive;
			Int cluster;
			Pointer<Byte> data;

			Float4 z[4];
			Float4 w;
			Float4 rhw;

			Float4 Dz[4];
			Float4 Dw;
			Float4 Dv[10][4];
			Float4 Df;

			Vector4s &current;
			Vector4s &diffuse;
			Vector4s &specular;

			Vector4s rs[6];
			Vector4s vs[2];
			Vector4s ts[6];

			RegisterArray<4096> rf;
			RegisterArray<10> vf;

			Vector4f vPos;
			Vector4f vFace;

			Vector4f oC[4];
			Float4 oDepth;

			Vector4f p0;
			Array<Int, 4> aL;

			Array<Int, 4> increment;
			Array<Int, 4> iteration;

			Int loopDepth;
			Int stackIndex;   // FIXME: Inc/decrement callStack
			Array<UInt, 16> callStack;

			Int enableIndex;
			Array<Int4, 1 + 24> enableStack;
			Int4 enableBreak;
			Int4 enableContinue;
			Int4 enableLeave;

			// bem(l) offsets and luminance
			Float4 du;
			Float4 dv;
			Short4 L;

			// texm3x3 temporaries
			Float4 u_;   // FIXME
			Float4 v_;   // FIXME
			Float4 w_;   // FIXME
			Float4 U;   // FIXME
			Float4 V;   // FIXME
			Float4 W;   // FIXME

			UInt occlusion;

#if PERF_PROFILE
			Long cycles[PERF_TIMERS];
#endif
		};

		virtual void quad(Registers &r, Pointer<Byte> cBuffer[4], Pointer<Byte> &zBuffer, Pointer<Byte> &sBuffer, Int cMask[4], Int &x, Int &y) = 0;

		bool interpolateZ() const;
		bool interpolateW() const;
		Float4 interpolate(Float4 &x, Float4 &D, Float4 &rhw, Pointer<Byte> planeEquation, bool flat, bool perspective);

		const PixelShader *const shader;

	private:
		void generate();

		void rasterize(Registers &r, Int &yMin, Int &yMax);
	};
}

#endif   // sw_QuadRasterizer_hpp
