// SwiftShader Software Renderer
//
// Copyright(c) 2005-2013 TransGaming Inc.
//
// All rights reserved. No part of this software may be copied, distributed, transmitted,
// transcribed, stored in a retrieval system, translated into any human or computer
// language by any means, or disclosed to third parties without the explicit written
// agreement of TransGaming Inc. Without such an agreement, no rights or licenses, express
// or implied, including but not limited to any patent rights, are granted to you.
//

#include "Image.hpp"

#include "../libEGL/Texture.hpp"
#include "../common/debug.h"
#include "Common/Math.hpp"
#include "Common/Thread.hpp"

#include <GLES/glext.h>
#include <GLES2/gl2ext.h>
#include <GLES3/gl3.h>

#include <string.h>

namespace egl
{
	sw::Format SelectInternalFormat(GLenum format, GLenum type)
	{
		switch(format)
		{
		case GL_ETC1_RGB8_OES:
			return sw::FORMAT_ETC1;
		case GL_COMPRESSED_R11_EAC:
			return sw::FORMAT_R11_EAC;
		case GL_COMPRESSED_SIGNED_R11_EAC:
			return sw::FORMAT_SIGNED_R11_EAC;
		case GL_COMPRESSED_RG11_EAC:
			return sw::FORMAT_RG11_EAC;
		case GL_COMPRESSED_SIGNED_RG11_EAC:
			return sw::FORMAT_SIGNED_RG11_EAC;
		case GL_COMPRESSED_RGB8_ETC2:
			return sw::FORMAT_RGB8_ETC2;
		case GL_COMPRESSED_SRGB8_ETC2:
			return sw::FORMAT_SRGB8_ETC2;
		case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:
			return sw::FORMAT_RGB8_PUNCHTHROUGH_ALPHA1_ETC2;
		case GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2:
			return sw::FORMAT_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2;
		case GL_COMPRESSED_RGBA8_ETC2_EAC:
			return sw::FORMAT_RGBA8_ETC2_EAC;
		case GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:
			return sw::FORMAT_SRGB8_ALPHA8_ETC2_EAC;
		case GL_COMPRESSED_RGBA_ASTC_4x4_KHR:
			return sw::FORMAT_RGBA_ASTC_4x4_KHR;
		case GL_COMPRESSED_RGBA_ASTC_5x4_KHR:
			return sw::FORMAT_RGBA_ASTC_5x4_KHR;
		case GL_COMPRESSED_RGBA_ASTC_5x5_KHR:
			return sw::FORMAT_RGBA_ASTC_5x5_KHR;
		case GL_COMPRESSED_RGBA_ASTC_6x5_KHR:
			return sw::FORMAT_RGBA_ASTC_6x5_KHR;
		case GL_COMPRESSED_RGBA_ASTC_6x6_KHR:
			return sw::FORMAT_RGBA_ASTC_6x6_KHR;
		case GL_COMPRESSED_RGBA_ASTC_8x5_KHR:
			return sw::FORMAT_RGBA_ASTC_8x5_KHR;
		case GL_COMPRESSED_RGBA_ASTC_8x6_KHR:
			return sw::FORMAT_RGBA_ASTC_8x6_KHR;
		case GL_COMPRESSED_RGBA_ASTC_8x8_KHR:
			return sw::FORMAT_RGBA_ASTC_8x8_KHR;
		case GL_COMPRESSED_RGBA_ASTC_10x5_KHR:
			return sw::FORMAT_RGBA_ASTC_10x5_KHR;
		case GL_COMPRESSED_RGBA_ASTC_10x6_KHR:
			return sw::FORMAT_RGBA_ASTC_10x6_KHR;
		case GL_COMPRESSED_RGBA_ASTC_10x8_KHR:
			return sw::FORMAT_RGBA_ASTC_10x8_KHR;
		case GL_COMPRESSED_RGBA_ASTC_10x10_KHR:
			return sw::FORMAT_RGBA_ASTC_10x10_KHR;
		case GL_COMPRESSED_RGBA_ASTC_12x10_KHR:
			return sw::FORMAT_RGBA_ASTC_12x10_KHR;
		case GL_COMPRESSED_RGBA_ASTC_12x12_KHR:
			return sw::FORMAT_RGBA_ASTC_12x12_KHR;
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR:
			return sw::FORMAT_SRGB8_ALPHA8_ASTC_4x4_KHR;
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR:
			return sw::FORMAT_SRGB8_ALPHA8_ASTC_5x4_KHR;
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR:
			return sw::FORMAT_SRGB8_ALPHA8_ASTC_5x5_KHR;
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR:
			return sw::FORMAT_SRGB8_ALPHA8_ASTC_6x5_KHR;
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR:
			return sw::FORMAT_SRGB8_ALPHA8_ASTC_6x6_KHR;
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR:
			return sw::FORMAT_SRGB8_ALPHA8_ASTC_8x5_KHR;
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR:
			return sw::FORMAT_SRGB8_ALPHA8_ASTC_8x6_KHR;
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR:
			return sw::FORMAT_SRGB8_ALPHA8_ASTC_8x8_KHR;
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR:
			return sw::FORMAT_SRGB8_ALPHA8_ASTC_10x5_KHR;
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR:
			return sw::FORMAT_SRGB8_ALPHA8_ASTC_10x6_KHR;
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR:
			return sw::FORMAT_SRGB8_ALPHA8_ASTC_10x8_KHR;
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR:
			return sw::FORMAT_SRGB8_ALPHA8_ASTC_10x10_KHR;
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR:
			return sw::FORMAT_SRGB8_ALPHA8_ASTC_12x10_KHR;
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR:
			return sw::FORMAT_SRGB8_ALPHA8_ASTC_12x12_KHR;
		#if S3TC_SUPPORT
		case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
		case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
			return sw::FORMAT_DXT1;
		case GL_COMPRESSED_RGBA_S3TC_DXT3_ANGLE:
			return sw::FORMAT_DXT3;
		case GL_COMPRESSED_RGBA_S3TC_DXT5_ANGLE:
			return sw::FORMAT_DXT5;
		#endif
		default:
			break;
		}

		switch(type)
		{
		case GL_FLOAT:
			switch(format)
			{
			case GL_ALPHA:
			case GL_ALPHA32F_EXT:
				return sw::FORMAT_A32F;
			case GL_LUMINANCE:
			case GL_LUMINANCE32F_EXT:
				return sw::FORMAT_L32F;
			case GL_LUMINANCE_ALPHA:
			case GL_LUMINANCE_ALPHA32F_EXT:
				return sw::FORMAT_A32L32F;
			case GL_RED:
			case GL_R32F:
				return sw::FORMAT_R32F;
			case GL_RG:
			case GL_RG32F:
				return sw::FORMAT_G32R32F;
			case GL_RGB:
			case GL_RGB32F:
			case GL_RGBA:
			case GL_RGBA32F:
				return sw::FORMAT_A32B32G32R32F;
			case GL_DEPTH_COMPONENT:
			case GL_DEPTH_COMPONENT32F:
				return sw::FORMAT_D32F;
			default:
				UNREACHABLE(format);
			}
		case GL_HALF_FLOAT:
		case GL_HALF_FLOAT_OES:
			switch(format)
			{
			case GL_ALPHA:
			case GL_ALPHA16F_EXT:
				return sw::FORMAT_A16F;
			case GL_LUMINANCE:
			case GL_LUMINANCE16F_EXT:
				return sw::FORMAT_L16F;
			case GL_LUMINANCE_ALPHA:
			case GL_LUMINANCE_ALPHA16F_EXT:
				return sw::FORMAT_A16L16F;
			case GL_RED:
			case GL_R16F:
				return sw::FORMAT_R16F;
			case GL_RG:
			case GL_RG16F:
				return sw::FORMAT_G16R16F;
			case GL_RGB:
			case GL_RGB16F:
			case GL_RGBA:
			case GL_RGBA16F:
				return sw::FORMAT_A16B16G16R16F;
			default:
				UNREACHABLE(format);
			}
		case GL_BYTE:
			switch(format)
			{
			case GL_R8_SNORM:
			case GL_R8:
			case GL_RED:
				return sw::FORMAT_R8I_SNORM;
			case GL_R8I:
			case GL_RED_INTEGER:
				return sw::FORMAT_R8I;
			case GL_RG8_SNORM:
			case GL_RG8:
			case GL_RG:
				return sw::FORMAT_G8R8I_SNORM;
			case GL_RG8I:
			case GL_RG_INTEGER:
				return sw::FORMAT_G8R8I;
			case GL_RGB8_SNORM:
			case GL_RGB8:
			case GL_RGB:
				return sw::FORMAT_X8B8G8R8I_SNORM;
			case GL_RGB8I:
			case GL_RGB_INTEGER:
				return sw::FORMAT_X8B8G8R8I;
			case GL_RGBA8_SNORM:
			case GL_RGBA8:
			case GL_RGBA:
				return sw::FORMAT_A8B8G8R8I_SNORM;
			case GL_RGBA8I:
			case GL_RGBA_INTEGER:
				return sw::FORMAT_A8B8G8R8I;
			default:
				UNREACHABLE(format);
			}
		case GL_UNSIGNED_BYTE:
			switch(format)
			{
			case GL_LUMINANCE:
			case GL_LUMINANCE8_EXT:
				return sw::FORMAT_L8;
			case GL_LUMINANCE_ALPHA:
			case GL_LUMINANCE8_ALPHA8_EXT:
				return sw::FORMAT_A8L8;
			case GL_R8_SNORM:
			case GL_R8:
			case GL_RED:
				return sw::FORMAT_R8;
			case GL_R8UI:
			case GL_RED_INTEGER:
				return sw::FORMAT_R8UI;
			case GL_RG8_SNORM:
			case GL_RG8:
			case GL_RG:
				return sw::FORMAT_G8R8;
			case GL_RG8UI:
			case GL_RG_INTEGER:
				return sw::FORMAT_G8R8UI;
			case GL_RGB8_SNORM:
			case GL_RGB8:
			case GL_RGB:
			case GL_SRGB8:
				return sw::FORMAT_X8B8G8R8;
			case GL_RGB8UI:
			case GL_RGB_INTEGER:
				return sw::FORMAT_X8B8G8R8UI;
			case GL_RGBA8_SNORM:
			case GL_RGBA8:
			case GL_RGBA:
			case GL_SRGB8_ALPHA8:
				return sw::FORMAT_A8B8G8R8;
			case GL_RGBA8UI:
			case GL_RGBA_INTEGER:
				return sw::FORMAT_A8B8G8R8UI;
			case GL_BGRA_EXT:
			case GL_BGRA8_EXT:
				return sw::FORMAT_A8R8G8B8;
			case GL_ALPHA:
			case GL_ALPHA8_EXT:
				return sw::FORMAT_A8;
			case SW_YV12_BT601:
				return sw::FORMAT_YV12_BT601;
			case SW_YV12_BT709:
				return sw::FORMAT_YV12_BT709;
			case SW_YV12_JFIF:
				return sw::FORMAT_YV12_JFIF;
			default:
				UNREACHABLE(format);
			}
		case GL_SHORT:
			switch(format)
			{
			case GL_R16I:
			case GL_RED_INTEGER:
				return sw::FORMAT_R16I;
			case GL_RG16I:
			case GL_RG_INTEGER:
				return sw::FORMAT_G16R16I;
			case GL_RGB16I:
			case GL_RGB_INTEGER:
				return sw::FORMAT_X16B16G16R16I;
			case GL_RGBA16I:
			case GL_RGBA_INTEGER:
				return sw::FORMAT_A16B16G16R16I;
			default:
				UNREACHABLE(format);
			}
		case GL_UNSIGNED_SHORT:
			switch(format)
			{
			case GL_R16UI:
			case GL_RED_INTEGER:
				return sw::FORMAT_R16UI;
			case GL_RG16UI:
			case GL_RG_INTEGER:
				return sw::FORMAT_G16R16UI;
			case GL_RGB16UI:
			case GL_RGB_INTEGER:
				return sw::FORMAT_X16B16G16R16UI;
			case GL_RGBA16UI:
			case GL_RGBA_INTEGER:
				return sw::FORMAT_A16B16G16R16UI;
			case GL_DEPTH_COMPONENT:
			case GL_DEPTH_COMPONENT16:
				return sw::FORMAT_D32FS8_TEXTURE;
			default:
				UNREACHABLE(format);
			}
		case GL_INT:
			switch(format)
			{
			case GL_RED_INTEGER:
			case GL_R32I:
				return sw::FORMAT_R32I;
			case GL_RG_INTEGER:
			case GL_RG32I:
				return sw::FORMAT_G32R32I;
			case GL_RGB_INTEGER:
			case GL_RGB32I:
				return sw::FORMAT_X32B32G32R32I;
			case GL_RGBA_INTEGER:
			case GL_RGBA32I:
				return sw::FORMAT_A32B32G32R32I;
			default:
				UNREACHABLE(format);
			}
		case GL_UNSIGNED_INT:
			switch(format)
			{
			case GL_RED_INTEGER:
			case GL_R32UI:
				return sw::FORMAT_R32UI;
			case GL_RG_INTEGER:
			case GL_RG32UI:
				return sw::FORMAT_G32R32UI;
			case GL_RGB_INTEGER:
			case GL_RGB32UI:
				return sw::FORMAT_X32B32G32R32UI;
			case GL_RGBA_INTEGER:
			case GL_RGBA32UI:
				return sw::FORMAT_A32B32G32R32UI;
			case GL_DEPTH_COMPONENT:
			case GL_DEPTH_COMPONENT16:
			case GL_DEPTH_COMPONENT24:
			case GL_DEPTH_COMPONENT32_OES:
				return sw::FORMAT_D32FS8_TEXTURE;
			default:
				UNREACHABLE(format);
			}
		case GL_UNSIGNED_INT_24_8_OES:
			if(format == GL_DEPTH_STENCIL || format == GL_DEPTH24_STENCIL8)
			{
				return sw::FORMAT_D32FS8_TEXTURE;
			}
			else UNREACHABLE(format);
		case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
			if(format == GL_DEPTH_STENCIL || format == GL_DEPTH32F_STENCIL8)
			{
				return sw::FORMAT_D32FS8_TEXTURE;
			}
			else UNREACHABLE(format);
		case GL_UNSIGNED_SHORT_4_4_4_4:
			return sw::FORMAT_A8R8G8B8;
		case GL_UNSIGNED_SHORT_5_5_5_1:
			return sw::FORMAT_A8R8G8B8;
		case GL_UNSIGNED_SHORT_5_6_5:
			return sw::FORMAT_R5G6B5;
		case GL_UNSIGNED_INT_2_10_10_10_REV:
			if(format == GL_RGB10_A2UI)
			{
				return sw::FORMAT_A16B16G16R16UI;
			}
			else
			{
				return sw::FORMAT_A2B10G10R10;
			}
		case GL_UNSIGNED_INT_10F_11F_11F_REV:
		case GL_UNSIGNED_INT_5_9_9_9_REV:
			return sw::FORMAT_A32B32G32R32F;
		default:
			UNREACHABLE(type);
		}

		return sw::FORMAT_NULL;
	}

	// Returns the size, in bytes, of a single texel in an Image
	static int ComputePixelSize(GLenum format, GLenum type)
	{
		switch(type)
		{
		case GL_BYTE:
			switch(format)
			{
			case GL_R8:
			case GL_R8I:
			case GL_R8_SNORM:
			case GL_RED:             return sizeof(char);
			case GL_RED_INTEGER:     return sizeof(char);
			case GL_RG8:
			case GL_RG8I:
			case GL_RG8_SNORM:
			case GL_RG:              return sizeof(char) * 2;
			case GL_RG_INTEGER:      return sizeof(char) * 2;
			case GL_RGB8:
			case GL_RGB8I:
			case GL_RGB8_SNORM:
			case GL_RGB:             return sizeof(char) * 3;
			case GL_RGB_INTEGER:     return sizeof(char) * 3;
			case GL_RGBA8:
			case GL_RGBA8I:
			case GL_RGBA8_SNORM:
			case GL_RGBA:            return sizeof(char) * 4;
			case GL_RGBA_INTEGER:    return sizeof(char) * 4;
			default: UNREACHABLE(format);
			}
			break;
		case GL_UNSIGNED_BYTE:
			switch(format)
			{
			case GL_R8:
			case GL_R8UI:
			case GL_RED:             return sizeof(unsigned char);
			case GL_RED_INTEGER:     return sizeof(unsigned char);
			case GL_ALPHA8_EXT:
			case GL_ALPHA:           return sizeof(unsigned char);
			case GL_LUMINANCE8_EXT:
			case GL_LUMINANCE:       return sizeof(unsigned char);
			case GL_LUMINANCE8_ALPHA8_EXT:
			case GL_LUMINANCE_ALPHA: return sizeof(unsigned char) * 2;
			case GL_RG8:
			case GL_RG8UI:
			case GL_RG:              return sizeof(unsigned char) * 2;
			case GL_RG_INTEGER:      return sizeof(unsigned char) * 2;
			case GL_RGB8:
			case GL_RGB8UI:
			case GL_SRGB8:
			case GL_RGB:             return sizeof(unsigned char) * 3;
			case GL_RGB_INTEGER:     return sizeof(unsigned char) * 3;
			case GL_RGBA8:
			case GL_RGBA8UI:
			case GL_SRGB8_ALPHA8:
			case GL_RGBA:            return sizeof(unsigned char) * 4;
			case GL_RGBA_INTEGER:    return sizeof(unsigned char) * 4;
			case GL_BGRA_EXT:
			case GL_BGRA8_EXT:       return sizeof(unsigned char)* 4;
			default: UNREACHABLE(format);
			}
			break;
		case GL_SHORT:
			switch(format)
			{
			case GL_R16I:
			case GL_RED_INTEGER:     return sizeof(short);
			case GL_RG16I:
			case GL_RG_INTEGER:      return sizeof(short) * 2;
			case GL_RGB16I:
			case GL_RGB_INTEGER:     return sizeof(short) * 3;
			case GL_RGBA16I:
			case GL_RGBA_INTEGER:    return sizeof(short) * 4;
			default: UNREACHABLE(format);
			}
			break;
		case GL_UNSIGNED_SHORT:
			switch(format)
			{
			case GL_DEPTH_COMPONENT16:
			case GL_DEPTH_COMPONENT: return sizeof(unsigned short);
			case GL_R16UI:
			case GL_RED_INTEGER:     return sizeof(unsigned short);
			case GL_RG16UI:
			case GL_RG_INTEGER:      return sizeof(unsigned short) * 2;
			case GL_RGB16UI:
			case GL_RGB_INTEGER:     return sizeof(unsigned short) * 3;
			case GL_RGBA16UI:
			case GL_RGBA_INTEGER:    return sizeof(unsigned short) * 4;
			default: UNREACHABLE(format);
			}
			break;
		case GL_INT:
			switch(format)
			{
			case GL_R32I:
			case GL_RED_INTEGER:     return sizeof(int);
			case GL_RG32I:
			case GL_RG_INTEGER:      return sizeof(int) * 2;
			case GL_RGB32I:
			case GL_RGB_INTEGER:     return sizeof(int) * 3;
			case GL_RGBA32I:
			case GL_RGBA_INTEGER:    return sizeof(int) * 4;
			default: UNREACHABLE(format);
			}
			break;
		case GL_UNSIGNED_INT:
			switch(format)
			{
			case GL_DEPTH_COMPONENT16:
			case GL_DEPTH_COMPONENT24:
			case GL_DEPTH_COMPONENT32_OES:
			case GL_DEPTH_COMPONENT: return sizeof(unsigned int);
			case GL_R32UI:
			case GL_RED_INTEGER:     return sizeof(unsigned int);
			case GL_RG32UI:
			case GL_RG_INTEGER:      return sizeof(unsigned int) * 2;
			case GL_RGB32UI:
			case GL_RGB_INTEGER:     return sizeof(unsigned int) * 3;
			case GL_RGBA32UI:
			case GL_RGBA_INTEGER:    return sizeof(unsigned int) * 4;
			default: UNREACHABLE(format);
			}
			break;
		case GL_UNSIGNED_SHORT_4_4_4_4:
		case GL_UNSIGNED_SHORT_5_5_5_1:
		case GL_UNSIGNED_SHORT_5_6_5:
			return sizeof(unsigned short);
		case GL_UNSIGNED_INT_10F_11F_11F_REV:
		case GL_UNSIGNED_INT_5_9_9_9_REV:
		case GL_UNSIGNED_INT_2_10_10_10_REV:
		case GL_UNSIGNED_INT_24_8_OES:
			return sizeof(unsigned int);
		case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
			return sizeof(float) + sizeof(unsigned int);
		case GL_FLOAT:
			switch(format)
			{
			case GL_DEPTH_COMPONENT32F:
			case GL_DEPTH_COMPONENT: return sizeof(float);
			case GL_ALPHA32F_EXT:
			case GL_ALPHA:           return sizeof(float);
			case GL_LUMINANCE32F_EXT:
			case GL_LUMINANCE:       return sizeof(float);
			case GL_LUMINANCE_ALPHA32F_EXT:
			case GL_LUMINANCE_ALPHA: return sizeof(float) * 2;
			case GL_RED:             return sizeof(float);
			case GL_R32F:            return sizeof(float);
			case GL_RG:              return sizeof(float) * 2;
			case GL_RG32F:           return sizeof(float) * 2;
			case GL_RGB:             return sizeof(float) * 3;
			case GL_RGB32F:          return sizeof(float) * 3;
			case GL_RGBA:            return sizeof(float) * 4;
			case GL_RGBA32F:         return sizeof(float) * 4;
			default: UNREACHABLE(format);
			}
			break;
		case GL_HALF_FLOAT:
		case GL_HALF_FLOAT_OES:
			switch(format)
			{
			case GL_ALPHA16F_EXT:
			case GL_ALPHA:           return sizeof(unsigned short);
			case GL_LUMINANCE16F_EXT:
			case GL_LUMINANCE:       return sizeof(unsigned short);
			case GL_LUMINANCE_ALPHA16F_EXT:
			case GL_LUMINANCE_ALPHA: return sizeof(unsigned short) * 2;
			case GL_RED:             return sizeof(unsigned short);
			case GL_R16F:            return sizeof(unsigned short);
			case GL_RG:              return sizeof(unsigned short) * 2;
			case GL_RG16F:           return sizeof(unsigned short) * 2;
			case GL_RGB:             return sizeof(unsigned short) * 3;
			case GL_RGB16F:          return sizeof(unsigned short) * 3;
			case GL_RGBA:            return sizeof(unsigned short) * 4;
			case GL_RGBA16F:         return sizeof(unsigned short) * 4;
			default: UNREACHABLE(format);
			}
			break;
		default: UNREACHABLE(type);
		}

		return 0;
	}

	GLsizei ComputePitch(GLsizei width, GLenum format, GLenum type, GLint alignment)
	{
		ASSERT(alignment > 0 && sw::isPow2(alignment));

		GLsizei rawPitch = ComputePixelSize(format, type) * width;
		return (rawPitch + alignment - 1) & ~(alignment - 1);
	}

	inline int GetNumBlocks(int w, int h, int blockSizeX, int blockSizeY)
	{
		return ((w + blockSizeX - 1) / blockSizeX) * ((h + blockSizeY - 1) / blockSizeY);
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
			return 8 * GetNumBlocks(width, height, 4, 4);
		case GL_COMPRESSED_RGBA_S3TC_DXT3_ANGLE:
		case GL_COMPRESSED_RGBA_S3TC_DXT5_ANGLE:
		case GL_COMPRESSED_RG11_EAC:
		case GL_COMPRESSED_SIGNED_RG11_EAC:
		case GL_COMPRESSED_RGBA8_ETC2_EAC:
		case GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:
		case GL_COMPRESSED_RGBA_ASTC_4x4_KHR:
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR:
			return 16 * GetNumBlocks(width, height, 4, 4);
		case GL_COMPRESSED_RGBA_ASTC_5x4_KHR:
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR:
			return 16 * GetNumBlocks(width, height, 5, 4);
		case GL_COMPRESSED_RGBA_ASTC_5x5_KHR:
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR:
			return 16 * GetNumBlocks(width, height, 5, 5);
		case GL_COMPRESSED_RGBA_ASTC_6x5_KHR:
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR:
			return 16 * GetNumBlocks(width, height, 6, 5);
		case GL_COMPRESSED_RGBA_ASTC_6x6_KHR:
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR:
			return 16 * GetNumBlocks(width, height, 6, 6);
		case GL_COMPRESSED_RGBA_ASTC_8x5_KHR:
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR:
			return 16 * GetNumBlocks(width, height, 8, 5);
		case GL_COMPRESSED_RGBA_ASTC_8x6_KHR:
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR:
			return 16 * GetNumBlocks(width, height, 8, 6);
		case GL_COMPRESSED_RGBA_ASTC_8x8_KHR:
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR:
			return 16 * GetNumBlocks(width, height, 8, 8);
		case GL_COMPRESSED_RGBA_ASTC_10x5_KHR:
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR:
			return 16 * GetNumBlocks(width, height, 10, 5);
		case GL_COMPRESSED_RGBA_ASTC_10x6_KHR:
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR:
			return 16 * GetNumBlocks(width, height, 10, 6);
		case GL_COMPRESSED_RGBA_ASTC_10x8_KHR:
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR:
			return 16 * GetNumBlocks(width, height, 10, 8);
		case GL_COMPRESSED_RGBA_ASTC_10x10_KHR:
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR:
			return 16 * GetNumBlocks(width, height, 10, 10);
		case GL_COMPRESSED_RGBA_ASTC_12x10_KHR:
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR:
			return 16 * GetNumBlocks(width, height, 12, 10);
		case GL_COMPRESSED_RGBA_ASTC_12x12_KHR:
		case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR:
			return 16 * GetNumBlocks(width, height, 12, 12);
		default:
			return 0;
		}
	}

	Image::~Image()
	{
		ASSERT(referenceCount == 0);
	}

	void Image::addRef()
	{
		if(parentTexture)
		{
			return parentTexture->addRef();
		}
		int newCount = sw::atomicIncrement(&referenceCount);
		LOGLOCK("%s image=%p referenceCount=%d", __FUNCTION__, this, newCount);
	}

	void Image::release()
	{
		if(parentTexture)
		{
			return parentTexture->release();
		}

		int newCount = sw::atomicDecrement(&referenceCount);
		LOGLOCK("%s image=%p referenceCount=%d", __FUNCTION__, this, newCount);
		if (newCount == 0)
		{
			ASSERT(!shared);   // Should still hold a reference if eglDestroyImage hasn't been called
			delete this;
		}
	}

	void Image::unbind(const egl::Texture *parent)
	{
		if(parentTexture == parent)
		{
			parentTexture = 0;
		}

		release();
	}

	void Image::loadImageData(GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const UnpackInfo& unpackInfo, const void *input)
	{
		ASSERT(false);   // Only TextureImage should have image loading 
	}

	void Image::loadCompressedData(GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLsizei imageSize, const void *pixels)
	{
		ASSERT(false);
	}
}
