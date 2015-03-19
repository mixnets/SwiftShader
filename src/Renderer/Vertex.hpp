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

#ifndef Vertex_hpp
#define Vertex_hpp

#include "Color.hpp"
#include "Common/MetaMacro.hpp"
#include "Common/Types.hpp"
#include "Main/Config.hpp"

namespace sw
{
	enum Out   // Default vertex output semantic
	{
		Pos = 0,
		D0 = 1,   // Diffuse
		D1 = 2,   // Specular
		T0 = 3,
		Fog = T0 + VERTEX_TEXTURE_IMAGE_UNITS,    // x component
		Pts = Fog,   // y component
		Unused,
		MAX_OUTPUT_VARYINGS = Unused
	};

	struct UVWQ
	{
		float u;
		float v;
		float w;
		float q;

		float &operator[](int i)
		{
			return (&u)[i];
		}
	};

	ALIGN(16, struct Vertex
	{
		union
		{
			struct   // Fixed semantics
			{
				union   // Position
				{
					struct
					{
						float x;
						float y;
						float z;
						float w;
					};

					struct
					{
						float4 P;
					};
				};

				float4 C[2];   // Diffuse and specular color

				UVWQ T[8];           // Texture coordinates

				float f;             // Fog
				float pSize;         // Point size
				unsigned char padding0[4];
				unsigned char clipFlags;
				unsigned char padding1[3];
			};

			float4 v[MAX_OUTPUT_VARYINGS];   // Generic components using semantic declaration
		};

		struct   // Projected coordinates
		{
			int X;
			int Y;
			float Z;
			float W;
		};
	});

	META_ASSERT((sizeof(Vertex) & 0x0000000F) == 0);
}

#endif   // Vertex_hpp
