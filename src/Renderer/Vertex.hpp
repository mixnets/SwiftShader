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
		Fog = T0 + TEXTURE_STAGES,    // x component
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
