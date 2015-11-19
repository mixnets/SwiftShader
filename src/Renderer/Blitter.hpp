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
		struct BlitState
		{
			bool operator==(const BlitState &state) const
			{
				return memcmp(this, &state, sizeof(BlitState)) == 0;
			}

			Format sourceFormat;
			Format destFormat;
			BlitterOptions options;
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

		void clear(void* pixel, sw::Format format, Surface *dest, const SliceRect &dRect, const BlitterOptions& options);
		void blit(Surface *source, const SliceRect &sRect, Surface *dest, const SliceRect &dRect, const BlitterOptions& options);
		void blit3D(Surface *source, Surface *dest);

	private:
		bool read(Float4 &color, Pointer<Byte> element, Format format);
		bool write(Float4 &color, Pointer<Byte> element, Format format, const BlitterOptions& options);
		bool read(Int4 &color, Pointer<Byte> element, Format format);
		bool write(Int4 &color, Pointer<Byte> element, Format format, const BlitterOptions& options);
		static bool GetScale(float4& scale, Format format);
		static bool ApplyScaleAndClamp(Float4& value, const BlitState& state);
		bool blitReactor(Surface *source, const SliceRect &sRect, Surface *dest, const SliceRect &dRect, const BlitterOptions& options);
		Routine *generate(BlitState &state);

		RoutineCache<BlitState> *blitCache;
		BackoffLock criticalSection;
	};

	extern Blitter blitter;
}

#endif   // sw_Blitter_hpp
