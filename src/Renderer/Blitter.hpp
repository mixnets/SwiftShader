// SwiftShader Software Renderer
//
// Copyright(c) 2005-2012 TransGaming Inc.
//
// All rights reserved. No part of this software may be copied, distributed, transmitted,
// transcribed, stored in a retrieval system, translated into any human or computer
// language by any means, or disclosed to third parties without the explicit written
// agreement of TransGaming Inc. Without such an agreement, no rights or licenses, express
// or implied, including but not limited to any patent rights, are granted to you.
//

#ifndef sw_Blitter_hpp
#define sw_Blitter_hpp

#include "Surface.hpp"
#include "RoutineCache.hpp"
#include "Reactor/Nucleus.hpp"

#include <string.h>

namespace sw
{
	class Blitter
	{
		enum Options : unsigned char
		{
			USE_NONE = 0x00,
			USE_R = 0x01,
			USE_G = 0x02,
			USE_B = 0x04,
			USE_A = 0x08,
			USE_RGBA = USE_R | USE_G | USE_B | USE_A,
			USE_FILTER = 0x10,
			USE_CONST_SRC = 0x20
		};

		struct BlitState
		{
			bool operator==(const BlitState &state) const
			{
				return memcmp(this, &state, sizeof(BlitState)) == 0;
			}

			Format sourceFormat;
			Format destFormat;
			Blitter::Options options;
		};

		struct BlitData
		{
			void *source;
			void *dest;
			int sPitchB;
			int dPitchB;

			float x0;
			float y0;
			float w;
			float h;

			int y0d;
			int y1d;
			int x0d;
			int x1d;

			int sWidth;
			int sHeight;
		};

	public:
		Blitter();

		virtual ~Blitter();

		void clear(void* pixel, sw::Format format, Surface *dest, const SliceRect &dRect, unsigned int rgbaMask);
		void blit(Surface *source, const SliceRect &sRect, Surface *dest, const SliceRect &dRect, bool filter);
		void blit3D(Surface *source, Surface *dest);

	private:
		bool read(Float4 &color, Pointer<Byte> element, Format format);
		bool write(Float4 &color, Pointer<Byte> element, Format format, const Blitter::Options& options);
		bool read(Int4 &color, Pointer<Byte> element, Format format);
		bool write(Int4 &color, Pointer<Byte> element, Format format, const Blitter::Options& options);
		static bool GetScale(float4& scale, Format format);
		static bool ApplyScaleAndClamp(Float4& value, const BlitState& state);
		void blit(Surface *source, const SliceRect &sRect, Surface *dest, const SliceRect &dRect, const Blitter::Options& options);
		bool blitReactor(Surface *source, const SliceRect &sRect, Surface *dest, const SliceRect &dRect, const Blitter::Options& options);
		Routine *generate(BlitState &state);

		RoutineCache<BlitState> *blitCache;
		BackoffLock criticalSection;
	};

	extern Blitter blitter;
}

#endif   // sw_Blitter_hpp
