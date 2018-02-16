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

#include "Image.hpp"

#include "../libEGL/Context.hpp"
#include "../libEGL/Texture.hpp"
#include "../common/debug.h"
#include "Common/Math.hpp"
#include "Common/Thread.hpp"

#include <GLES3/gl3.h>

#include <string.h>

namespace
{
	int getNumBlocks(int w, int h, int blockSizeX, int blockSizeY)
	{
		return ((w + blockSizeX - 1) / blockSizeX) * ((h + blockSizeY - 1) / blockSizeY);
	}

	enum DataType
	{
		Bytes,
		ByteRGB,
		UByteRGB,
		ShortRGB,
		UShortRGB,
		IntRGB,
		UIntRGB,
		FloatRGB,
		HalfFloatRGB,
		RGBA4444,
		RGBA5551,
		R11G11B10F,
		RGB9E5,
		D16,
		D24X8,
		D32,
		D32F,
		X24S8,
		D32FX32,
		X56S8,
		RGBA1010102to8888,
		RGB8to565,
		R32Fto16F,
		RG32Fto16F,
		RGB32Fto16F,
		RGBA32Fto16F
	};

	template<DataType dataType>
	void TransferRow(unsigned char *dest, const unsigned char *source, GLsizei width, GLsizei bytes);

	template<>
	void TransferRow<Bytes>(unsigned char *dest, const unsigned char *source, GLsizei width, GLsizei bytes)
	{
		memcpy(dest, source, width * bytes);
	}

	template<>
	void TransferRow<ByteRGB>(unsigned char *dest, const unsigned char *source, GLsizei width, GLsizei bytes)
	{
		unsigned char *destB = dest;

		for(int x = 0; x < width; x++)
		{
			destB[4 * x + 0] = source[x * 3 + 0];
			destB[4 * x + 1] = source[x * 3 + 1];
			destB[4 * x + 2] = source[x * 3 + 2];
			destB[4 * x + 3] = 0x7F;
		}
	}

	template<>
	void TransferRow<UByteRGB>(unsigned char *dest, const unsigned char *source, GLsizei width, GLsizei bytes)
	{
		unsigned char *destB = dest;

		for(int x = 0; x < width; x++)
		{
			destB[4 * x + 0] = source[x * 3 + 0];
			destB[4 * x + 1] = source[x * 3 + 1];
			destB[4 * x + 2] = source[x * 3 + 2];
			destB[4 * x + 3] = 0xFF;
		}
	}

	template<>
	void TransferRow<ShortRGB>(unsigned char *dest, const unsigned char *source, GLsizei width, GLsizei bytes)
	{
		const unsigned short *sourceS = reinterpret_cast<const unsigned short*>(source);
		unsigned short *destS = reinterpret_cast<unsigned short*>(dest);

		for(int x = 0; x < width; x++)
		{
			destS[4 * x + 0] = sourceS[x * 3 + 0];
			destS[4 * x + 1] = sourceS[x * 3 + 1];
			destS[4 * x + 2] = sourceS[x * 3 + 2];
			destS[4 * x + 3] = 0x7FFF;
		}
	}

	template<>
	void TransferRow<UShortRGB>(unsigned char *dest, const unsigned char *source, GLsizei width, GLsizei bytes)
	{
		const unsigned short *sourceS = reinterpret_cast<const unsigned short*>(source);
		unsigned short *destS = reinterpret_cast<unsigned short*>(dest);

		for(int x = 0; x < width; x++)
		{
			destS[4 * x + 0] = sourceS[x * 3 + 0];
			destS[4 * x + 1] = sourceS[x * 3 + 1];
			destS[4 * x + 2] = sourceS[x * 3 + 2];
			destS[4 * x + 3] = 0xFFFF;
		}
	}

	template<>
	void TransferRow<IntRGB>(unsigned char *dest, const unsigned char *source, GLsizei width, GLsizei bytes)
	{
		const unsigned int *sourceI = reinterpret_cast<const unsigned int*>(source);
		unsigned int *destI = reinterpret_cast<unsigned int*>(dest);

		for(int x = 0; x < width; x++)
		{
			destI[4 * x + 0] = sourceI[x * 3 + 0];
			destI[4 * x + 1] = sourceI[x * 3 + 1];
			destI[4 * x + 2] = sourceI[x * 3 + 2];
			destI[4 * x + 3] = 0x7FFFFFFF;
		}
	}

	template<>
	void TransferRow<UIntRGB>(unsigned char *dest, const unsigned char *source, GLsizei width, GLsizei bytes)
	{
		const unsigned int *sourceI = reinterpret_cast<const unsigned int*>(source);
		unsigned int *destI = reinterpret_cast<unsigned int*>(dest);

		for(int x = 0; x < width; x++)
		{
			destI[4 * x + 0] = sourceI[x * 3 + 0];
			destI[4 * x + 1] = sourceI[x * 3 + 1];
			destI[4 * x + 2] = sourceI[x * 3 + 2];
			destI[4 * x + 3] = 0xFFFFFFFF;
		}
	}

	template<>
	void TransferRow<FloatRGB>(unsigned char *dest, const unsigned char *source, GLsizei width, GLsizei bytes)
	{
		const float *sourceF = reinterpret_cast<const float*>(source);
		float *destF = reinterpret_cast<float*>(dest);

		for(int x = 0; x < width; x++)
		{
			destF[4 * x + 0] = sourceF[x * 3 + 0];
			destF[4 * x + 1] = sourceF[x * 3 + 1];
			destF[4 * x + 2] = sourceF[x * 3 + 2];
			destF[4 * x + 3] = 1.0f;
		}
	}

	template<>
	void TransferRow<HalfFloatRGB>(unsigned char *dest, const unsigned char *source, GLsizei width, GLsizei bytes)
	{
		const unsigned short *sourceH = reinterpret_cast<const unsigned short*>(source);
		unsigned short *destH = reinterpret_cast<unsigned short*>(dest);

		for(int x = 0; x < width; x++)
		{
			destH[4 * x + 0] = sourceH[x * 3 + 0];
			destH[4 * x + 1] = sourceH[x * 3 + 1];
			destH[4 * x + 2] = sourceH[x * 3 + 2];
			destH[4 * x + 3] = 0x3C00;   // SEEEEEMMMMMMMMMM, S = 0, E = 15, M = 0: 16-bit floating-point representation of 1.0
		}
	}

	template<>
	void TransferRow<RGBA4444>(unsigned char *dest, const unsigned char *source, GLsizei width, GLsizei bytes)
	{
		const unsigned short *source4444 = reinterpret_cast<const unsigned short*>(source);
		unsigned char *dest4444 = dest;

		for(int x = 0; x < width; x++)
		{
			unsigned short rgba = source4444[x];
			dest4444[4 * x + 0] = ((rgba & 0xF000) >> 8) | ((rgba & 0xF000) >> 12);
			dest4444[4 * x + 1] = ((rgba & 0x0F00) >> 4) | ((rgba & 0x0F00) >> 8);
			dest4444[4 * x + 2] = ((rgba & 0x00F0) << 0) | ((rgba & 0x00F0) >> 4);
			dest4444[4 * x + 3] = ((rgba & 0x000F) << 4) | ((rgba & 0x000F) >> 0);
		}
	}

	template<>
	void TransferRow<RGBA5551>(unsigned char *dest, const unsigned char *source, GLsizei width, GLsizei bytes)
	{
		const unsigned short *source5551 = reinterpret_cast<const unsigned short*>(source);
		unsigned char *dest8888 = dest;

		for(int x = 0; x < width; x++)
		{
			unsigned short rgba = source5551[x];
			dest8888[4 * x + 0] = ((rgba & 0xF800) >> 8) | ((rgba & 0xF800) >> 13);
			dest8888[4 * x + 1] = ((rgba & 0x07C0) >> 3) | ((rgba & 0x07C0) >> 8);
			dest8888[4 * x + 2] = ((rgba & 0x003E) << 2) | ((rgba & 0x003E) >> 3);
			dest8888[4 * x + 3] = (rgba & 0x0001) ? 0xFF : 0;
		}
	}

	template<>
	void TransferRow<RGBA1010102to8888>(unsigned char *dest, const unsigned char *source, GLsizei width, GLsizei bytes)
	{
		const unsigned int *source1010102 = reinterpret_cast<const unsigned int*>(source);
		unsigned char *dest8888 = dest;

		for(int x = 0; x < width; x++)
		{
			unsigned int rgba = source1010102[x];
			dest8888[4 * x + 0] = sw::unorm<8>((rgba & 0x000003FF) * (1.0f / 0x000003FF));
			dest8888[4 * x + 1] = sw::unorm<8>((rgba & 0x000FFC00) * (1.0f / 0x000FFC00));
			dest8888[4 * x + 2] = sw::unorm<8>((rgba & 0x3FF00000) * (1.0f / 0x3FF00000));
			dest8888[4 * x + 3] = sw::unorm<8>((rgba & 0xC0000000) * (1.0f / 0xC0000000));
		}
	}

	template<>
	void TransferRow<RGB8to565>(unsigned char *dest, const unsigned char *source, GLsizei width, GLsizei bytes)
	{
		unsigned short *dest565 = reinterpret_cast<unsigned short*>(dest);

		for(int x = 0; x < width; x++)
		{
			float r = source[3 * x + 0] * (1.0f / 0xFF);
			float g = source[3 * x + 1] * (1.0f / 0xFF);
			float b = source[3 * x + 2] * (1.0f / 0xFF);
			dest565[x] = (sw::unorm<5>(r) << 11) | (sw::unorm<6>(g) << 5) | (sw::unorm<5>(b) << 0);
		}
	}

	template<>
	void TransferRow<R11G11B10F>(unsigned char *dest, const unsigned char *source, GLsizei width, GLsizei bytes)
	{
		const sw::R11G11B10F *sourceRGB = reinterpret_cast<const sw::R11G11B10F*>(source);
		sw::half *destF = reinterpret_cast<sw::half*>(dest);

		for(int x = 0; x < width; x++, sourceRGB++, destF+=4)
		{
			sourceRGB->toRGB16F(destF);
			destF[3] = 1.0f;
		}
	}

	template<>
	void TransferRow<RGB9E5>(unsigned char *dest, const unsigned char *source, GLsizei width, GLsizei bytes)
	{
		const sw::RGB9E5 *sourceRGB = reinterpret_cast<const sw::RGB9E5*>(source);
		sw::half *destF = reinterpret_cast<sw::half*>(dest);

		for(int x = 0; x < width; x++, sourceRGB++, destF += 4)
		{
			sourceRGB->toRGB16F(destF);
			destF[3] = 1.0f;
		}
	}

	template<>
	void TransferRow<R32Fto16F>(unsigned char *dest, const unsigned char *source, GLsizei width, GLsizei bytes)
	{
		const float *source32F = reinterpret_cast<const float*>(source);
		sw::half *dest16F = reinterpret_cast<sw::half*>(dest);

		for(int x = 0; x < width; x++)
		{
			dest16F[x] = source32F[x];
		}
	}

	template<>
	void TransferRow<RG32Fto16F>(unsigned char *dest, const unsigned char *source, GLsizei width, GLsizei bytes)
	{
		const float *source32F = reinterpret_cast<const float*>(source);
		sw::half *dest16F = reinterpret_cast<sw::half*>(dest);

		for(int x = 0; x < width; x++)
		{
			dest16F[2 * x + 0] = source32F[2 * x + 0];
			dest16F[2 * x + 1] = source32F[2 * x + 1];
		}
	}

	template<>
	void TransferRow<RGB32Fto16F>(unsigned char *dest, const unsigned char *source, GLsizei width, GLsizei bytes)
	{
		const float *source32F = reinterpret_cast<const float*>(source);
		sw::half *dest16F = reinterpret_cast<sw::half*>(dest);

		for(int x = 0; x < width; x++)
		{
			dest16F[4 * x + 0] = source32F[3 * x + 0];
			dest16F[4 * x + 1] = source32F[3 * x + 1];
			dest16F[4 * x + 2] = source32F[3 * x + 3];
			dest16F[4 * x + 3] = 1.0f;
		}
	}

	template<>
	void TransferRow<RGBA32Fto16F>(unsigned char *dest, const unsigned char *source, GLsizei width, GLsizei bytes)
	{
		const sw::RGB9E5 *sourceRGB = reinterpret_cast<const sw::RGB9E5*>(source);
		sw::half *destF = reinterpret_cast<sw::half*>(dest);

		for(int x = 0; x < width; x++, sourceRGB++, destF += 4)
		{
			sourceRGB->toRGB16F(destF);
			destF[3] = 1.0f;
		}
	}

	template<>
	void TransferRow<D16>(unsigned char *dest, const unsigned char *source, GLsizei width, GLsizei bytes)
	{
		const unsigned short *sourceD16 = reinterpret_cast<const unsigned short*>(source);
		float *destF = reinterpret_cast<float*>(dest);

		for(int x = 0; x < width; x++)
		{
			destF[x] = (float)sourceD16[x] / 0xFFFF;
		}
	}

	template<>
	void TransferRow<D24X8>(unsigned char *dest, const unsigned char *source, GLsizei width, GLsizei bytes)
	{
		const unsigned int *sourceD24 = reinterpret_cast<const unsigned int*>(source);
		float *destF = reinterpret_cast<float*>(dest);

		for(int x = 0; x < width; x++)
		{
			destF[x] = (float)(sourceD24[x] & 0xFFFFFF00) / 0xFFFFFF00;
		}
	}

	template<>
	void TransferRow<D32>(unsigned char *dest, const unsigned char *source, GLsizei width, GLsizei bytes)
	{
		const unsigned int *sourceD32 = reinterpret_cast<const unsigned int*>(source);
		float *destF = reinterpret_cast<float*>(dest);

		for(int x = 0; x < width; x++)
		{
			destF[x] = (float)sourceD32[x] / 0xFFFFFFFF;
		}
	}

	template<>
	void TransferRow<X24S8>(unsigned char *dest, const unsigned char *source, GLsizei width, GLsizei bytes)
	{
		const unsigned int *sourceI = reinterpret_cast<const unsigned int*>(source);
		unsigned char *destI = dest;

		for(int x = 0; x < width; x++)
		{
			destI[x] = static_cast<unsigned char>(sourceI[x] & 0x000000FF);   // FIXME: Quad layout
		}
	}

	template<>
	void TransferRow<D32F>(unsigned char *dest, const unsigned char *source, GLsizei width, GLsizei bytes)
	{
		const float *sourceF = reinterpret_cast<const float*>(source);
		float *destF = reinterpret_cast<float*>(dest);

		for(int x = 0; x < width; x++)
		{
			destF[x] = sw::clamp(sourceF[x], 0.0f, 1.0f);
		}
	}

	template<>
	void TransferRow<D32FX32>(unsigned char *dest, const unsigned char *source, GLsizei width, GLsizei bytes)
	{
		struct D32FS8 { float depth32f; unsigned int stencil24_8; };
		const D32FS8 *sourceD32FS8 = reinterpret_cast<const D32FS8*>(source);
		float *destF = reinterpret_cast<float*>(dest);

		for(int x = 0; x < width; x++)
		{
			destF[x] = sw::clamp(sourceD32FS8[x].depth32f, 0.0f, 1.0f);
		}
	}

	template<>
	void TransferRow<X56S8>(unsigned char *dest, const unsigned char *source, GLsizei width, GLsizei bytes)
	{
		struct D32FS8 { float depth32f; unsigned int stencil24_8; };
		const D32FS8 *sourceD32FS8 = reinterpret_cast<const D32FS8*>(source);
		unsigned char *destI = dest;

		for(int x = 0; x < width; x++)
		{
			destI[x] = static_cast<unsigned char>(sourceD32FS8[x].stencil24_8 & 0x000000FF);   // FIXME: Quad layout
		}
	}

	struct Region
	{
		GLsizei bytes;
		GLsizei width;
		GLsizei height;
		GLsizei depth;
		int inputPitch;
		int inputHeight;
		int destPitch;
		GLsizei destSlice;
	};

	template<DataType dataType>
	void Transfer(void *buffer, const void *input, Region region)
	{
		for(int z = 0; z < region.depth; z++)
		{
			const unsigned char *inputStart = static_cast<const unsigned char*>(input) + (z * region.inputPitch * region.inputHeight);
			unsigned char *destStart = static_cast<unsigned char*>(buffer) + (z * region.destSlice);
			for(int y = 0; y < region.height; y++)
			{
				const unsigned char *source = inputStart + y * region.inputPitch;
				unsigned char *dest = destStart + y * region.destPitch;

				TransferRow<dataType>(dest, source, region.width, region.bytes);
			}
		}
	}
}

namespace egl
{
	sw::Format ConvertReadFormatType(GLenum format, GLenum type)
	{
		switch(format)
		{
		case GL_LUMINANCE:
			switch(type)
			{
			case GL_UNSIGNED_BYTE:  return sw::FORMAT_L8;
			case GL_HALF_FLOAT:     return sw::FORMAT_L16F;
			case GL_HALF_FLOAT_OES: return sw::FORMAT_L16F;
			case GL_FLOAT:          return sw::FORMAT_L32F;
			default: UNREACHABLE(type);
			}
			break;
		case GL_LUMINANCE_ALPHA:
			switch(type)
			{
			case GL_UNSIGNED_BYTE:  return sw::FORMAT_A8L8;
			case GL_HALF_FLOAT:     return sw::FORMAT_A16L16F;
			case GL_HALF_FLOAT_OES: return sw::FORMAT_A16L16F;
			case GL_FLOAT:          return sw::FORMAT_A32L32F;
			default: UNREACHABLE(type);
			}
			break;
		case GL_RGBA:
			switch(type)
			{
			case GL_UNSIGNED_BYTE:          return sw::FORMAT_A8B8G8R8;
			case GL_UNSIGNED_SHORT_4_4_4_4: return sw::FORMAT_R4G4B4A4;
			case GL_UNSIGNED_SHORT_5_5_5_1: return sw::FORMAT_R5G5B5A1;
			case GL_HALF_FLOAT:             return sw::FORMAT_A16B16G16R16F;
			case GL_HALF_FLOAT_OES:         return sw::FORMAT_A16B16G16R16F;
			case GL_FLOAT:                  return sw::FORMAT_A32B32G32R32F;
			case GL_UNSIGNED_INT_2_10_10_10_REV_EXT: return sw::FORMAT_A2B10G10R10;
			default: UNREACHABLE(type);
			}
			break;
		case GL_BGRA_EXT:
			switch(type)
			{
			case GL_UNSIGNED_BYTE:                  return sw::FORMAT_A8R8G8B8;
			case GL_UNSIGNED_SHORT_4_4_4_4_REV_EXT: return sw::FORMAT_A4R4G4B4;
			case GL_UNSIGNED_SHORT_1_5_5_5_REV_EXT: return sw::FORMAT_A1R5G5B5;
			default: UNREACHABLE(type);
			}
			break;
		case GL_RGB:
			switch(type)
			{
			case GL_UNSIGNED_BYTE:          return sw::FORMAT_B8G8R8;
			case GL_UNSIGNED_SHORT_5_6_5:   return sw::FORMAT_R5G6B5;
			case GL_HALF_FLOAT:             return sw::FORMAT_B16G16R16F;
			case GL_HALF_FLOAT_OES:         return sw::FORMAT_B16G16R16F;
			case GL_FLOAT:                  return sw::FORMAT_B32G32R32F;
			default: UNREACHABLE(type);
			}
			break;
		case GL_RG:
			switch(type)
			{
			case GL_UNSIGNED_BYTE:          return sw::FORMAT_G8R8;
			case GL_HALF_FLOAT:             return sw::FORMAT_G16R16F;
			case GL_HALF_FLOAT_OES:         return sw::FORMAT_G16R16F;
			case GL_FLOAT:                  return sw::FORMAT_G32R32F;
			default: UNREACHABLE(type);
			}
			break;
		case GL_RED:
			switch(type)
			{
			case GL_UNSIGNED_BYTE:          return sw::FORMAT_R8;
			case GL_HALF_FLOAT:             return sw::FORMAT_R16F;
			case GL_HALF_FLOAT_OES:         return sw::FORMAT_R16F;
			case GL_FLOAT:                  return sw::FORMAT_R32F;
			default: UNREACHABLE(type);
			}
			break;
		case GL_ALPHA:
			switch(type)
			{
			case GL_UNSIGNED_BYTE:          return sw::FORMAT_A8;
			case GL_HALF_FLOAT:             return sw::FORMAT_A16F;
			case GL_HALF_FLOAT_OES:         return sw::FORMAT_A16F;
			case GL_FLOAT:                  return sw::FORMAT_A32F;
			default: UNREACHABLE(type);
			}
			break;
		case GL_RED_INTEGER:
			switch(type)
			{
			case GL_INT:          return sw::FORMAT_R32I;
			case GL_UNSIGNED_INT: return sw::FORMAT_R32UI;
			default: UNREACHABLE(type);
			}
			break;
		case GL_RG_INTEGER:
			switch(type)
			{
			case GL_INT:          return sw::FORMAT_G32R32I;
			case GL_UNSIGNED_INT: return sw::FORMAT_G32R32UI;
			default: UNREACHABLE(type);
			}
			break;
		case GL_RGB_INTEGER:
			switch(type)
			{
			case GL_INT:          return sw::FORMAT_X32B32G32R32I;
			case GL_UNSIGNED_INT: return sw::FORMAT_X32B32G32R32UI;
			default: UNREACHABLE(type);
			}
			break;
		case GL_RGBA_INTEGER:
			switch(type)
			{
			case GL_INT:          return sw::FORMAT_A32B32G32R32I;
			case GL_UNSIGNED_INT: return sw::FORMAT_A32B32G32R32UI;
			case GL_UNSIGNED_INT_2_10_10_10_REV: return sw::FORMAT_A2B10G10R10UI;
			default: UNREACHABLE(type);
			}
			break;
		case GL_DEPTH_COMPONENT:
			switch(type)
			{
			case GL_UNSIGNED_SHORT:        return sw::FORMAT_D16;
			case GL_UNSIGNED_INT_24_8_OES: return sw::FORMAT_D24S8;
			case GL_UNSIGNED_INT:          return sw::FORMAT_D32;
			case GL_FLOAT:                 return sw::FORMAT_D32F_LOCKABLE;
			default: UNREACHABLE(type);
			}
			break;
		default:
			UNREACHABLE(format);
			break;
		}

		return sw::FORMAT_NULL;
	}

	sw::Format SelectInternalFormat(GLint format)
	{
		switch(format)
		{
		case GL_NONE:                 return sw::FORMAT_NULL;
		case GL_RGBA4:                return sw::FORMAT_A8B8G8R8;
		case GL_RGB5_A1:              return sw::FORMAT_A8B8G8R8;
		case GL_RGBA8:                return sw::FORMAT_A8B8G8R8;
		case GL_RGB565:               return sw::FORMAT_R5G6B5;
		case GL_RGB8:                 return sw::FORMAT_X8B8G8R8;

		case GL_DEPTH_COMPONENT32F:    return sw::FORMAT_D32F_LOCKABLE;
		case GL_DEPTH_COMPONENT16:     return sw::FORMAT_D32F_LOCKABLE;
		case GL_DEPTH_COMPONENT24:     return sw::FORMAT_D32F_LOCKABLE;
		case GL_DEPTH_COMPONENT32_OES: return sw::FORMAT_D32F_LOCKABLE;
		case GL_DEPTH24_STENCIL8:      return sw::FORMAT_D32FS8_TEXTURE;
		case GL_DEPTH32F_STENCIL8:     return sw::FORMAT_D32FS8_TEXTURE;
		case GL_STENCIL_INDEX8:        return sw::FORMAT_S8;

		case GL_R8:                   return sw::FORMAT_R8;
		case GL_RG8:                  return sw::FORMAT_G8R8;
		case GL_R8I:                  return sw::FORMAT_R8I;
		case GL_RG8I:                 return sw::FORMAT_G8R8I;
		case GL_RGB8I:                return sw::FORMAT_X8B8G8R8I;
		case GL_RGBA8I:               return sw::FORMAT_A8B8G8R8I;
		case GL_R8UI:                 return sw::FORMAT_R8UI;
		case GL_RG8UI:                return sw::FORMAT_G8R8UI;
		case GL_RGB8UI:               return sw::FORMAT_X8B8G8R8UI;
		case GL_RGBA8UI:              return sw::FORMAT_A8B8G8R8UI;
		case GL_R16I:                 return sw::FORMAT_R16I;
		case GL_RG16I:                return sw::FORMAT_G16R16I;
		case GL_RGB16I:               return sw::FORMAT_X16B16G16R16I;
		case GL_RGBA16I:              return sw::FORMAT_A16B16G16R16I;
		case GL_R16UI:                return sw::FORMAT_R16UI;
		case GL_RG16UI:               return sw::FORMAT_G16R16UI;
		case GL_RGB16UI:              return sw::FORMAT_X16B16G16R16UI;
		case GL_RGBA16UI:             return sw::FORMAT_A16B16G16R16UI;
		case GL_R32I:                 return sw::FORMAT_R32I;
		case GL_RG32I:                return sw::FORMAT_G32R32I;
		case GL_RGB32I:               return sw::FORMAT_X32B32G32R32I;
		case GL_RGBA32I:              return sw::FORMAT_A32B32G32R32I;
		case GL_R32UI:                return sw::FORMAT_R32UI;
		case GL_RG32UI:               return sw::FORMAT_G32R32UI;
		case GL_RGB32UI:              return sw::FORMAT_X32B32G32R32UI;
		case GL_RGBA32UI:             return sw::FORMAT_A32B32G32R32UI;
		case GL_R16F:                 return sw::FORMAT_R16F;
		case GL_RG16F:                return sw::FORMAT_G16R16F;
		case GL_R11F_G11F_B10F:       return sw::FORMAT_X16B16G16R16F_UNSIGNED;
		case GL_RGB16F:               return sw::FORMAT_X16B16G16R16F;
		case GL_RGBA16F:              return sw::FORMAT_A16B16G16R16F;
		case GL_R32F:                 return sw::FORMAT_R32F;
		case GL_RG32F:                return sw::FORMAT_G32R32F;
		case GL_RGB32F:               return sw::FORMAT_X32B32G32R32F;
		case GL_RGBA32F:              return sw::FORMAT_A32B32G32R32F;
		case GL_RGB10_A2:             return sw::FORMAT_A2B10G10R10;
		case GL_RGB10_A2UI:           return sw::FORMAT_A2B10G10R10UI;
		case GL_SRGB8:                return sw::FORMAT_SRGB8_X8;
		case GL_SRGB8_ALPHA8:         return sw::FORMAT_SRGB8_A8;


		case GL_ETC1_RGB8_OES: return sw::FORMAT_ETC1;
		case GL_COMPRESSED_R11_EAC: return sw::FORMAT_R11_EAC;
		case GL_COMPRESSED_SIGNED_R11_EAC: return sw::FORMAT_SIGNED_R11_EAC;
		case GL_COMPRESSED_RG11_EAC: return sw::FORMAT_RG11_EAC;
		case GL_COMPRESSED_SIGNED_RG11_EAC: return sw::FORMAT_SIGNED_RG11_EAC;
		case GL_COMPRESSED_RGB8_ETC2: return sw::FORMAT_RGB8_ETC2;
		case GL_COMPRESSED_SRGB8_ETC2: return sw::FORMAT_SRGB8_ETC2;
		case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2: return sw::FORMAT_RGB8_PUNCHTHROUGH_ALPHA1_ETC2;
		case GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2: return sw::FORMAT_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2;
		case GL_COMPRESSED_RGBA8_ETC2_EAC: return sw::FORMAT_RGBA8_ETC2_EAC;
		case GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC: return sw::FORMAT_SRGB8_ALPHA8_ETC2_EAC;
		case GL_COMPRESSED_RGBA_ASTC_4x4_KHR: return sw::FORMAT_RGBA_ASTC_4x4_KHR;
		case GL_COMPRESSED_RGBA_ASTC_5x4_KHR: return sw::FORMAT_RGBA_ASTC_5x4_KHR;
		case GL_COMPRESSED_RGBA_ASTC_5x5_KHR: return sw::FORMAT_RGBA_ASTC_5x5_KHR;
		case GL_COMPRESSED_RGBA_ASTC_6x5_KHR: return sw::FORMAT_RGBA_ASTC_6x5_KHR;
		case GL_COMPRESSED_RGBA_ASTC_6x6_KHR: return sw::FORMAT_RGBA_ASTC_6x6_KHR;
		case GL_COMPRESSED_RGBA_ASTC_8x5_KHR: return sw::FORMAT_RGBA_ASTC_8x5_KHR;
		case GL_COMPRESSED_RGBA_ASTC_8x6_KHR: return sw::FORMAT_RGBA_ASTC_8x6_KHR;
		case GL_COMPRESSED_RGBA_ASTC_8x8_KHR: return sw::FORMAT_RGBA_ASTC_8x8_KHR;
		case GL_COMPRESSED_RGBA_ASTC_10x5_KHR: return sw::FORMAT_RGBA_ASTC_10x5_KHR;
		case GL_COMPRESSED_RGBA_ASTC_10x6_KHR: return sw::FORMAT_RGBA_ASTC_10x6_KHR;
		case GL_COMPRESSED_RGBA_ASTC_10x8_KHR: return sw::FORMAT_RGBA_ASTC_10x8_KHR;
		case GL_COMPRESSED_RGBA_ASTC_10x10_KHR: return sw::FORMAT_RGBA_ASTC_10x10_KHR;
		case GL_COMPRESSED_RGBA_ASTC_12x10_KHR: return sw::FORMAT_RGBA_ASTC_12x10_KHR;
		case GL_COMPRESSED_RGBA_ASTC_12x12_KHR: return sw::FORMAT_RGBA_ASTC_12x12_KHR;
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR: return sw::FORMAT_SRGB8_ALPHA8_ASTC_4x4_KHR;
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR: return sw::FORMAT_SRGB8_ALPHA8_ASTC_5x4_KHR;
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR: return sw::FORMAT_SRGB8_ALPHA8_ASTC_5x5_KHR;
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR: return sw::FORMAT_SRGB8_ALPHA8_ASTC_6x5_KHR;
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR: return sw::FORMAT_SRGB8_ALPHA8_ASTC_6x6_KHR;
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR: return sw::FORMAT_SRGB8_ALPHA8_ASTC_8x5_KHR;
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR: return sw::FORMAT_SRGB8_ALPHA8_ASTC_8x6_KHR;
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR: return sw::FORMAT_SRGB8_ALPHA8_ASTC_8x8_KHR;
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR: return sw::FORMAT_SRGB8_ALPHA8_ASTC_10x5_KHR;
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR: return sw::FORMAT_SRGB8_ALPHA8_ASTC_10x6_KHR;
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR: return sw::FORMAT_SRGB8_ALPHA8_ASTC_10x8_KHR;
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR: return sw::FORMAT_SRGB8_ALPHA8_ASTC_10x10_KHR;
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR: return sw::FORMAT_SRGB8_ALPHA8_ASTC_12x10_KHR;
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR: return sw::FORMAT_SRGB8_ALPHA8_ASTC_12x12_KHR;
		case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
		case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT: return sw::FORMAT_DXT1;
		case GL_COMPRESSED_RGBA_S3TC_DXT3_ANGLE: return sw::FORMAT_DXT3;
		case GL_COMPRESSED_RGBA_S3TC_DXT5_ANGLE: return sw::FORMAT_DXT5;
		case GL_ALPHA32F_EXT: return sw::FORMAT_A32F;
		case GL_LUMINANCE32F_EXT: return sw::FORMAT_L32F;
		case GL_LUMINANCE_ALPHA32F_EXT: return sw::FORMAT_A32L32F;
		case GL_RGB9_E5: return sw::FORMAT_X16B16G16R16F_UNSIGNED;
		case GL_ALPHA16F_EXT: return sw::FORMAT_A16F;
		case GL_LUMINANCE16F_EXT: return sw::FORMAT_L16F;
		case GL_LUMINANCE_ALPHA16F_EXT: return sw::FORMAT_A16L16F;
		case GL_R8_SNORM: return sw::FORMAT_R8_SNORM;
		case GL_RG8_SNORM: return sw::FORMAT_G8R8_SNORM;
		case GL_RGB8_SNORM: return sw::FORMAT_X8B8G8R8_SNORM;
		case GL_RGBA8_SNORM: return sw::FORMAT_A8B8G8R8_SNORM;
		case GL_LUMINANCE8_EXT: return sw::FORMAT_L8;
		case GL_LUMINANCE8_ALPHA8_EXT: return sw::FORMAT_A8L8;
		case GL_BGRA8_EXT: return sw::FORMAT_A8R8G8B8;
		case GL_ALPHA8_EXT: return sw::FORMAT_A8;
		case SW_YV12_BT601: return sw::FORMAT_YV12_BT601;
		case SW_YV12_BT709: return sw::FORMAT_YV12_BT709;
		case SW_YV12_JFIF: return sw::FORMAT_YV12_JFIF;
		}

		return sw::FORMAT_NULL;
	}

	// Returns the size, in bytes, of a single client-side pixel.
    // OpenGL ES 3.0.5 table 3.2.
	static int ComputePixelSize(GLenum format, GLenum type)
	{
		switch(format)
		{
		case GL_RED:
		case GL_RED_INTEGER:
		case GL_ALPHA:
		case GL_LUMINANCE:
			switch(type)
			{
			case GL_BYTE:           return 1;
			case GL_UNSIGNED_BYTE:  return 1;
			case GL_FLOAT:          return 4;
			case GL_HALF_FLOAT:     return 2;
			case GL_HALF_FLOAT_OES: return 2;
			case GL_SHORT:          return 2;
			case GL_UNSIGNED_SHORT: return 2;
			case GL_INT:            return 4;
			case GL_UNSIGNED_INT:   return 4;
			default: UNREACHABLE(type);
			}
			break;
		case GL_RG:
		case GL_RG_INTEGER:
		case GL_LUMINANCE_ALPHA:
			switch(type)
			{
			case GL_BYTE:           return 2;
			case GL_UNSIGNED_BYTE:  return 2;
			case GL_FLOAT:          return 8;
			case GL_HALF_FLOAT:     return 4;
			case GL_HALF_FLOAT_OES: return 4;
			case GL_SHORT:          return 4;
			case GL_UNSIGNED_SHORT: return 4;
			case GL_INT:            return 8;
			case GL_UNSIGNED_INT:   return 8;
			default: UNREACHABLE(type);
			}
			break;
		case GL_RGB:
		case GL_RGB_INTEGER:
			switch(type)
			{
			case GL_BYTE:                         return 3;
			case GL_UNSIGNED_BYTE:                return 3;
			case GL_UNSIGNED_SHORT_5_6_5:         return 2;
			case GL_UNSIGNED_INT_10F_11F_11F_REV: return 4;
			case GL_UNSIGNED_INT_5_9_9_9_REV:     return 4;
			case GL_FLOAT:                        return 12;
			case GL_HALF_FLOAT:                   return 6;
			case GL_HALF_FLOAT_OES:               return 6;
			case GL_SHORT:                        return 6;
			case GL_UNSIGNED_SHORT:               return 6;
			case GL_INT:                          return 12;
			case GL_UNSIGNED_INT:                 return 12;
			default: UNREACHABLE(type);
			}
			break;
		case GL_RGBA:
		case GL_RGBA_INTEGER:
		case GL_BGRA_EXT:
			switch(type)
			{
			case GL_BYTE:                           return 4;
			case GL_UNSIGNED_BYTE:                  return 4;
			case GL_UNSIGNED_SHORT_4_4_4_4:         return 2;
			case GL_UNSIGNED_SHORT_4_4_4_4_REV_EXT: return 2;
			case GL_UNSIGNED_SHORT_5_5_5_1:         return 2;
			case GL_UNSIGNED_SHORT_1_5_5_5_REV_EXT: return 2;
			case GL_UNSIGNED_INT_2_10_10_10_REV:    return 4;
			case GL_FLOAT:                          return 16;
			case GL_HALF_FLOAT:                     return 8;
			case GL_HALF_FLOAT_OES:                 return 8;
			case GL_SHORT:                          return 8;
			case GL_UNSIGNED_SHORT:                 return 8;
			case GL_INT:                            return 16;
			case GL_UNSIGNED_INT:                   return 16;
			default: UNREACHABLE(type);
			}
			break;
		case GL_DEPTH_COMPONENT:
			switch(type)
			{
			case GL_FLOAT:          return 4;
			case GL_UNSIGNED_SHORT: return 2;
			case GL_UNSIGNED_INT:   return 4;
			default: UNREACHABLE(type);
			}
			break;
		case GL_DEPTH_STENCIL:
			switch(type)
			{
			case GL_UNSIGNED_INT_24_8:              return 4;
			case GL_FLOAT_32_UNSIGNED_INT_24_8_REV: return 8;
			default: UNREACHABLE(type);
			}
			break;
		default:
			UNREACHABLE(format);
		}

		return 0;
	}

	GLsizei ComputePitch(GLsizei width, GLenum format, GLenum type, GLint alignment)
	{
		ASSERT(alignment > 0 && sw::isPow2(alignment));

		GLsizei rawPitch = ComputePixelSize(format, type) * width;
		return (rawPitch + alignment - 1) & ~(alignment - 1);
	}

	size_t ComputePackingOffset(GLenum format, GLenum type, GLsizei width, GLsizei height, const PixelStorageModes &storageModes)
	{
		GLsizei pitchB = ComputePitch(width, format, type, storageModes.alignment);
		return (storageModes.skipImages * height + storageModes.skipRows) * pitchB + storageModes.skipPixels * ComputePixelSize(format, type);
	}

	inline GLsizei ComputeCompressedPitch(GLsizei width, GLenum format)
	{
		return ComputeCompressedSize(width, 1, format);
	}

	GLsizei ComputeCompressedSize(GLsizei width, GLsizei height, GLenum format)
	{
		switch(format)
		{
		case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
		case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
		case GL_ETC1_RGB8_OES:
		case GL_COMPRESSED_R11_EAC:
		case GL_COMPRESSED_SIGNED_R11_EAC:
		case GL_COMPRESSED_RGB8_ETC2:
		case GL_COMPRESSED_SRGB8_ETC2:
		case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:
		case GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2:
			return 8 * getNumBlocks(width, height, 4, 4);
		case GL_COMPRESSED_RGBA_S3TC_DXT3_ANGLE:
		case GL_COMPRESSED_RGBA_S3TC_DXT5_ANGLE:
		case GL_COMPRESSED_RG11_EAC:
		case GL_COMPRESSED_SIGNED_RG11_EAC:
		case GL_COMPRESSED_RGBA8_ETC2_EAC:
		case GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:
		case GL_COMPRESSED_RGBA_ASTC_4x4_KHR:
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR:
			return 16 * getNumBlocks(width, height, 4, 4);
		case GL_COMPRESSED_RGBA_ASTC_5x4_KHR:
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR:
			return 16 * getNumBlocks(width, height, 5, 4);
		case GL_COMPRESSED_RGBA_ASTC_5x5_KHR:
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR:
			return 16 * getNumBlocks(width, height, 5, 5);
		case GL_COMPRESSED_RGBA_ASTC_6x5_KHR:
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR:
			return 16 * getNumBlocks(width, height, 6, 5);
		case GL_COMPRESSED_RGBA_ASTC_6x6_KHR:
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR:
			return 16 * getNumBlocks(width, height, 6, 6);
		case GL_COMPRESSED_RGBA_ASTC_8x5_KHR:
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR:
			return 16 * getNumBlocks(width, height, 8, 5);
		case GL_COMPRESSED_RGBA_ASTC_8x6_KHR:
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR:
			return 16 * getNumBlocks(width, height, 8, 6);
		case GL_COMPRESSED_RGBA_ASTC_8x8_KHR:
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR:
			return 16 * getNumBlocks(width, height, 8, 8);
		case GL_COMPRESSED_RGBA_ASTC_10x5_KHR:
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR:
			return 16 * getNumBlocks(width, height, 10, 5);
		case GL_COMPRESSED_RGBA_ASTC_10x6_KHR:
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR:
			return 16 * getNumBlocks(width, height, 10, 6);
		case GL_COMPRESSED_RGBA_ASTC_10x8_KHR:
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR:
			return 16 * getNumBlocks(width, height, 10, 8);
		case GL_COMPRESSED_RGBA_ASTC_10x10_KHR:
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR:
			return 16 * getNumBlocks(width, height, 10, 10);
		case GL_COMPRESSED_RGBA_ASTC_12x10_KHR:
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR:
			return 16 * getNumBlocks(width, height, 12, 10);
		case GL_COMPRESSED_RGBA_ASTC_12x12_KHR:
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR:
			return 16 * getNumBlocks(width, height, 12, 12);
		default:
			return 0;
		}
	}

	class ImageImplementation : public Image
	{
	public:
		ImageImplementation(Texture *parentTexture, GLsizei width, GLsizei height, GLint format)
			: Image(parentTexture, width, height, format) {}
		ImageImplementation(Texture *parentTexture, GLsizei width, GLsizei height, GLsizei depth, int border, GLint format)
			: Image(parentTexture, width, height, depth, border, format) {}
		ImageImplementation(GLsizei width, GLsizei height, GLint format, int pitchP)
			: Image(width, height, format, pitchP) {}
		ImageImplementation(GLsizei width, GLsizei height, sw::Format internalFormat, int multiSampleDepth, bool lockable)
			: Image(width, height, internalFormat, multiSampleDepth, lockable) {}

		~ImageImplementation() override
		{
			sync();   // Wait for any threads that use this image to finish.
		}

		void *lockInternal(int x, int y, int z, sw::Lock lock, sw::Accessor client) override
		{
			return Image::lockInternal(x, y, z, lock, client);
		}

		void unlockInternal() override
		{
			return Image::unlockInternal();
		}

		void release() override
		{
			return Image::release();
		}
	};

	Image *Image::create(Texture *parentTexture, GLsizei width, GLsizei height, GLint format)
	{
		return new ImageImplementation(parentTexture, width, height, format);
	}

	Image *Image::create(Texture *parentTexture, GLsizei width, GLsizei height, GLsizei depth, int border, GLint format)
	{
		return new ImageImplementation(parentTexture, width, height, depth, border, format);
	}

	Image *Image::create(GLsizei width, GLsizei height, GLint format, int pitchP)
	{
		return new ImageImplementation(width, height, format, pitchP);
	}

	Image *Image::create(GLsizei width, GLsizei height, sw::Format internalFormat, int multiSampleDepth, bool lockable)
	{
		return new ImageImplementation(width, height, internalFormat, multiSampleDepth, lockable);
	}

	Image::~Image()
	{
		// sync() must be called in the destructor of the most derived class to ensure their vtable isn't destroyed
		// before all threads are done using this image. Image itself is abstract so it can't be the most derived.
		ASSERT(isUnlocked());

		if(parentTexture)
		{
			parentTexture->release();
		}

		ASSERT(!shared);
	}

	void *Image::lockInternal(int x, int y, int z, sw::Lock lock, sw::Accessor client)
	{
		return Surface::lockInternal(x, y, z, lock, client);
	}

	void Image::unlockInternal()
	{
		Surface::unlockInternal();
	}

	void Image::release()
	{
		int refs = dereference();

		if(refs > 0)
		{
			if(parentTexture)
			{
				parentTexture->sweep();
			}
		}
		else
		{
			delete this;
		}
	}

	void Image::unbind(const egl::Texture *parent)
	{
		if(parentTexture == parent)
		{
			parentTexture = nullptr;
		}

		release();
	}

	bool Image::isChildOf(const egl::Texture *parent) const
	{
		return parentTexture == parent;
	}

	void Image::loadImageData(GLsizei width, GLsizei height, GLsizei depth, int inputPitch, int inputHeight, GLenum format, GLenum type, const void *input, void *buffer)
	{
		Region region;
		region.bytes = ComputePixelSize(format, type);
		region.width = width;
		region.height = height;
		region.depth = depth;
		region.inputPitch = inputPitch;
		region.inputHeight = inputHeight;
		region.destPitch = getPitch();
		region.destSlice = getSlice();

		// OpenGL ES 3.0.5 table 3.2.
		switch(format)
		{
		case GL_RED:
		case GL_RED_INTEGER:
		case GL_ALPHA:
		case GL_LUMINANCE:
			switch(type)
			{
			case GL_BYTE:
			case GL_UNSIGNED_BYTE:
				return Transfer<Bytes>(buffer, input, region);
			case GL_FLOAT:
				ASSERT(format != GL_RED_INTEGER);
				switch(internalformat)
				{
				case GL_R32F:
					return Transfer<Bytes>(buffer, input, region);
				case GL_R16F:
					return Transfer<R32Fto16F>(buffer, input, region);
				default:
					UNREACHABLE(internalformat);
				}
			case GL_HALF_FLOAT:
			case GL_HALF_FLOAT_OES:
				ASSERT(format != GL_RED_INTEGER);
				return Transfer<Bytes>(buffer, input, region);
			case GL_SHORT:
			case GL_UNSIGNED_SHORT:
				return Transfer<Bytes>(buffer, input, region);
			case GL_INT:
			case GL_UNSIGNED_INT:
				return Transfer<Bytes>(buffer, input, region);
			default:
				UNREACHABLE(type);
			}
		case GL_RG:
		case GL_RG_INTEGER:
		case GL_LUMINANCE_ALPHA:
			switch(type)
			{
			case GL_BYTE:
			case GL_UNSIGNED_BYTE:
				return Transfer<Bytes>(buffer, input, region);
			case GL_FLOAT:
				ASSERT(format != GL_RG_INTEGER);
				switch(internalformat)
				{
				case GL_RG32F:
					return Transfer<Bytes>(buffer, input, region);
				case GL_RG16F:
					return Transfer<RG32Fto16F>(buffer, input, region);
				default:
					UNREACHABLE(internalformat);
				}
			case GL_HALF_FLOAT:
			case GL_HALF_FLOAT_OES:
				ASSERT(format != GL_RG_INTEGER);
				return Transfer<Bytes>(buffer, input, region);
			case GL_SHORT:
			case GL_UNSIGNED_SHORT:
				return Transfer<Bytes>(buffer, input, region);
			case GL_INT:
			case GL_UNSIGNED_INT:
				return Transfer<Bytes>(buffer, input, region);
			default:
				UNREACHABLE(type);
			}
		case GL_RGB:
		case GL_RGB_INTEGER:
			switch(type)
			{
			case GL_BYTE:
				return Transfer<ByteRGB>(buffer, input, region);
			case GL_UNSIGNED_BYTE:
				switch(internalformat)
				{
				case GL_RGB8:
				case GL_RGB8UI:
				case GL_SRGB8:
					return Transfer<UByteRGB>(buffer, input, region);
				case GL_RGB565:
					return Transfer<RGB8to565>(buffer, input, region);
				default:
					UNREACHABLE(internalformat);
				}
			case GL_UNSIGNED_SHORT_5_6_5:
				ASSERT(format == GL_RGB);
				return Transfer<Bytes>(buffer, input, region);
			case GL_UNSIGNED_INT_10F_11F_11F_REV:
				ASSERT(format == GL_RGB);
				return Transfer<R11G11B10F>(buffer, input, region);
			case GL_UNSIGNED_INT_5_9_9_9_REV:
				ASSERT(format == GL_RGB);
				return Transfer<RGB9E5>(buffer, input, region);
			case GL_FLOAT:
				ASSERT(format == GL_RGB);
				switch(internalformat)
				{
				case GL_RGB32F:
					return Transfer<FloatRGB>(buffer, input, region);
				case GL_RGB16F:
				case GL_R11F_G11F_B10F:
				case GL_RGB9_E5:
					ASSERT(implementationFormat == sw::FORMAT_X16B16G16R16F);
					return Transfer<RGB32Fto16F>(buffer, input, region);
				default:
					UNREACHABLE(internalformat);
				}
			case GL_HALF_FLOAT:
			case GL_HALF_FLOAT_OES:
				ASSERT(format == GL_RGB);
				switch(internalformat)
				{
				case GL_RGB16F:
				case GL_R11F_G11F_B10F:
				case GL_RGB9_E5:
					ASSERT(implementationFormat == sw::FORMAT_X16B16G16R16F);
					return Transfer<HalfFloatRGB>(buffer, input, region);
				default:
					UNREACHABLE(internalformat);
				}
			case GL_SHORT:
				return Transfer<ShortRGB>(buffer, input, region);
			case GL_UNSIGNED_SHORT:
				return Transfer<UShortRGB>(buffer, input, region);
			case GL_INT:
				return Transfer<IntRGB>(buffer, input, region);
			case GL_UNSIGNED_INT:
				return Transfer<UIntRGB>(buffer, input, region);
			default:
				UNREACHABLE(type);
			}
		case GL_RGBA:
		case GL_RGBA_INTEGER:
		case GL_BGRA_EXT:
			switch(type)
			{
			case GL_BYTE:
			case GL_UNSIGNED_BYTE:
				switch(internalformat)
				{
				case GL_RGBA8:
				case GL_RGBA8I:
				case GL_RGBA8UI:
				case GL_RGBA8_SNORM:
				case GL_SRGB8_ALPHA8:
				case GL_BGRA8_EXT:
					return Transfer<Bytes>(buffer, input, region);
				case GL_RGB5_A1:
				case GL_RGBA4:
					ASSERT(format == GL_RGBA && implementationFormat == sw::FORMAT_A8B8G8R8);
					return Transfer<Bytes>(buffer, input, region);
				default:
					UNREACHABLE(internalformat);
				}
			case GL_UNSIGNED_SHORT_4_4_4_4:
				ASSERT(format == GL_RGBA);
				return Transfer<RGBA4444>(buffer, input, region);
			case GL_UNSIGNED_SHORT_5_5_5_1:
				ASSERT(format == GL_RGBA);
				return Transfer<RGBA5551>(buffer, input, region);
			case GL_UNSIGNED_INT_2_10_10_10_REV:
				ASSERT(format != GL_BGRA_EXT);
				switch(internalformat)
				{
				case GL_RGB10_A2:
				case GL_RGB10_A2UI:
					return Transfer<Bytes>(buffer, input, region);
				case GL_RGB5_A1:
					ASSERT(format == GL_RGBA && implementationFormat == sw::FORMAT_A8B8G8R8);
					return Transfer<RGBA1010102to8888>(buffer, input, region);
				default:
					UNREACHABLE(internalformat);
				}
			case GL_FLOAT:
				ASSERT(format == GL_RGBA);
				switch(internalformat)
				{
				case GL_RGBA32F:
					return Transfer<Bytes>(buffer, input, region);
				case GL_RGBA16F:
					return Transfer<RGBA32Fto16F>(buffer, input, region);
				default:
					UNREACHABLE(internalformat);
				}
			case GL_HALF_FLOAT:
			case GL_HALF_FLOAT_OES:
				ASSERT(format == GL_RGBA);
				return Transfer<Bytes>(buffer, input, region);
			case GL_SHORT:
			case GL_UNSIGNED_SHORT:
				ASSERT(format != GL_BGRA_EXT);
				return Transfer<Bytes>(buffer, input, region);
			case GL_INT:
			case GL_UNSIGNED_INT:
				ASSERT(format != GL_BGRA_EXT);
				return Transfer<Bytes>(buffer, input, region);
			case GL_UNSIGNED_SHORT_4_4_4_4_REV_EXT:   // Only valid for glReadPixels calls.
			case GL_UNSIGNED_SHORT_1_5_5_5_REV_EXT:   // Only valid for glReadPixels calls.
			default:
				UNREACHABLE(type);
			}
		case GL_DEPTH_COMPONENT:
		case GL_DEPTH_STENCIL:
			switch(type)
			{
			case GL_FLOAT:                          return Transfer<D32F>(buffer, input, region);
			case GL_UNSIGNED_SHORT:                 return Transfer<D16>(buffer, input, region);
			case GL_UNSIGNED_INT:                   return Transfer<D32>(buffer, input, region);
			case GL_UNSIGNED_INT_24_8:              return Transfer<D24X8>(buffer, input, region);
			case GL_FLOAT_32_UNSIGNED_INT_24_8_REV: return Transfer<D32FX32>(buffer, input, region);
			default: UNREACHABLE(type);
			}
		default:
			UNREACHABLE(format);
		}
	}

	void Image::loadStencilData(GLsizei width, GLsizei height, GLsizei depth, int inputPitch, int inputHeight, GLenum format, GLenum type, const void *input, void *buffer)
	{
		Region region;
		region.bytes = ComputePixelSize(format, type);
		region.width = width;
		region.height = height;
		region.depth = depth;
		region.inputPitch = inputPitch;
		region.inputHeight = inputHeight;
		region.destPitch = getStencilPitchB();
		region.destSlice = getStencilSliceB();

		switch(type)
		{
		case GL_UNSIGNED_INT_24_8:              return Transfer<X24S8>(buffer, input, region);
		case GL_FLOAT_32_UNSIGNED_INT_24_8_REV: return Transfer<X56S8>(buffer, input, region);
		default: UNREACHABLE(format);
		}
	}

	void Image::loadImageData(Context *context, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const PixelStorageModes &unpackParameters, const void *pixels)
	{
		GLsizei inputWidth = (unpackParameters.rowLength == 0) ? width : unpackParameters.rowLength;
		GLsizei inputPitch = ComputePitch(inputWidth, format, type, unpackParameters.alignment);
		GLsizei inputHeight = (unpackParameters.imageHeight == 0) ? height : unpackParameters.imageHeight;
		char *input = ((char*)pixels) + ComputePackingOffset(format, type, inputWidth, inputHeight, unpackParameters);

		void *buffer = lock(xoffset, yoffset, zoffset, sw::LOCK_WRITEONLY);

		if(buffer)
		{
			loadImageData(width, height, depth, inputPitch, inputHeight, format, type, input, buffer);
		}

		unlock();

		if(hasStencil())
		{
			unsigned char *stencil = reinterpret_cast<unsigned char*>(lockStencil(xoffset, yoffset, zoffset, sw::PUBLIC));

			if(stencil)
			{
				loadStencilData(width, height, depth, inputPitch, inputHeight, format, type, input, stencil);
			}

			unlockStencil();
		}
	}

	void Image::loadCompressedData(GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLsizei imageSize, const void *pixels)
	{
		int inputPitch = ComputeCompressedPitch(width, internalformat);
		int inputSlice = imageSize / depth;
		int rows = inputSlice / inputPitch;

		void *buffer = lock(xoffset, yoffset, zoffset, sw::LOCK_WRITEONLY);

		if(buffer)
		{
			for(int z = 0; z < depth; z++)
			{
				for(int y = 0; y < rows; y++)
				{
					GLbyte *dest = (GLbyte*)buffer + y * getPitch() + z * getSlice();
					GLbyte *source = (GLbyte*)pixels + y * inputPitch + z * inputSlice;
					memcpy(dest, source, inputPitch);
				}
			}
		}

		unlock();
	}
}
