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

#include "SamplerCore.hpp"

#include "Constants.hpp"
#include "Debug.hpp"

namespace
{
	void applySwizzle(sw::SwizzleType swizzle, sw::Short4& s, const sw::Vector4s& c)
	{
		switch(swizzle)
		{
		case sw::SWIZZLE_RED:	s = c.x; break;
		case sw::SWIZZLE_GREEN: s = c.y; break;
		case sw::SWIZZLE_BLUE:  s = c.z; break;
		case sw::SWIZZLE_ALPHA: s = c.w; break;
		case sw::SWIZZLE_ZERO:  s = sw::Short4(0x0000); break;
		case sw::SWIZZLE_ONE:   s = sw::Short4(0x1000); break;
		default: ASSERT(false);
		}
	}

	void applySwizzle(sw::SwizzleType swizzle, sw::Float4& f, const sw::Vector4f& c)
	{
		switch(swizzle)
		{
		case sw::SWIZZLE_RED:	f = c.x; break;
		case sw::SWIZZLE_GREEN: f = c.y; break;
		case sw::SWIZZLE_BLUE:  f = c.z; break;
		case sw::SWIZZLE_ALPHA: f = c.w; break;
		case sw::SWIZZLE_ZERO:  f = sw::Float4(0.0f, 0.0f, 0.0f, 0.0f); break;
		case sw::SWIZZLE_ONE:   f = sw::Float4(1.0f, 1.0f, 1.0f, 1.0f); break;
		default: ASSERT(false);
		}
	}
}

namespace sw
{
	SamplerCore::SamplerCore(Pointer<Byte> &constants, const Sampler::State &state) : constants(constants), state(state)
	{
	}

	void SamplerCore::sampleTexture(Pointer<Byte> &texture, Vector4s &c, Float4 &u, Float4 &v, Float4 &w, Float4 &q, Vector4f &dsx, Vector4f &dsy)
	{
		sampleTexture(texture, c, u, v, w, q, dsx, dsy, dsx, Implicit, true);
	}

	void SamplerCore::sampleTexture(Pointer<Byte> &texture, Vector4s &c, Float4 &u, Float4 &v, Float4 &w, Float4 &q, Vector4f &dsx, Vector4f &dsy, Vector4f &offset, SamplerFunction function, bool fixed12)
	{
		#if PERF_PROFILE
			AddAtomic(Pointer<Long>(&profiler.texOperations), 4);

			if(state.compressedFormat)
			{
				AddAtomic(Pointer<Long>(&profiler.compressedTex), 4);
			}
		#endif

		Float4 uuuu = u;
		Float4 vvvv = v;
		Float4 wwww = w;

		if(state.textureType == TEXTURE_NULL)
		{
			c.x = Short4(0x0000);
			c.y = Short4(0x0000);
			c.z = Short4(0x0000);

			if(fixed12)   // FIXME: Convert to fixed12 at higher level, when required
			{
				c.w = Short4(0x1000);
			}
			else
			{
				c.w = Short4(0xFFFFu);   // FIXME
			}
		}
		else
		{
			Int face[4];
			Float4 lodX;
			Float4 lodY;
			Float4 lodZ;

			if(state.textureType == TEXTURE_CUBE)
			{
				cubeFace(face, uuuu, vvvv, lodX, lodY, lodZ, u, v, w);
			}

			Float lod;
			Float anisotropy;
			Float4 uDelta;
			Float4 vDelta;
			Float lodBias = (function == Fetch) ? Float4(As<Int4>(q)).x : q.x;

			if(state.textureType != TEXTURE_3D)
			{
				if(state.textureType != TEXTURE_CUBE)
				{
					computeLod(texture, lod, anisotropy, uDelta, vDelta, uuuu, vvvv, lodBias, dsx, dsy, function);
				}
				else
				{
					computeLodCube(texture, lod, lodX, lodY, lodZ, lodBias, dsx, dsy, function);
				}
			}
			else
			{
				computeLod3D(texture, lod, uuuu, vvvv, wwww, lodBias, dsx, dsy, function);
			}

			if(!hasFloatTexture())
			{
				sampleFilter(texture, c, uuuu, vvvv, wwww, offset, lod, anisotropy, uDelta, vDelta, face, function);
			}
			else
			{
				Vector4f cf;

				sampleFloatFilter(texture, cf, uuuu, vvvv, wwww, offset, lod, anisotropy, uDelta, vDelta, face, function);

				convertFixed12(c, cf);
			}

			if(fixed12 && !hasFloatTexture())
			{
				if(has16bitTextureFormat())
				{
					switch(state.textureFormat)
					{
					case FORMAT_R5G6B5:
						if(state.sRGB)
						{
							sRGBtoLinear16_5_12(c.x);
							sRGBtoLinear16_6_12(c.y);
							sRGBtoLinear16_5_12(c.z);
						}
						else
						{
							c.x = MulHigh(As<UShort4>(c.x), UShort4(0x10000000 / 0xF800));
							c.y = MulHigh(As<UShort4>(c.y), UShort4(0x10000000 / 0xFC00));
							c.z = MulHigh(As<UShort4>(c.z), UShort4(0x10000000 / 0xF800));
						}
						break;
					default:
						ASSERT(false);
					}
				}
				else
				{
					for(int component = 0; component < textureComponentCount(); component++)
					{
						if(state.sRGB && isRGBComponent(component))
						{
							sRGBtoLinear16_8_12(c[component]);   // FIXME: Perform linearization at surface level for read-only textures
						}
						else
						{
							if(hasUnsignedTextureComponent(component))
							{
								c[component] = As<UShort4>(c[component]) >> 4;
							}
							else
							{
								c[component] = c[component] >> 3;
							}
						}
					}
				}
			}

			if(fixed12 && state.textureFilter != FILTER_GATHER)
			{
				int componentCount = textureComponentCount();

				switch(state.textureFormat)
				{
				case FORMAT_R8I_SNORM:
				case FORMAT_G8R8I_SNORM:
				case FORMAT_X8B8G8R8I_SNORM:
				case FORMAT_A8B8G8R8I_SNORM:
				case FORMAT_R8:
				case FORMAT_R5G6B5:
				case FORMAT_G8R8:
				case FORMAT_R8I:
				case FORMAT_R8UI:
				case FORMAT_G8R8I:
				case FORMAT_G8R8UI:
				case FORMAT_X8B8G8R8I:
				case FORMAT_X8B8G8R8UI:
				case FORMAT_A8B8G8R8I:
				case FORMAT_A8B8G8R8UI:
				case FORMAT_R16I:
				case FORMAT_R16UI:
				case FORMAT_G16R16:
				case FORMAT_G16R16I:
				case FORMAT_G16R16UI:
				case FORMAT_X16B16G16R16I:
				case FORMAT_X16B16G16R16UI:
				case FORMAT_A16B16G16R16:
				case FORMAT_A16B16G16R16I:
				case FORMAT_A16B16G16R16UI:
				case FORMAT_R32I:
				case FORMAT_R32UI:
				case FORMAT_G32R32I:
				case FORMAT_G32R32UI:
				case FORMAT_X32B32G32R32I:
				case FORMAT_X32B32G32R32UI:
				case FORMAT_A32B32G32R32I:
				case FORMAT_A32B32G32R32UI:
				case FORMAT_X8R8G8B8:
				case FORMAT_X8B8G8R8:
				case FORMAT_A8R8G8B8:
				case FORMAT_A8B8G8R8:
				case FORMAT_SRGB8_X8:
				case FORMAT_SRGB8_A8:
				case FORMAT_V8U8:
				case FORMAT_Q8W8V8U8:
				case FORMAT_X8L8V8U8:
				case FORMAT_V16U16:
				case FORMAT_A16W16V16U16:
				case FORMAT_Q16W16V16U16:
				case FORMAT_YV12_BT601:
				case FORMAT_YV12_BT709:
				case FORMAT_YV12_JFIF:
					if(componentCount < 2) c.y = Short4(0x1000);
					if(componentCount < 3) c.z = Short4(0x1000);
					if(componentCount < 4) c.w = Short4(0x1000);
					break;
				case FORMAT_A8:
					c.w = c.x;
					c.x = Short4(0x0000);
					c.y = Short4(0x0000);
					c.z = Short4(0x0000);
					break;
				case FORMAT_L8:
				case FORMAT_L16:
					c.y = c.x;
					c.z = c.x;
					c.w = Short4(0x1000);
					break;
				case FORMAT_A8L8:
					c.w = c.y;
					c.y = c.x;
					c.z = c.x;
					break;
				case FORMAT_R32F:
					c.y = Short4(0x1000);
				case FORMAT_G32R32F:
					c.z = Short4(0x1000);
				case FORMAT_X32B32G32R32F:
					c.w = Short4(0x1000);
				case FORMAT_A32B32G32R32F:
					break;
				case FORMAT_D32F:
				case FORMAT_D32F_LOCKABLE:
				case FORMAT_D32FS8_TEXTURE:
				case FORMAT_D32FS8_SHADOW:
					c.y = c.x;
					c.z = c.x;
					c.w = c.x;
					break;
				default:
					ASSERT(false);
				}
			}
		}

		if(fixed12 &&
		   ((state.swizzleR != SWIZZLE_RED) ||
		    (state.swizzleG != SWIZZLE_GREEN) ||
		    (state.swizzleB != SWIZZLE_BLUE) ||
		    (state.swizzleA != SWIZZLE_ALPHA)))
		{
			const Vector4s col(c);
			applySwizzle(state.swizzleR, c.x, col);
			applySwizzle(state.swizzleG, c.y, col);
			applySwizzle(state.swizzleB, c.z, col);
			applySwizzle(state.swizzleA, c.w, col);
		}
	}

	void SamplerCore::sampleTexture(Pointer<Byte> &texture, Vector4f &c, Float4 &u, Float4 &v, Float4 &w, Float4 &q, Vector4f &dsx, Vector4f &dsy, Vector4f &offset, SamplerFunction function)
	{
		#if PERF_PROFILE
			AddAtomic(Pointer<Long>(&profiler.texOperations), 4);

			if(state.compressedFormat)
			{
				AddAtomic(Pointer<Long>(&profiler.compressedTex), 4);
			}
		#endif

		if(state.textureType == TEXTURE_NULL)
		{
			c.x = Float4(0.0f);
			c.y = Float4(0.0f);
			c.z = Float4(0.0f);
			c.w = Float4(1.0f);
		}
		else
		{
			if(hasFloatTexture())   // FIXME: Mostly identical to integer sampling
			{
				Float4 uuuu = u;
				Float4 vvvv = v;
				Float4 wwww = w;

				Int face[4];
				Float4 lodX;
				Float4 lodY;
				Float4 lodZ;

				if(state.textureType == TEXTURE_CUBE)
				{
					cubeFace(face, uuuu, vvvv, lodX, lodY, lodZ, u, v, w);
				}

				Float lod;
				Float anisotropy;
				Float4 uDelta;
				Float4 vDelta;
				Float lodBias = (function == Fetch) ? Float4(As<Int4>(q)).x : q.x;

				if(state.textureType != TEXTURE_3D)
				{
					if(state.textureType != TEXTURE_CUBE)
					{
						computeLod(texture, lod, anisotropy, uDelta, vDelta, uuuu, vvvv, lodBias, dsx, dsy, function);
					}
					else
					{
						computeLodCube(texture, lod, lodX, lodY, lodZ, lodBias, dsx, dsy, function);
					}
				}
				else
				{
					computeLod3D(texture, lod, uuuu, vvvv, wwww, lodBias, dsx, dsy, function);
				}

				sampleFloatFilter(texture, c, uuuu, vvvv, wwww, offset, lod, anisotropy, uDelta, vDelta, face, function);
			}
			else
			{
				Vector4s cs;

				sampleTexture(texture, cs, u, v, w, q, dsx, dsy, offset, function, false);

				for(int component = 0; component < textureComponentCount(); component++)
				{
					if(has16bitTextureFormat())
					{
						switch(state.textureFormat)
						{
						case FORMAT_R5G6B5:
							if(state.sRGB)
							{
								sRGBtoLinear16_5_12(cs.x);
								sRGBtoLinear16_6_12(cs.y);
								sRGBtoLinear16_5_12(cs.z);

								convertSigned12(c.x, cs.x);
								convertSigned12(c.y, cs.y);
								convertSigned12(c.z, cs.z);
							}
							else
							{
								c.x = Float4(As<UShort4>(cs.x)) * Float4(1.0f / 0xF800);
								c.y = Float4(As<UShort4>(cs.y)) * Float4(1.0f / 0xFC00);
								c.z = Float4(As<UShort4>(cs.z)) * Float4(1.0f / 0xF800);
							}
							break;
						default:
							ASSERT(false);
						}
					}
					else
					{
						switch(state.textureFormat)
						{
						case FORMAT_R8I:
						case FORMAT_G8R8I:
						case FORMAT_X8B8G8R8I:
						case FORMAT_A8B8G8R8I:
							c[component] = As<Float4>(Int4(cs[component]) >> 8);
							break;
						case FORMAT_R8UI:
						case FORMAT_G8R8UI:
						case FORMAT_X8B8G8R8UI:
						case FORMAT_A8B8G8R8UI:
							c[component] = As<Float4>(Int4(As<UShort4>(cs[component]) >> 8));
							break;
						case FORMAT_R16I:
						case FORMAT_G16R16I:
						case FORMAT_X16B16G16R16I:
						case FORMAT_A16B16G16R16I:
							c[component] = As<Float4>(Int4(cs[component]));
							break;
						case FORMAT_R16UI:
						case FORMAT_G16R16UI:
						case FORMAT_X16B16G16R16UI:
						case FORMAT_A16B16G16R16UI:
							c[component] = As<Float4>(Int4(As<UShort4>(cs[component])));
							break;
						default:
							// Normalized integer formats
							if(state.sRGB && isRGBComponent(component))
							{
								sRGBtoLinear16_8_12(cs[component]);   // FIXME: Perform linearization at surface level for read-only textures
								convertSigned12(c[component], cs[component]);
							}
							else
							{
								if(hasUnsignedTextureComponent(component))
								{
									convertUnsigned16(c[component], cs[component]);
								}
								else
								{
									convertSigned15(c[component], cs[component]);
								}
							}
							break;
						}
					}
				}
			}

			int componentCount = textureComponentCount();

			if(state.textureFilter != FILTER_GATHER)
			{
				switch(state.textureFormat)
				{
				case FORMAT_R8I:
				case FORMAT_R8UI:
				case FORMAT_R16I:
				case FORMAT_R16UI:
				case FORMAT_R32I:
				case FORMAT_R32UI:
					c.y = As<Float4>(UInt4(0));
				case FORMAT_G8R8I:
				case FORMAT_G8R8UI:
				case FORMAT_G16R16I:
				case FORMAT_G16R16UI:
				case FORMAT_G32R32I:
				case FORMAT_G32R32UI:
					c.z = As<Float4>(UInt4(0));
				case FORMAT_X8B8G8R8I:
				case FORMAT_X8B8G8R8UI:
				case FORMAT_X16B16G16R16I:
				case FORMAT_X16B16G16R16UI:
				case FORMAT_X32B32G32R32I:
				case FORMAT_X32B32G32R32UI:
					c.w = As<Float4>(UInt4(1));
				case FORMAT_A8B8G8R8I:
				case FORMAT_A8B8G8R8UI:
				case FORMAT_A16B16G16R16I:
				case FORMAT_A16B16G16R16UI:
				case FORMAT_A32B32G32R32I:
				case FORMAT_A32B32G32R32UI:
					break;
				case FORMAT_R8I_SNORM:
				case FORMAT_G8R8I_SNORM:
				case FORMAT_X8B8G8R8I_SNORM:
				case FORMAT_A8B8G8R8I_SNORM:
				case FORMAT_R8:
				case FORMAT_R5G6B5:
				case FORMAT_G8R8:
				case FORMAT_G16R16:
				case FORMAT_A16B16G16R16:
				case FORMAT_X8R8G8B8:
				case FORMAT_X8B8G8R8:
				case FORMAT_A8R8G8B8:
				case FORMAT_A8B8G8R8:
				case FORMAT_SRGB8_X8:
				case FORMAT_SRGB8_A8:
				case FORMAT_V8U8:
				case FORMAT_Q8W8V8U8:
				case FORMAT_X8L8V8U8:
				case FORMAT_V16U16:
				case FORMAT_A16W16V16U16:
				case FORMAT_Q16W16V16U16:
				case FORMAT_YV12_BT601:
				case FORMAT_YV12_BT709:
				case FORMAT_YV12_JFIF:
					if(componentCount < 2) c.y = Float4(1.0f);
					if(componentCount < 3) c.z = Float4(1.0f);
					if(componentCount < 4) c.w = Float4(1.0f);
					break;
				case FORMAT_A8:
					c.w = c.x;
					c.x = Float4(0.0f);
					c.y = Float4(0.0f);
					c.z = Float4(0.0f);
					break;
				case FORMAT_L8:
				case FORMAT_L16:
					c.y = c.x;
					c.z = c.x;
					c.w = Float4(1.0f);
					break;
				case FORMAT_A8L8:
					c.w = c.y;
					c.y = c.x;
					c.z = c.x;
					break;
				case FORMAT_R32F:
					c.y = Float4(1.0f);
				case FORMAT_G32R32F:
					c.z = Float4(1.0f);
				case FORMAT_X32B32G32R32F:
					c.w = Float4(1.0f);
				case FORMAT_A32B32G32R32F:
					break;
				case FORMAT_D32F:
				case FORMAT_D32F_LOCKABLE:
				case FORMAT_D32FS8_TEXTURE:
				case FORMAT_D32FS8_SHADOW:
					c.y = c.x;
					c.z = c.x;
					c.w = c.x;
					break;
				default:
					ASSERT(false);
				}
			}
		}

		if((state.swizzleR != SWIZZLE_RED) ||
		   (state.swizzleG != SWIZZLE_GREEN) ||
		   (state.swizzleB != SWIZZLE_BLUE) ||
		   (state.swizzleA != SWIZZLE_ALPHA))
		{
			const Vector4f col(c);
			applySwizzle(state.swizzleR, c.x, col);
			applySwizzle(state.swizzleG, c.y, col);
			applySwizzle(state.swizzleB, c.z, col);
			applySwizzle(state.swizzleA, c.w, col);
		}
	}

	void SamplerCore::textureSize(Pointer<Byte> &texture, Vector4f &size, Float4 &lod)
	{
		for(int i = 0; i < 4; ++i)
		{
			Int baseLevel = *Pointer<Int>(texture + OFFSET(Texture, baseLevel));
			Pointer<Byte> mipmap = texture + OFFSET(Texture, mipmap) + (As<Int>(Extract(lod, i)) + baseLevel) * sizeof(Mipmap);
			size.x = Insert(size.x, As<Float>(*Pointer<Int>(mipmap + OFFSET(Mipmap, width))), i);
			size.y = Insert(size.y, As<Float>(*Pointer<Int>(mipmap + OFFSET(Mipmap, height))), i);
			size.z = Insert(size.z, As<Float>(*Pointer<Int>(mipmap + OFFSET(Mipmap, depth))), i);
		}
	}

	void SamplerCore::border(Short4 &mask, Float4 &coordinates)
	{
		Int4 border = As<Int4>(CmpLT(Abs(coordinates - Float4(0.5f)), Float4(0.5f)));
		mask = As<Short4>(Int2(As<Int4>(Pack(border, border))));
	}

	void SamplerCore::border(Int4 &mask, Float4 &coordinates)
	{
		mask = As<Int4>(CmpLT(Abs(coordinates - Float4(0.5f)), Float4(0.5f)));
	}

	void SamplerCore::sampleFilter(Pointer<Byte> &texture, Vector4s &c, Float4 &u, Float4 &v, Float4 &w, Vector4f &offset, Float &lod, Float &anisotropy, Float4 &uDelta, Float4 &vDelta, Int face[4], SamplerFunction function)
	{
		sampleAniso(texture, c, u, v, w, offset, lod, anisotropy, uDelta, vDelta, face, false, function);

		if(function == Fetch)
		{
			return;
		}

		if(state.mipmapFilter > MIPMAP_POINT)
		{
			Vector4s cc;

			sampleAniso(texture, cc, u, v, w, offset, lod, anisotropy, uDelta, vDelta, face, true, function);

			lod *= Float(1 << 16);

			UShort4 utri = UShort4(Float4(lod));   // FIXME: Optimize
			Short4 stri = utri >> 1;   // FIXME: Optimize
			UShort4 utri2 = ~utri;
			Short4 stri2 = Short4(0x7FFF) - stri;

			int componentCount = textureComponentCount();
			for(int n = 0; n < 4; ++n)
			{
				if(componentCount > n)
				{
					c[n] = hasUnsignedTextureComponent(n) ?
					       As<Short4>(MulHigh(As<UShort4>(cc[n]), utri) + MulHigh(As<UShort4>(c[n]), utri2)) :
					       (MulHigh(cc[n], stri) + MulHigh(c[n], stri2)) << 1;
				}
			}
		}

		Short4 borderMask;

		if(state.addressingModeU == ADDRESSING_BORDER)
		{
			Short4 u0;

			border(u0, u);

			borderMask = u0;
		}

		if(state.addressingModeV == ADDRESSING_BORDER)
		{
			Short4 v0;

			border(v0, v);

			if(state.addressingModeU == ADDRESSING_BORDER)
			{
				borderMask &= v0;
			}
			else
			{
				borderMask = v0;
			}
		}

		if(state.addressingModeW == ADDRESSING_BORDER && state.textureType == TEXTURE_3D)
		{
			Short4 s0;

			border(s0, w);

			if(state.addressingModeU == ADDRESSING_BORDER ||
			   state.addressingModeV == ADDRESSING_BORDER)
			{
				borderMask &= s0;
			}
			else
			{
				borderMask = s0;
			}
		}

		if(state.addressingModeU == ADDRESSING_BORDER ||
		   state.addressingModeV == ADDRESSING_BORDER ||
		   (state.addressingModeW == ADDRESSING_BORDER && state.textureType == TEXTURE_3D))
		{
			for(int n = 0; n < 4; ++n)
			{
				c[n] = (borderMask & c[n]) | (~borderMask & (*Pointer<Short4>(texture + OFFSET(Texture, borderColor4[n])) >> (hasUnsignedTextureComponent(n) ? 0 : 1)));
			}
		}
	}

	void SamplerCore::sampleAniso(Pointer<Byte> &texture, Vector4s &c, Float4 &u, Float4 &v, Float4 &w, Vector4f &offset, Float &lod, Float &anisotropy, Float4 &uDelta, Float4 &vDelta, Int face[4], bool secondLOD, SamplerFunction function)
	{
		if(state.textureFilter != FILTER_ANISOTROPIC || function == Lod || function == Fetch)
		{
			sampleQuad(texture, c, u, v, w, offset, lod, face, secondLOD, function);
		}
		else
		{
			Int a = RoundInt(anisotropy);

			Vector4s cSum;

			cSum.x = Short4(0);
			cSum.y = Short4(0);
			cSum.z = Short4(0);
			cSum.w = Short4(0);

			Float4 A = *Pointer<Float4>(constants + OFFSET(Constants,uvWeight) + 16 * a);
			Float4 B = *Pointer<Float4>(constants + OFFSET(Constants,uvStart) + 16 * a);
			UShort4 cw = *Pointer<UShort4>(constants + OFFSET(Constants,cWeight) + 8 * a);
			Short4 sw = Short4(cw >> 1);

			Float4 du = uDelta;
			Float4 dv = vDelta;

			Float4 u0 = u + B * du;
			Float4 v0 = v + B * dv;

			du *= A;
			dv *= A;

			Int i = 0;

			Do
			{
				sampleQuad(texture, c, u0, v0, w, offset, lod, face, secondLOD, function);

				u0 += du;
				v0 += dv;

				for(int n = 0; n < 4; ++n)
				{
					cSum[n] += hasUnsignedTextureComponent(n) ? As<Short4>(MulHigh(As<UShort4>(c[n]), cw)) : MulHigh(c[n], sw);
				}

				i++;
			}
			Until(i >= a)

			for(int n = 0; n < 4; ++n)
			{
				c[n] = hasUnsignedTextureComponent(n) ? cSum[n] : AddSat(cSum[n], cSum[n]);
			}
		}
	}

	void SamplerCore::sampleQuad(Pointer<Byte> &texture, Vector4s &c, Float4 &u, Float4 &v, Float4 &w, Vector4f &offset, Float &lod, Int face[4], bool secondLOD, SamplerFunction function)
	{
		if(state.textureType != TEXTURE_3D)
		{
			sampleQuad2D(texture, c, u, v, w, offset, lod, face, secondLOD, function);
		}
		else
		{
			sample3D(texture, c, u, v, w, offset, lod, secondLOD, function);
		}
	}

	void SamplerCore::computeTextureCoordinates(Int4* x, Int4* y, Int4* z, Float4* fu, Float4* fv, Float4* fw, Pointer<Byte> &mipmap, Float4 &u, Float4 &v, Float4 &w, Vector4f &offset, Float &lod, SamplerFunction function)
	{
		Int4 filter = computeFilterOffset(lod);
		address(u, x, fu, mipmap, offset.x, filter, OFFSET(Mipmap, width), state.addressingModeU, function);
		address(v, y, fv, mipmap, offset.y, filter, OFFSET(Mipmap, height), state.addressingModeV, function);
		address(w, z, fw, mipmap, offset.z, filter, OFFSET(Mipmap, depth), state.addressingModeW, function);
	}

	void SamplerCore::sampleQuad2D(Pointer<Byte> &texture, Vector4s &c, Float4 &u, Float4 &v, Float4 &w, Vector4f &offset, Float &lod, Int face[4], bool secondLOD, SamplerFunction function)
	{
		int componentCount = textureComponentCount();
		bool gather = state.textureFilter == FILTER_GATHER;

		Pointer<Byte> mipmap;
		Pointer<Byte> buffer[4];

		selectMipmap(texture, buffer, mipmap, lod, face, secondLOD);

		Int4 x[2], y[2], z[2];
		Float4 fu[2], fv[2];
		computeTextureCoordinates(x, y, z, fu, fv, nullptr, mipmap, u, v, w, offset, lod, function);

		if(state.textureFilter == FILTER_POINT || (function == Fetch))
		{
			sampleTexel(c, x[0], y[0], z[0], mipmap, buffer, function);
		}
		else
		{
			Vector4s cs[4];
			for(int n = 0; n < 4; ++n)
			{
				sampleTexel(cs[n], x[n & 1], y[(n & 2) >> 1], z[0], mipmap, buffer, function);
			}

			if(!gather)   // Blend
			{
				// Fractions
				Float4 fuv[2][2];
				fuv[0][0] = fu[0] * fv[0];
				fuv[1][0] = fu[1] * fv[0];
				fuv[0][1] = fu[0] * fv[1];
				fuv[1][1] = fu[1] * fv[1];

				// Bilinear interpolation
				for(int n = 0; n < 4; ++n)
				{
					if(componentCount > n)
					{
						c[n] = hasUnsignedTextureComponent(n) ?
						       As<Short4>(UShort4(Float4(As<UShort4>(cs[0][n])) * fuv[1][1] +
						                          Float4(As<UShort4>(cs[1][n])) * fuv[0][1] +
						                          Float4(As<UShort4>(cs[2][n])) * fuv[1][0] +
						                          Float4(As<UShort4>(cs[3][n])) * fuv[0][0])) :
						       Short4(Float4(cs[0][n]) * fuv[1][1] +
						              Float4(cs[1][n]) * fuv[0][1] +
						              Float4(cs[2][n]) * fuv[1][0] +
						              Float4(cs[3][n]) * fuv[0][0]);
					}
				}
			}
			else
			{
				c.x = cs[1].x;
				c.y = cs[2].x;
				c.z = cs[3].x;
				c.w = cs[0].x;
			}
		}
	}

	void SamplerCore::sample3D(Pointer<Byte> &texture, Vector4s &c_, Float4 &u_, Float4 &v_, Float4 &w_, Vector4f &offset, Float &lod, bool secondLOD, SamplerFunction function)
	{
		int componentCount = textureComponentCount();

		Pointer<Byte> mipmap;
		Pointer<Byte> buffer[4];
		Int face[4];

		selectMipmap(texture, buffer, mipmap, lod, face, secondLOD);

		Int4 x[2], y[2], z[2];
		Float4 fu[2], fv[2], fw[2];
		computeTextureCoordinates(x, y, z, fu, fv, fw, mipmap, u_, v_, w_, offset, lod, function);

		if(state.textureFilter == FILTER_POINT || (function == Fetch))
		{
			sampleTexel(c_, x[0], y[0], z[0], mipmap, buffer, function);
		}
		else
		{
			Vector4f cf;
			for(int i = 0; i < 2; i++)
			{
				for(int j = 0; j < 2; j++)
				{
					Float4 fuv = fu[1 - i] * fv[1 - j];
					for(int k = 0; k < 2; k++)
					{
						Vector4s c;
						sampleTexel(c, x[i], y[j], z[k], mipmap, buffer, function);

						Float4 f = fuv * fw[1 - k];
						for(int n = 0; n < 4; ++n)
						{
							if(componentCount > n)
							{
								if((i == 0) && (j == 0) && (k == 0))
								{
									cf[n] = (hasUnsignedTextureComponent(n) ? Float4(As<UShort4>(c[n])) : Float4(c[n])) * f;
								}
								else
								{
									cf[n] += (hasUnsignedTextureComponent(n) ? Float4(As<UShort4>(c[n])) : Float4(c[n])) * f;
								}
							}
						}
					}
				}
			}

			for(int n = 0; n < 4; ++n)
			{
				if(componentCount > n)
				{
					c_[n] = hasUnsignedTextureComponent(n) ? As<Short4>(UShort4(cf[n])) : Short4(cf[n]);
				}
			}
		}
	}

	void SamplerCore::sampleFloatFilter(Pointer<Byte> &texture, Vector4f &c, Float4 &u, Float4 &v, Float4 &w, Vector4f &offset, Float &lod, Float &anisotropy, Float4 &uDelta, Float4 &vDelta, Int face[4], SamplerFunction function)
	{
		sampleFloatAniso(texture, c, u, v, w, offset, lod, anisotropy, uDelta, vDelta, face, false, function);

		if(function == Fetch)
		{
			return;
		}

		if(state.mipmapFilter > MIPMAP_POINT)
		{
			Vector4f cc;

			sampleFloatAniso(texture, cc, u, v, w, offset, lod, anisotropy, uDelta, vDelta, face, true, function);

			Float4 lod4 = Float4(Frac(lod));

			c.x = (cc.x - c.x) * lod4 + c.x;
			c.y = (cc.y - c.y) * lod4 + c.y;
			c.z = (cc.z - c.z) * lod4 + c.z;
			c.w = (cc.w - c.w) * lod4 + c.w;
		}

		Int4 borderMask;

		if(state.addressingModeU == ADDRESSING_BORDER)
		{
			Int4 u0;

			border(u0, u);

			borderMask = u0;
		}

		if(state.addressingModeV == ADDRESSING_BORDER)
		{
			Int4 v0;

			border(v0, v);

			if(state.addressingModeU == ADDRESSING_BORDER)
			{
				borderMask &= v0;
			}
			else
			{
				borderMask = v0;
			}
		}

		if(state.addressingModeW == ADDRESSING_BORDER && state.textureType == TEXTURE_3D)
		{
			Int4 s0;

			border(s0, w);

			if(state.addressingModeU == ADDRESSING_BORDER ||
			   state.addressingModeV == ADDRESSING_BORDER)
			{
				borderMask &= s0;
			}
			else
			{
				borderMask = s0;
			}
		}

		if(state.addressingModeU == ADDRESSING_BORDER ||
		   state.addressingModeV == ADDRESSING_BORDER ||
		   (state.addressingModeW == ADDRESSING_BORDER && state.textureType == TEXTURE_3D))
		{
			for(int n = 0; n < 4; ++n)
			{
				c[n] = As<Float4>((borderMask & As<Int4>(c[n])) | (~borderMask & *Pointer<Int4>(texture + OFFSET(Texture, borderColorF[n]))));
			}
		}
	}

	void SamplerCore::sampleFloatAniso(Pointer<Byte> &texture, Vector4f &c, Float4 &u, Float4 &v, Float4 &w, Vector4f &offset, Float &lod, Float &anisotropy, Float4 &uDelta, Float4 &vDelta, Int face[4], bool secondLOD, SamplerFunction function)
	{
		if(state.textureFilter != FILTER_ANISOTROPIC || function == Lod || function == Fetch)
		{
			sampleFloat(texture, c, u, v, w, offset, lod, face, secondLOD, function);
		}
		else
		{
			Int a = RoundInt(anisotropy);

			Vector4f cSum;

			cSum.x = Float4(0.0f);
			cSum.y = Float4(0.0f);
			cSum.z = Float4(0.0f);
			cSum.w = Float4(0.0f);

			Float4 A = *Pointer<Float4>(constants + OFFSET(Constants,uvWeight) + 16 * a);
			Float4 B = *Pointer<Float4>(constants + OFFSET(Constants,uvStart) + 16 * a);

			Float4 du = uDelta;
			Float4 dv = vDelta;

			Float4 u0 = u + B * du;
			Float4 v0 = v + B * dv;

			du *= A;
			dv *= A;

			Int i = 0;

			Do
			{
				sampleFloat(texture, c, u0, v0, w, offset, lod, face, secondLOD, function);

				u0 += du;
				v0 += dv;

				cSum.x += c.x * A;
				cSum.y += c.y * A;
				cSum.z += c.z * A;
				cSum.w += c.w * A;

				i++;
			}
			Until(i >= a)

			c.x = cSum.x;
			c.y = cSum.y;
			c.z = cSum.z;
			c.w = cSum.w;
		}
	}

	void SamplerCore::sampleFloat(Pointer<Byte> &texture, Vector4f &c, Float4 &u, Float4 &v, Float4 &w, Vector4f &offset, Float &lod, Int face[4], bool secondLOD, SamplerFunction function)
	{
		if(state.textureType != TEXTURE_3D)
		{
			sampleFloat2D(texture, c, u, v, w, offset, lod, face, secondLOD, function);
		}
		else
		{
			sampleFloat3D(texture, c, u, v, w, offset, lod, secondLOD, function);
		}
	}

	void SamplerCore::sampleFloat2D(Pointer<Byte> &texture, Vector4f &c, Float4 &u, Float4 &v, Float4 &w, Vector4f &offset, Float &lod, Int face[4], bool secondLOD, SamplerFunction function)
	{
		int componentCount = textureComponentCount();
		bool gather = state.textureFilter == FILTER_GATHER;

		Pointer<Byte> mipmap;
		Pointer<Byte> buffer[4];

		selectMipmap(texture, buffer, mipmap, lod, face, secondLOD);

		Int4 x[2], y[2], z[2];
		Float4 fu[2], fv[2];
		computeTextureCoordinates(x, y, z, fu, fv, nullptr, mipmap, u, v, w, offset, lod, function);

		if(state.textureFilter == FILTER_POINT || (function == Fetch))
		{
			sampleTexel(c, x[0], y[0], z[0], w, mipmap, buffer, function);
		}
		else
		{
			Vector4f cf[4];
			for(int n = 0; n < 4; ++n)
			{
				sampleTexel(cf[n], x[n & 1], y[(n & 2) >> 1], z[0], w, mipmap, buffer, function);
			}

			if(!gather)   // Blend
			{
				for(int n = 0; n < 4; ++n)
				{
					if(componentCount > n)
					{
						cf[0][n] = cf[0][n] + fu[0] * (cf[1][n] - cf[0][n]);
						cf[2][n] = cf[2][n] + fu[0] * (cf[3][n] - cf[2][n]);
						c[n] = cf[0][n] + fv[0] * (cf[2][n] - cf[0][n]);
					}
				}
			}
			else
			{
				c.x = cf[1].x;
				c.y = cf[2].x;
				c.z = cf[3].x;
				c.w = cf[0].x;
			}
		}
	}

	void SamplerCore::sampleFloat3D(Pointer<Byte> &texture, Vector4f &c, Float4 &u, Float4 &v, Float4 &w, Vector4f &offset, Float &lod, bool secondLOD, SamplerFunction function)
	{
		int componentCount = textureComponentCount();

		Pointer<Byte> mipmap;
		Pointer<Byte> buffer[4];
		Int face[4];

		selectMipmap(texture, buffer, mipmap, lod, face, secondLOD);

		Int4 x[2], y[2], z[2];
		Float4 fu[2], fv[2], fw[2];
		computeTextureCoordinates(x, y, z, fu, fv, fw, mipmap, u, v, w, offset, lod, function);

		if(state.textureFilter == FILTER_POINT || (function == Fetch))
		{
			sampleTexel(c, x[0], y[0], z[0], w, mipmap, buffer, function);
		}
		else
		{
			Vector4f cf[8];
			for(int t = 0; t < 8; ++t)
			{
				sampleTexel(cf[t], x[t & 1], y[(t & 2) >> 1], z[(t & 4) >> 2], w, mipmap, buffer, function);
			}

			for(int n = 0; n < 4; ++n)
			{
				if(componentCount > n)
				{
					for(int s = 0; s < 5; s += 4) // Blend first and second slices
					{
						cf[0 + s][n] = cf[0 + s][n] + fu[0] * (cf[1 + s][n] - cf[0 + s][n]);
						cf[2 + s][n] = cf[2 + s][n] + fu[0] * (cf[3 + s][n] - cf[2 + s][n]);
						cf[0 + s][n] = cf[0 + s][n] + fv[0] * (cf[2 + s][n] - cf[0 + s][n]);
					}

					// Blend slices
					c[n] = cf[0][n] + fw[0] * (cf[4][n] - cf[0][n]);
				}
			}
		}
	}

	void SamplerCore::computeLod(Pointer<Byte> &texture, Float &lod, Float &anisotropy, Float4 &uDelta, Float4 &vDelta, Float4 &uuuu, Float4 &vvvv, const Float &lodBias, Vector4f &dsx, Vector4f &dsy, SamplerFunction function)
	{
		if(function != Lod && function != Fetch)
		{
			Float4 duvdxy;

			if(function != Grad)
			{
				duvdxy = Float4(uuuu.yz, vvvv.yz) - Float4(uuuu.xx, vvvv.xx);
			}
			else
			{
				Float4 dudxy = Float4(dsx.x.xx, dsy.x.xx);
				Float4 dvdxy = Float4(dsx.y.xx, dsy.y.xx);

				duvdxy = Float4(dudxy.xz, dvdxy.xz);
			}

			// Scale by texture dimensions and LOD
			Float4 dUVdxy = duvdxy * *Pointer<Float4>(texture + OFFSET(Texture,widthHeightLOD));

			Float4 dUV2dxy = dUVdxy * dUVdxy;
			Float4 dUV2 = dUV2dxy.xy + dUV2dxy.zw;

			lod = Max(Float(dUV2.x), Float(dUV2.y));   // Square length of major axis

			if(state.textureFilter == FILTER_ANISOTROPIC)
			{
				Float det = Abs(Float(dUVdxy.x) * Float(dUVdxy.w) - Float(dUVdxy.y) * Float(dUVdxy.z));

				Float4 dudx = duvdxy.xxxx;
				Float4 dudy = duvdxy.yyyy;
				Float4 dvdx = duvdxy.zzzz;
				Float4 dvdy = duvdxy.wwww;

				Int4 mask = As<Int4>(CmpNLT(dUV2.x, dUV2.y));
				uDelta = As<Float4>((As<Int4>(dudx) & mask) | ((As<Int4>(dudy) & ~mask)));
				vDelta = As<Float4>((As<Int4>(dvdx) & mask) | ((As<Int4>(dvdy) & ~mask)));

				anisotropy = lod * Rcp_pp(det);
				anisotropy = Min(anisotropy, *Pointer<Float>(texture + OFFSET(Texture,maxAnisotropy)));

				lod *= Rcp_pp(anisotropy * anisotropy);
			}

			// log2(sqrt(lod))
			lod = Float(As<Int>(lod));
			lod -= Float(0x3F800000);
			lod *= As<Float>(Int(0x33800000));

			if(function == Bias)
			{
				lod += lodBias;
			}
		}
		else
		{
			lod = lodBias + Float(*Pointer<Int>(texture + OFFSET(Texture,baseLevel)));
		}

		lod = Max(lod, *Pointer<Float>(texture + OFFSET(Texture, minLod)));
		lod = Min(lod, *Pointer<Float>(texture + OFFSET(Texture, maxLod)));
	}

	void SamplerCore::computeLodCube(Pointer<Byte> &texture, Float &lod, Float4 &u, Float4 &v, Float4 &s, const Float &lodBias, Vector4f &dsx, Vector4f &dsy, SamplerFunction function)
	{
		if(function != Lod && function != Fetch)
		{
			if(function != Grad)
			{
				Float4 dudxy = u.ywyw - u;
				Float4 dvdxy = v.ywyw - v;
				Float4 dsdxy = s.ywyw - s;

				// Scale by texture dimensions and LOD
				dudxy *= *Pointer<Float4>(texture + OFFSET(Texture,widthLOD));
				dvdxy *= *Pointer<Float4>(texture + OFFSET(Texture,widthLOD));
				dsdxy *= *Pointer<Float4>(texture + OFFSET(Texture,widthLOD));

				dudxy *= dudxy;
				dvdxy *= dvdxy;
				dsdxy *= dsdxy;

				dudxy += dvdxy;
				dudxy += dsdxy;

				lod = Max(Float(dudxy.x), Float(dudxy.y));   // FIXME: Max(dudxy.x, dudxy.y);
			}
			else
			{
				Float4 dudxy = Float4(dsx.x.xx, dsy.x.xx);
				Float4 dvdxy = Float4(dsx.y.xx, dsy.y.xx);

				Float4 duvdxy = Float4(dudxy.xz, dvdxy.xz);

				// Scale by texture dimensions and LOD
				Float4 dUVdxy = duvdxy * *Pointer<Float4>(texture + OFFSET(Texture,widthLOD));

				Float4 dUV2dxy = dUVdxy * dUVdxy;
				Float4 dUV2 = dUV2dxy.xy + dUV2dxy.zw;

				lod = Max(Float(dUV2.x), Float(dUV2.y));   // Square length of major axis
			}

			// log2(sqrt(lod))
			lod = Float(As<Int>(lod));
			lod -= Float(0x3F800000);
			lod *= As<Float>(Int(0x33800000));

			if(function == Bias)
			{
				lod += lodBias;
			}
		}
		else
		{
			lod = lodBias + Float(*Pointer<Int>(texture + OFFSET(Texture,baseLevel)));
		}

		lod = Max(lod, *Pointer<Float>(texture + OFFSET(Texture, minLod)));
		lod = Min(lod, *Pointer<Float>(texture + OFFSET(Texture, maxLod)));
	}

	void SamplerCore::computeLod3D(Pointer<Byte> &texture, Float &lod, Float4 &uuuu, Float4 &vvvv, Float4 &wwww, const Float &lodBias, Vector4f &dsx, Vector4f &dsy, SamplerFunction function)
	{
		if(state.mipmapFilter == MIPMAP_NONE)
		{
		}
		else   // Point and linear filter
		{
			if(function != Lod && function != Fetch)
			{
				Float4 dudxy;
				Float4 dvdxy;
				Float4 dsdxy;

				if(function != Grad)
				{
					dudxy = uuuu.ywyw - uuuu;
					dvdxy = vvvv.ywyw - vvvv;
					dsdxy = wwww.ywyw - wwww;
				}
				else
				{
					dudxy = dsx.x;
					dvdxy = dsx.y;
					dsdxy = dsx.z;

					dudxy = Float4(dudxy.xx, dsy.x.xx);
					dvdxy = Float4(dvdxy.xx, dsy.y.xx);
					dsdxy = Float4(dsdxy.xx, dsy.z.xx);

					dudxy = Float4(dudxy.xz, dudxy.xz);
					dvdxy = Float4(dvdxy.xz, dvdxy.xz);
					dsdxy = Float4(dsdxy.xz, dsdxy.xz);
				}

				// Scale by texture dimensions and LOD
				dudxy *= *Pointer<Float4>(texture + OFFSET(Texture,widthLOD));
				dvdxy *= *Pointer<Float4>(texture + OFFSET(Texture,heightLOD));
				dsdxy *= *Pointer<Float4>(texture + OFFSET(Texture,depthLOD));

				dudxy *= dudxy;
				dvdxy *= dvdxy;
				dsdxy *= dsdxy;

				dudxy += dvdxy;
				dudxy += dsdxy;

				lod = Max(Float(dudxy.x), Float(dudxy.y));   // FIXME: Max(dudxy.x, dudxy.y);

				// log2(sqrt(lod))
				lod = Float(As<Int>(lod));
				lod -= Float(0x3F800000);
				lod *= As<Float>(Int(0x33800000));

				if(function == Bias)
				{
					lod += lodBias;
				}
			}
			else
			{
				lod = lodBias + Float(*Pointer<Int>(texture + OFFSET(Texture,baseLevel)));
			}

			lod = Max(lod, *Pointer<Float>(texture + OFFSET(Texture, minLod)));
			lod = Min(lod, *Pointer<Float>(texture + OFFSET(Texture, maxLod)));
		}
	}

	void SamplerCore::cubeFace(Int face[4], Float4 &U, Float4 &V, Float4 &lodX, Float4 &lodY, Float4 &lodZ, Float4 &x, Float4 &y, Float4 &z)
	{
		Int4 xn = CmpLT(x, Float4(0.0f));   // x < 0
		Int4 yn = CmpLT(y, Float4(0.0f));   // y < 0
		Int4 zn = CmpLT(z, Float4(0.0f));   // z < 0

		Float4 absX = Abs(x);
		Float4 absY = Abs(y);
		Float4 absZ = Abs(z);

		Int4 xy = CmpNLE(absX, absY);   // abs(x) > abs(y)
		Int4 yz = CmpNLE(absY, absZ);   // abs(y) > abs(z)
		Int4 zx = CmpNLE(absZ, absX);   // abs(z) > abs(x)
		Int4 xMajor = xy & ~zx;   // abs(x) > abs(y) && abs(x) > abs(z)
		Int4 yMajor = yz & ~xy;   // abs(y) > abs(z) && abs(y) > abs(x)
		Int4 zMajor = zx & ~yz;   // abs(z) > abs(x) && abs(z) > abs(y)

		// FACE_POSITIVE_X = 000b
		// FACE_NEGATIVE_X = 001b
		// FACE_POSITIVE_Y = 010b
		// FACE_NEGATIVE_Y = 011b
		// FACE_POSITIVE_Z = 100b
		// FACE_NEGATIVE_Z = 101b

		Int yAxis = SignMask(yMajor);
		Int zAxis = SignMask(zMajor);

		Int4 n = ((xn & xMajor) | (yn & yMajor) | (zn & zMajor)) & Int4(0x80000000);
		Int negative = SignMask(n);

		face[0] = *Pointer<Int>(constants + OFFSET(Constants,transposeBit0) + negative * 4);
		face[0] |= *Pointer<Int>(constants + OFFSET(Constants,transposeBit1) + yAxis * 4);
		face[0] |= *Pointer<Int>(constants + OFFSET(Constants,transposeBit2) + zAxis * 4);
		face[1] = (face[0] >> 4)  & 0x7;
		face[2] = (face[0] >> 8)  & 0x7;
		face[3] = (face[0] >> 12) & 0x7;
		face[0] &= 0x7;

		Float4 M = Max(Max(absX, absY), absZ);

		// U = xMajor ? (neg ^ -z) : (zMajor & neg) ^ x)
		U = As<Float4>((xMajor & (n ^ As<Int4>(-z))) | (~xMajor & ((zMajor & n) ^ As<Int4>(x))));

		// V = !yMajor ? -y : (n ^ z)
		V = As<Float4>((~yMajor & As<Int4>(-y)) | (yMajor & (n ^ As<Int4>(z))));

		M = reciprocal(M) * Float4(0.5f);
		U = U * M + Float4(0.5f);
		V = V * M + Float4(0.5f);

		lodX = x * M;
		lodY = y * M;
		lodZ = z * M;
	}

	void SamplerCore::computeIndices(UInt index[4], Int4 uuuu, Int4 vvvv, Int4 wwww, const Pointer<Byte> &mipmap, SamplerFunction function)
	{
		UInt4 indices = uuuu + vvvv * *Pointer<Int4>(mipmap + OFFSET(Mipmap, pitchP));

		if((state.textureType == TEXTURE_3D) || (state.textureType == TEXTURE_2D_ARRAY))
		{
			indices += As<UInt4>(wwww * *Pointer<Int4>(mipmap + OFFSET(Mipmap, sliceP)));
		}

		if(function == Fetch)
		{
			UInt4 size = *Pointer<UInt4>(mipmap + OFFSET(Mipmap, sliceP));
			if((state.textureType == TEXTURE_3D) || (state.textureType == TEXTURE_2D_ARRAY))
			{
				size *= *Pointer<UInt4>(mipmap + OFFSET(Mipmap, depth));
			}
			indices = Min(Max(indices, UInt4(0)), size - UInt4(1));
		}

		for(int i = 0; i < 4; i++)
		{
			index[i] = Extract(As<Int4>(indices), i);
		}
	}

	void SamplerCore::sampleTexel(Vector4s &c, Int4 &uuuu, Int4 &vvvv, Int4 &wwww, Pointer<Byte> &mipmap, Pointer<Byte> buffer[4], SamplerFunction function)
	{
		UInt index[4];
		computeIndices(index, uuuu, vvvv, wwww, mipmap, function);

		int f0 = state.textureType == TEXTURE_CUBE ? 0 : 0;
		int f1 = state.textureType == TEXTURE_CUBE ? 1 : 0;
		int f2 = state.textureType == TEXTURE_CUBE ? 2 : 0;
		int f3 = state.textureType == TEXTURE_CUBE ? 3 : 0;

		if(has16bitTextureFormat())
		{
			c.x = Insert(c.x, Pointer<Short>(buffer[f0])[index[0]], 0);
			c.x = Insert(c.x, Pointer<Short>(buffer[f1])[index[1]], 1);
			c.x = Insert(c.x, Pointer<Short>(buffer[f2])[index[2]], 2);
			c.x = Insert(c.x, Pointer<Short>(buffer[f3])[index[3]], 3);

			switch(state.textureFormat)
			{
			case FORMAT_R5G6B5:
				c.z = (c.x & Short4(0x001Fu)) << 11;
				c.y = (c.x & Short4(0x07E0u)) << 5;
				c.x = (c.x & Short4(0xF800u));
				break;
			default:
				ASSERT(false);
			}
		}
		else if(has8bitTextureComponents())
		{
			switch(textureComponentCount())
			{
			case 4:
				{
					Byte4 c0 = Pointer<Byte4>(buffer[f0])[index[0]];
					Byte4 c1 = Pointer<Byte4>(buffer[f1])[index[1]];
					Byte4 c2 = Pointer<Byte4>(buffer[f2])[index[2]];
					Byte4 c3 = Pointer<Byte4>(buffer[f3])[index[3]];
					c.x = Unpack(c0, c1);
					c.y = Unpack(c2, c3);

					switch(state.textureFormat)
					{
					case FORMAT_A8R8G8B8:
						c.z = c.x;
						c.z = As<Short4>(UnpackLow(c.z, c.y));
						c.x = As<Short4>(UnpackHigh(c.x, c.y));
						c.y = c.z;
						c.w = c.x;
						c.z = UnpackLow(As<Byte8>(c.z), As<Byte8>(c.z));
						c.y = UnpackHigh(As<Byte8>(c.y), As<Byte8>(c.y));
						c.x = UnpackLow(As<Byte8>(c.x), As<Byte8>(c.x));
						c.w = UnpackHigh(As<Byte8>(c.w), As<Byte8>(c.w));
						break;
					case FORMAT_A8B8G8R8:
					case FORMAT_A8B8G8R8I:
					case FORMAT_A8B8G8R8UI:
					case FORMAT_A8B8G8R8I_SNORM:
					case FORMAT_Q8W8V8U8:
					case FORMAT_SRGB8_A8:
						c.z = c.x;
						c.x = As<Short4>(UnpackLow(c.x, c.y));
						c.z = As<Short4>(UnpackHigh(c.z, c.y));
						c.y = c.x;
						c.w = c.z;
						c.x = UnpackLow(As<Byte8>(c.x), As<Byte8>(c.x));
						c.y = UnpackHigh(As<Byte8>(c.y), As<Byte8>(c.y));
						c.z = UnpackLow(As<Byte8>(c.z), As<Byte8>(c.z));
						c.w = UnpackHigh(As<Byte8>(c.w), As<Byte8>(c.w));
						break;
					default:
						ASSERT(false);
					}
				}
				break;
			case 3:
				{
					Byte4 c0 = Pointer<Byte4>(buffer[f0])[index[0]];
					Byte4 c1 = Pointer<Byte4>(buffer[f1])[index[1]];
					Byte4 c2 = Pointer<Byte4>(buffer[f2])[index[2]];
					Byte4 c3 = Pointer<Byte4>(buffer[f3])[index[3]];
					c.x = Unpack(c0, c1);
					c.y = Unpack(c2, c3);

					switch(state.textureFormat)
					{
					case FORMAT_X8R8G8B8:
						c.z = c.x;
						c.z = As<Short4>(UnpackLow(c.z, c.y));
						c.x = As<Short4>(UnpackHigh(c.x, c.y));
						c.y = c.z;
						c.z = UnpackLow(As<Byte8>(c.z), As<Byte8>(c.z));
						c.y = UnpackHigh(As<Byte8>(c.y), As<Byte8>(c.y));
						c.x = UnpackLow(As<Byte8>(c.x), As<Byte8>(c.x));
						break;
					case FORMAT_X8B8G8R8I_SNORM:
					case FORMAT_X8B8G8R8UI:
					case FORMAT_X8B8G8R8I:
					case FORMAT_X8B8G8R8:
					case FORMAT_X8L8V8U8:
					case FORMAT_SRGB8_X8:
						c.z = c.x;
						c.x = As<Short4>(UnpackLow(c.x, c.y));
						c.z = As<Short4>(UnpackHigh(c.z, c.y));
						c.y = c.x;
						c.x = UnpackLow(As<Byte8>(c.x), As<Byte8>(c.x));
						c.y = UnpackHigh(As<Byte8>(c.y), As<Byte8>(c.y));
						c.z = UnpackLow(As<Byte8>(c.z), As<Byte8>(c.z));
						break;
					default:
						ASSERT(false);
					}
				}
				break;
			case 2:
				c.x = Insert(c.x, Pointer<Short>(buffer[f0])[index[0]], 0);
				c.x = Insert(c.x, Pointer<Short>(buffer[f1])[index[1]], 1);
				c.x = Insert(c.x, Pointer<Short>(buffer[f2])[index[2]], 2);
				c.x = Insert(c.x, Pointer<Short>(buffer[f3])[index[3]], 3);

				switch(state.textureFormat)
				{
				case FORMAT_G8R8:
				case FORMAT_G8R8I:
				case FORMAT_G8R8UI:
				case FORMAT_G8R8I_SNORM:
				case FORMAT_V8U8:
				case FORMAT_A8L8:
					c.y = (c.x & Short4(0xFF00u)) | As<Short4>(As<UShort4>(c.x) >> 8);
					c.x = (c.x & Short4(0x00FFu)) | (c.x << 8);
					break;
				default:
					ASSERT(false);
				}
				break;
			case 1:
				{
					Int c0 = Int(*Pointer<Byte>(buffer[f0] + index[0]));
					Int c1 = Int(*Pointer<Byte>(buffer[f1] + index[1]));
					Int c2 = Int(*Pointer<Byte>(buffer[f2] + index[2]));
					Int c3 = Int(*Pointer<Byte>(buffer[f3] + index[3]));
					c0 = c0 | (c1 << 8) | (c2 << 16) | (c3 << 24);
					c.x = Unpack(As<Byte4>(c0));
				}
				break;
			default:
				ASSERT(false);
			}
		}
		else if(has16bitTextureComponents())
		{
			switch(textureComponentCount())
			{
			case 4:
				c.x = Pointer<Short4>(buffer[f0])[index[0]];
				c.y = Pointer<Short4>(buffer[f1])[index[1]];
				c.z = Pointer<Short4>(buffer[f2])[index[2]];
				c.w = Pointer<Short4>(buffer[f3])[index[3]];
				transpose4x4(c.x, c.y, c.z, c.w);
				break;
			case 2:
				c.x = *Pointer<Short4>(buffer[f0] + 4 * index[0]);
				c.x = As<Short4>(UnpackLow(c.x, *Pointer<Short4>(buffer[f1] + 4 * index[1])));
				c.z = *Pointer<Short4>(buffer[f2] + 4 * index[2]);
				c.z = As<Short4>(UnpackLow(c.z, *Pointer<Short4>(buffer[f3] + 4 * index[3])));
				c.y = c.x;
				c.x = UnpackLow(As<Int2>(c.x), As<Int2>(c.z));
				c.y = UnpackHigh(As<Int2>(c.y), As<Int2>(c.z));
				break;
			case 1:
				c.x = Insert(c.x, Pointer<Short>(buffer[f0])[index[0]], 0);
				c.x = Insert(c.x, Pointer<Short>(buffer[f1])[index[1]], 1);
				c.x = Insert(c.x, Pointer<Short>(buffer[f2])[index[2]], 2);
				c.x = Insert(c.x, Pointer<Short>(buffer[f3])[index[3]], 3);
				break;
			default:
				ASSERT(false);
			}
		}
		else if(hasYuvFormat())
		{
			// Generic YPbPr to RGB transformation
			// R = Y                               +           2 * (1 - Kr) * Pr
			// G = Y - 2 * Kb * (1 - Kb) / Kg * Pb - 2 * Kr * (1 - Kr) / Kg * Pr
			// B = Y +           2 * (1 - Kb) * Pb

			float Kb = 0.114f;
			float Kr = 0.299f;
			int studioSwing = 1;

			switch(state.textureFormat)
			{
			case FORMAT_YV12_BT601:
				Kb = 0.114f;
				Kr = 0.299f;
				studioSwing = 1;
				break;
			case FORMAT_YV12_BT709:
				Kb = 0.0722f;
				Kr = 0.2126f;
				studioSwing = 1;
				break;
			case FORMAT_YV12_JFIF:
				Kb = 0.114f;
				Kr = 0.299f;
				studioSwing = 0;
				break;
			default:
				ASSERT(false);
			}

			const float Kg = 1.0f - Kr - Kb;

			const float Rr = 2 * (1 - Kr);
			const float Gb = -2 * Kb * (1 - Kb) / Kg;
			const float Gr = -2 * Kr * (1 - Kr) / Kg;
			const float Bb = 2 * (1 - Kb);

			// Scaling and bias for studio-swing range: Y = [16 .. 235], U/V = [16 .. 240]
			const float Yy = studioSwing ? 255.0f / (235 - 16) : 1.0f;
			const float Uu = studioSwing ? 255.0f / (240 - 16) : 1.0f;
			const float Vv = studioSwing ? 255.0f / (240 - 16) : 1.0f;

			const float Rv = Vv *  Rr;
			const float Gu = Uu *  Gb;
			const float Gv = Vv *  Gr;
			const float Bu = Uu *  Bb;

			const float R0 = (studioSwing * -16 * Yy - 128 * Rv) / 255;
			const float G0 = (studioSwing * -16 * Yy - 128 * Gu - 128 * Gv) / 255;
			const float B0 = (studioSwing * -16 * Yy - 128 * Bu) / 255;

			Int c0 = Int(buffer[0][index[0]]);
			Int c1 = Int(buffer[0][index[1]]);
			Int c2 = Int(buffer[0][index[2]]);
			Int c3 = Int(buffer[0][index[3]]);
			c0 = c0 | (c1 << 8) | (c2 << 16) | (c3 << 24);
			UShort4 Y = As<UShort4>(Unpack(As<Byte4>(c0)));

			computeIndices(index, uuuu >> 1, vvvv >> 1, wwww, mipmap + sizeof(Mipmap), function);
			c0 = Int(buffer[1][index[0]]);
			c1 = Int(buffer[1][index[1]]);
			c2 = Int(buffer[1][index[2]]);
			c3 = Int(buffer[1][index[3]]);
			c0 = c0 | (c1 << 8) | (c2 << 16) | (c3 << 24);
			UShort4 V = As<UShort4>(Unpack(As<Byte4>(c0)));

			c0 = Int(buffer[2][index[0]]);
			c1 = Int(buffer[2][index[1]]);
			c2 = Int(buffer[2][index[2]]);
			c3 = Int(buffer[2][index[3]]);
			c0 = c0 | (c1 << 8) | (c2 << 16) | (c3 << 24);
			UShort4 U = As<UShort4>(Unpack(As<Byte4>(c0)));

			const UShort4 yY = UShort4(iround(Yy * 0x4000));
			const UShort4 rV = UShort4(iround(Rv * 0x4000));
			const UShort4 gU = UShort4(iround(-Gu * 0x4000));
			const UShort4 gV = UShort4(iround(-Gv * 0x4000));
			const UShort4 bU = UShort4(iround(Bu * 0x4000));

			const UShort4 r0 = UShort4(iround(-R0 * 0x4000));
			const UShort4 g0 = UShort4(iround(G0 * 0x4000));
			const UShort4 b0 = UShort4(iround(-B0 * 0x4000));

			UShort4 y = MulHigh(Y, yY);
			UShort4 r = SubSat(y + MulHigh(V, rV), r0);
			UShort4 g = SubSat(y + g0, MulHigh(U, gU) + MulHigh(V, gV));
			UShort4 b = SubSat(y + MulHigh(U, bU), b0);

			c.x = Min(r, UShort4(0x3FFF)) << 2;
			c.y = Min(g, UShort4(0x3FFF)) << 2;
			c.z = Min(b, UShort4(0x3FFF)) << 2;
		}
		else ASSERT(false);
	}

	void SamplerCore::sampleTexel(Vector4f &c, Int4 &uuuu, Int4 &vvvv, Int4 &wwww, Float4 &z, Pointer<Byte> &mipmap, Pointer<Byte> buffer[4], SamplerFunction function)
	{
		UInt index[4];

		computeIndices(index, uuuu, vvvv, wwww, mipmap, function);

		int f0 = state.textureType == TEXTURE_CUBE ? 0 : 0;
		int f1 = state.textureType == TEXTURE_CUBE ? 1 : 0;
		int f2 = state.textureType == TEXTURE_CUBE ? 2 : 0;
		int f3 = state.textureType == TEXTURE_CUBE ? 3 : 0;

		// Read texels
		switch(textureComponentCount())
		{
		case 4:
			c.x = *Pointer<Float4>(buffer[f0] + index[0] * 16, 16);
			c.y = *Pointer<Float4>(buffer[f1] + index[1] * 16, 16);
			c.z = *Pointer<Float4>(buffer[f2] + index[2] * 16, 16);
			c.w = *Pointer<Float4>(buffer[f3] + index[3] * 16, 16);
			transpose4x4(c.x, c.y, c.z, c.w);
			break;
		case 3:
			ASSERT(state.textureFormat == FORMAT_X32B32G32R32F);
			c.x = *Pointer<Float4>(buffer[f0] + index[0] * 16, 16);
			c.y = *Pointer<Float4>(buffer[f1] + index[1] * 16, 16);
			c.z = *Pointer<Float4>(buffer[f2] + index[2] * 16, 16);
			c.w = *Pointer<Float4>(buffer[f3] + index[3] * 16, 16);
			transpose4x3(c.x, c.y, c.z, c.w);
			c.w = Float4(1.0f);
			break;
		case 2:
			// FIXME: Optimal shuffling?
			c.x.xy = *Pointer<Float4>(buffer[f0] + index[0] * 8);
			c.x.zw = *Pointer<Float4>(buffer[f1] + index[1] * 8 - 8);
			c.z.xy = *Pointer<Float4>(buffer[f2] + index[2] * 8);
			c.z.zw = *Pointer<Float4>(buffer[f3] + index[3] * 8 - 8);
			c.y = c.x;
			c.x = Float4(c.x.xz, c.z.xz);
			c.y = Float4(c.y.yw, c.z.yw);
			break;
		case 1:
			// FIXME: Optimal shuffling?
			c.x.x = *Pointer<Float>(buffer[f0] + index[0] * 4);
			c.x.y = *Pointer<Float>(buffer[f1] + index[1] * 4);
			c.x.z = *Pointer<Float>(buffer[f2] + index[2] * 4);
			c.x.w = *Pointer<Float>(buffer[f3] + index[3] * 4);

			if(state.textureFormat == FORMAT_D32FS8_SHADOW && state.textureFilter != FILTER_GATHER)
			{
				Float4 d = Min(Max(z, Float4(0.0f)), Float4(1.0f));

				c.x = As<Float4>(As<Int4>(CmpNLT(c.x, d)) & As<Int4>(Float4(1.0f)));   // FIXME: Only less-equal?
			}
			break;
		default:
			ASSERT(false);
		}
	}

	void SamplerCore::selectMipmap(Pointer<Byte> &texture, Pointer<Byte> buffer[4], Pointer<Byte> &mipmap, Float &lod, Int face[4], bool secondLOD)
	{
		if(state.mipmapFilter < MIPMAP_POINT)
		{
			mipmap = texture + OFFSET(Texture,mipmap[0]);
		}
		else
		{
			Int ilod;

			if(state.mipmapFilter == MIPMAP_POINT)
			{
				ilod = RoundInt(lod);
			}
			else   // Linear
			{
				ilod = Int(lod);
			}

			mipmap = texture + OFFSET(Texture,mipmap) + ilod * sizeof(Mipmap) + secondLOD * sizeof(Mipmap);
		}

		if(state.textureType != TEXTURE_CUBE)
		{
			buffer[0] = *Pointer<Pointer<Byte> >(mipmap + OFFSET(Mipmap,buffer[0]));

			if(hasYuvFormat())
			{
				buffer[1] = *Pointer<Pointer<Byte> >(mipmap + OFFSET(Mipmap,buffer[1]));
				buffer[2] = *Pointer<Pointer<Byte> >(mipmap + OFFSET(Mipmap,buffer[2]));
			}
		}
		else
		{
			for(int i = 0; i < 4; i++)
			{
				buffer[i] = *Pointer<Pointer<Byte> >(mipmap + OFFSET(Mipmap,buffer) + face[i] * sizeof(void*));
			}
		}
	}

	Int4 SamplerCore::computeFilterOffset(Float &lod)
	{
		Int4 filtering((state.textureFilter == FILTER_POINT) ? 0 : 1);
		if(state.textureFilter == FILTER_MIN_LINEAR_MAG_POINT)
		{
			filtering &= CmpNLE(Float4(lod), Float4(0.0f));
		}
		else if(state.textureFilter == FILTER_MIN_POINT_MAG_LINEAR)
		{
			filtering &= CmpLE(Float4(lod), Float4(0.0f));
		}
		return filtering;
	}

	void SamplerCore::address(Float4 &uvw, Int4* xyz, Float4* ratio, Pointer<Byte>& mipmap, Float4 &texOffset, Int4 &filter, int whd, AddressingMode addressingMode, SamplerFunction function)
	{
		if(addressingMode == ADDRESSING_LAYER && state.textureType != TEXTURE_2D_ARRAY)
		{
			return; // Unused
		}

		Int4 dim = *Pointer<Int4>(mipmap + whd);

		if(function == Fetch)
		{
			xyz[0] = Min(Max(((function.option == Offset) && (addressingMode != ADDRESSING_LAYER)) ? As<Int4>(uvw) + As<Int4>(texOffset) : As<Int4>(uvw), Int4(0)), dim - Int4(1));
		}
		else if(addressingMode == ADDRESSING_LAYER && state.textureType == TEXTURE_2D_ARRAY) // Note: Offset does not apply to array layers
		{
			xyz[0] = Min(Max(RoundInt(uvw), Int4(0)), dim - Int4(1));
		}
		else
		{
			Float4 tmp = uvw;

			switch(addressingMode)
			{
			case ADDRESSING_CLAMP:
				tmp = Min(Max(tmp, Float4(0.0f)), Float4(0.99999f));
				break;
			case ADDRESSING_MIRROR:
				tmp = Float4(0.99999f) - Abs(Float4(1.99998f) * Frac(tmp * Float4(0.5f)) - Float4(0.99999f));
				break;
			case ADDRESSING_MIRRORONCE:
				tmp = Float4(0.99999f) - Abs(Float4(1.99998f) * Frac(Min(Max(tmp, Float4(-1.0f)), Float4(2.0f)) * Float4(0.5f)) - Float4(0.99999f));
				break;
			default:   // Wrap (or border)
				tmp = Frac(tmp);
				break;
			}

			tmp *= Float4(dim);
			xyz[0] = Int4(tmp);

			if(function.option == Offset)
			{
				xyz[0] += As<Int4>(texOffset);
				switch(addressingMode)
				{
				case ADDRESSING_MIRROR:
				case ADDRESSING_MIRRORONCE:
				case ADDRESSING_BORDER:
					// FIXME: Implement ADDRESSING_MIRROR, ADDRESSING_MIRRORONCE and ADDRESSING_BORDER. Fall through to Clamp.
				case ADDRESSING_CLAMP:
					xyz[0] = (xyz[0] + dim * Int4(-MIN_PROGRAM_TEXEL_OFFSET)) % dim;
					break;
				default:   // Wrap
					xyz[0] = Min(Max(xyz[0], Int4(0)), dim - Int4(1));
					break;
				}
			}

			if(state.textureFilter != FILTER_POINT) // Compute 2nd coordinate
			{
				bool gather = state.textureFilter == FILTER_GATHER;

				xyz[1] = xyz[0] + filter; // Increment
				
				if(!gather)
				{
					ASSERT(ratio);
					Float4 frac = Frac(tmp);
					ratio[0] = Abs(frac - Float4(0.5f));
					ratio[1] = Float4(1.0f) - ratio[0];
					xyz[1] -= CmpLT(frac, Float4(0.5f)) & (filter + filter); // Decrement xyz if necessary
				}

				switch(addressingMode)
				{
				case ADDRESSING_MIRROR:
				case ADDRESSING_MIRRORONCE:
				case ADDRESSING_BORDER:
					// FIXME: Implement ADDRESSING_MIRROR, ADDRESSING_MIRRORONCE and ADDRESSING_BORDER. Fall through to Clamp.
				case ADDRESSING_CLAMP:
					xyz[1] = gather ? Min(xyz[1], dim - Int4(1)) : Min(Max(xyz[1], Int4(0)), dim - Int4(1));
					break;
				default:   // Wrap
					{
						// The coordinates overflow or underflow by at most 1
						Int4 over = CmpNLT(xyz[1], dim);
						xyz[1] = (over & Int4(1)) | (~over & xyz[1]); // xyz >= dim ? 1 : xyz
						if(!gather)
						{
							Int4 under = CmpLT(xyz[1], Int4(0));
							xyz[1] = (under & (dim - Int4(1))) | (~under & xyz[1]); // xyz < 0 ? dim - 1 : xyz
						}
					}
					break;
				}
			}
		}
	}

	void SamplerCore::convertFixed12(Short4 &cs, Float4 &cf)
	{
		cs = RoundShort4(cf * Float4(0x1000));
	}

	void SamplerCore::convertFixed12(Vector4s &cs, Vector4f &cf)
	{
		for(int n = 0; n < 4; ++n)
		{
			convertFixed12(cs[n], cf[n]);
		}
	}

	void SamplerCore::convertSigned12(Float4 &cf, Short4 &cs)
	{
		cf = Float4(cs) * Float4(1.0f / 0x0FFE);
	}

//	void SamplerCore::convertSigned12(Vector4f &cf, Vector4s &cs)
//	{
//		for(int n = 0; n < 4; ++n)
//		{
//			convertSigned12(cf[n], cs[n]);
//		}
//	}

	void SamplerCore::convertSigned15(Float4 &cf, Short4 &cs)
	{
		cf = Float4(cs) * Float4(1.0f / 0x7FFF);
	}

	void SamplerCore::convertUnsigned16(Float4 &cf, Short4 &cs)
	{
		cf = Float4(As<UShort4>(cs)) * Float4(1.0f / 0xFFFF);
	}

	void SamplerCore::sRGBtoLinear16_8_12(Short4 &c)
	{
		c = As<UShort4>(c) >> 8;

		Pointer<Byte> LUT = Pointer<Byte>(constants + OFFSET(Constants,sRGBtoLinear8_12));

		c = Insert(c, *Pointer<Short>(LUT + 2 * Int(Extract(c, 0))), 0);
		c = Insert(c, *Pointer<Short>(LUT + 2 * Int(Extract(c, 1))), 1);
		c = Insert(c, *Pointer<Short>(LUT + 2 * Int(Extract(c, 2))), 2);
		c = Insert(c, *Pointer<Short>(LUT + 2 * Int(Extract(c, 3))), 3);
	}

	void SamplerCore::sRGBtoLinear16_6_12(Short4 &c)
	{
		c = As<UShort4>(c) >> 10;

		Pointer<Byte> LUT = Pointer<Byte>(constants + OFFSET(Constants,sRGBtoLinear6_12));

		c = Insert(c, *Pointer<Short>(LUT + 2 * Int(Extract(c, 0))), 0);
		c = Insert(c, *Pointer<Short>(LUT + 2 * Int(Extract(c, 1))), 1);
		c = Insert(c, *Pointer<Short>(LUT + 2 * Int(Extract(c, 2))), 2);
		c = Insert(c, *Pointer<Short>(LUT + 2 * Int(Extract(c, 3))), 3);
	}

	void SamplerCore::sRGBtoLinear16_5_12(Short4 &c)
	{
		c = As<UShort4>(c) >> 11;

		Pointer<Byte> LUT = Pointer<Byte>(constants + OFFSET(Constants,sRGBtoLinear5_12));

		c = Insert(c, *Pointer<Short>(LUT + 2 * Int(Extract(c, 0))), 0);
		c = Insert(c, *Pointer<Short>(LUT + 2 * Int(Extract(c, 1))), 1);
		c = Insert(c, *Pointer<Short>(LUT + 2 * Int(Extract(c, 2))), 2);
		c = Insert(c, *Pointer<Short>(LUT + 2 * Int(Extract(c, 3))), 3);
	}

	bool SamplerCore::hasFloatTexture() const
	{
		return Surface::isFloatFormat(state.textureFormat);
	}

	bool SamplerCore::hasUnsignedTextureComponent(int component) const
	{
		return Surface::isUnsignedComponent(state.textureFormat, component);
	}

	int SamplerCore::textureComponentCount() const
	{
		return Surface::componentCount(state.textureFormat);
	}

	bool SamplerCore::has16bitTextureFormat() const
	{
		switch(state.textureFormat)
		{
		case FORMAT_R5G6B5:
			return true;
		case FORMAT_R8I_SNORM:
		case FORMAT_G8R8I_SNORM:
		case FORMAT_X8B8G8R8I_SNORM:
		case FORMAT_A8B8G8R8I_SNORM:
		case FORMAT_R8I:
		case FORMAT_R8UI:
		case FORMAT_G8R8I:
		case FORMAT_G8R8UI:
		case FORMAT_X8B8G8R8I:
		case FORMAT_X8B8G8R8UI:
		case FORMAT_A8B8G8R8I:
		case FORMAT_A8B8G8R8UI:
		case FORMAT_R32I:
		case FORMAT_R32UI:
		case FORMAT_G32R32I:
		case FORMAT_G32R32UI:
		case FORMAT_X32B32G32R32I:
		case FORMAT_X32B32G32R32UI:
		case FORMAT_A32B32G32R32I:
		case FORMAT_A32B32G32R32UI:
		case FORMAT_G8R8:
		case FORMAT_X8R8G8B8:
		case FORMAT_X8B8G8R8:
		case FORMAT_A8R8G8B8:
		case FORMAT_A8B8G8R8:
		case FORMAT_SRGB8_X8:
		case FORMAT_SRGB8_A8:
		case FORMAT_V8U8:
		case FORMAT_Q8W8V8U8:
		case FORMAT_X8L8V8U8:
		case FORMAT_R32F:
		case FORMAT_G32R32F:
		case FORMAT_X32B32G32R32F:
		case FORMAT_A32B32G32R32F:
		case FORMAT_A8:
		case FORMAT_R8:
		case FORMAT_L8:
		case FORMAT_A8L8:
		case FORMAT_D32F:
		case FORMAT_D32F_LOCKABLE:
		case FORMAT_D32FS8_TEXTURE:
		case FORMAT_D32FS8_SHADOW:
		case FORMAT_L16:
		case FORMAT_G16R16:
		case FORMAT_A16B16G16R16:
		case FORMAT_V16U16:
		case FORMAT_A16W16V16U16:
		case FORMAT_Q16W16V16U16:
		case FORMAT_R16I:
		case FORMAT_R16UI:
		case FORMAT_G16R16I:
		case FORMAT_G16R16UI:
		case FORMAT_X16B16G16R16I:
		case FORMAT_X16B16G16R16UI:
		case FORMAT_A16B16G16R16I:
		case FORMAT_A16B16G16R16UI:
		case FORMAT_YV12_BT601:
		case FORMAT_YV12_BT709:
		case FORMAT_YV12_JFIF:
			return false;
		default:
			ASSERT(false);
		}

		return false;
	}

	bool SamplerCore::has8bitTextureComponents() const
	{
		switch(state.textureFormat)
		{
		case FORMAT_G8R8:
		case FORMAT_X8R8G8B8:
		case FORMAT_X8B8G8R8:
		case FORMAT_A8R8G8B8:
		case FORMAT_A8B8G8R8:
		case FORMAT_SRGB8_X8:
		case FORMAT_SRGB8_A8:
		case FORMAT_V8U8:
		case FORMAT_Q8W8V8U8:
		case FORMAT_X8L8V8U8:
		case FORMAT_A8:
		case FORMAT_R8:
		case FORMAT_L8:
		case FORMAT_A8L8:
		case FORMAT_R8I_SNORM:
		case FORMAT_G8R8I_SNORM:
		case FORMAT_X8B8G8R8I_SNORM:
		case FORMAT_A8B8G8R8I_SNORM:
		case FORMAT_R8I:
		case FORMAT_R8UI:
		case FORMAT_G8R8I:
		case FORMAT_G8R8UI:
		case FORMAT_X8B8G8R8I:
		case FORMAT_X8B8G8R8UI:
		case FORMAT_A8B8G8R8I:
		case FORMAT_A8B8G8R8UI:
			return true;
		case FORMAT_R5G6B5:
		case FORMAT_R32F:
		case FORMAT_G32R32F:
		case FORMAT_X32B32G32R32F:
		case FORMAT_A32B32G32R32F:
		case FORMAT_D32F:
		case FORMAT_D32F_LOCKABLE:
		case FORMAT_D32FS8_TEXTURE:
		case FORMAT_D32FS8_SHADOW:
		case FORMAT_L16:
		case FORMAT_G16R16:
		case FORMAT_A16B16G16R16:
		case FORMAT_V16U16:
		case FORMAT_A16W16V16U16:
		case FORMAT_Q16W16V16U16:
		case FORMAT_R32I:
		case FORMAT_R32UI:
		case FORMAT_G32R32I:
		case FORMAT_G32R32UI:
		case FORMAT_X32B32G32R32I:
		case FORMAT_X32B32G32R32UI:
		case FORMAT_A32B32G32R32I:
		case FORMAT_A32B32G32R32UI:
		case FORMAT_R16I:
		case FORMAT_R16UI:
		case FORMAT_G16R16I:
		case FORMAT_G16R16UI:
		case FORMAT_X16B16G16R16I:
		case FORMAT_X16B16G16R16UI:
		case FORMAT_A16B16G16R16I:
		case FORMAT_A16B16G16R16UI:
		case FORMAT_YV12_BT601:
		case FORMAT_YV12_BT709:
		case FORMAT_YV12_JFIF:
			return false;
		default:
			ASSERT(false);
		}

		return false;
	}

	bool SamplerCore::has16bitTextureComponents() const
	{
		switch(state.textureFormat)
		{
		case FORMAT_R5G6B5:
		case FORMAT_R8I_SNORM:
		case FORMAT_G8R8I_SNORM:
		case FORMAT_X8B8G8R8I_SNORM:
		case FORMAT_A8B8G8R8I_SNORM:
		case FORMAT_R8I:
		case FORMAT_R8UI:
		case FORMAT_G8R8I:
		case FORMAT_G8R8UI:
		case FORMAT_X8B8G8R8I:
		case FORMAT_X8B8G8R8UI:
		case FORMAT_A8B8G8R8I:
		case FORMAT_A8B8G8R8UI:
		case FORMAT_R32I:
		case FORMAT_R32UI:
		case FORMAT_G32R32I:
		case FORMAT_G32R32UI:
		case FORMAT_X32B32G32R32I:
		case FORMAT_X32B32G32R32UI:
		case FORMAT_A32B32G32R32I:
		case FORMAT_A32B32G32R32UI:
		case FORMAT_G8R8:
		case FORMAT_X8R8G8B8:
		case FORMAT_X8B8G8R8:
		case FORMAT_A8R8G8B8:
		case FORMAT_A8B8G8R8:
		case FORMAT_SRGB8_X8:
		case FORMAT_SRGB8_A8:
		case FORMAT_V8U8:
		case FORMAT_Q8W8V8U8:
		case FORMAT_X8L8V8U8:
		case FORMAT_R32F:
		case FORMAT_G32R32F:
		case FORMAT_X32B32G32R32F:
		case FORMAT_A32B32G32R32F:
		case FORMAT_A8:
		case FORMAT_R8:
		case FORMAT_L8:
		case FORMAT_A8L8:
		case FORMAT_D32F:
		case FORMAT_D32F_LOCKABLE:
		case FORMAT_D32FS8_TEXTURE:
		case FORMAT_D32FS8_SHADOW:
		case FORMAT_YV12_BT601:
		case FORMAT_YV12_BT709:
		case FORMAT_YV12_JFIF:
			return false;
		case FORMAT_L16:
		case FORMAT_G16R16:
		case FORMAT_A16B16G16R16:
		case FORMAT_R16I:
		case FORMAT_R16UI:
		case FORMAT_G16R16I:
		case FORMAT_G16R16UI:
		case FORMAT_X16B16G16R16I:
		case FORMAT_X16B16G16R16UI:
		case FORMAT_A16B16G16R16I:
		case FORMAT_A16B16G16R16UI:
		case FORMAT_V16U16:
		case FORMAT_A16W16V16U16:
		case FORMAT_Q16W16V16U16:
			return true;
		default:
			ASSERT(false);
		}

		return false;
	}

	bool SamplerCore::hasYuvFormat() const
	{
		switch(state.textureFormat)
		{
		case FORMAT_YV12_BT601:
		case FORMAT_YV12_BT709:
		case FORMAT_YV12_JFIF:
			return true;
		case FORMAT_R5G6B5:
		case FORMAT_R8I_SNORM:
		case FORMAT_G8R8I_SNORM:
		case FORMAT_X8B8G8R8I_SNORM:
		case FORMAT_A8B8G8R8I_SNORM:
		case FORMAT_R8I:
		case FORMAT_R8UI:
		case FORMAT_G8R8I:
		case FORMAT_G8R8UI:
		case FORMAT_X8B8G8R8I:
		case FORMAT_X8B8G8R8UI:
		case FORMAT_A8B8G8R8I:
		case FORMAT_A8B8G8R8UI:
		case FORMAT_R32I:
		case FORMAT_R32UI:
		case FORMAT_G32R32I:
		case FORMAT_G32R32UI:
		case FORMAT_X32B32G32R32I:
		case FORMAT_X32B32G32R32UI:
		case FORMAT_A32B32G32R32I:
		case FORMAT_A32B32G32R32UI:
		case FORMAT_G8R8:
		case FORMAT_X8R8G8B8:
		case FORMAT_X8B8G8R8:
		case FORMAT_A8R8G8B8:
		case FORMAT_A8B8G8R8:
		case FORMAT_SRGB8_X8:
		case FORMAT_SRGB8_A8:
		case FORMAT_V8U8:
		case FORMAT_Q8W8V8U8:
		case FORMAT_X8L8V8U8:
		case FORMAT_R32F:
		case FORMAT_G32R32F:
		case FORMAT_X32B32G32R32F:
		case FORMAT_A32B32G32R32F:
		case FORMAT_A8:
		case FORMAT_R8:
		case FORMAT_L8:
		case FORMAT_A8L8:
		case FORMAT_D32F:
		case FORMAT_D32F_LOCKABLE:
		case FORMAT_D32FS8_TEXTURE:
		case FORMAT_D32FS8_SHADOW:
		case FORMAT_L16:
		case FORMAT_G16R16:
		case FORMAT_A16B16G16R16:
		case FORMAT_R16I:
		case FORMAT_R16UI:
		case FORMAT_G16R16I:
		case FORMAT_G16R16UI:
		case FORMAT_X16B16G16R16I:
		case FORMAT_X16B16G16R16UI:
		case FORMAT_A16B16G16R16I:
		case FORMAT_A16B16G16R16UI:
		case FORMAT_V16U16:
		case FORMAT_A16W16V16U16:
		case FORMAT_Q16W16V16U16:
			return false;
		default:
			ASSERT(false);
		}

		return false;
	}

	bool SamplerCore::isRGBComponent(int component) const
	{
		switch(state.textureFormat)
		{
		case FORMAT_R5G6B5:         return component < 3;
		case FORMAT_R8I_SNORM:      return component < 1;
		case FORMAT_G8R8I_SNORM:    return component < 2;
		case FORMAT_X8B8G8R8I_SNORM: return component < 3;
		case FORMAT_A8B8G8R8I_SNORM: return component < 3;
		case FORMAT_R8I:            return component < 1;
		case FORMAT_R8UI:           return component < 1;
		case FORMAT_G8R8I:          return component < 2;
		case FORMAT_G8R8UI:         return component < 2;
		case FORMAT_X8B8G8R8I:      return component < 3;
		case FORMAT_X8B8G8R8UI:     return component < 3;
		case FORMAT_A8B8G8R8I:      return component < 3;
		case FORMAT_A8B8G8R8UI:     return component < 3;
		case FORMAT_R32I:           return component < 1;
		case FORMAT_R32UI:          return component < 1;
		case FORMAT_G32R32I:        return component < 2;
		case FORMAT_G32R32UI:       return component < 2;
		case FORMAT_X32B32G32R32I:  return component < 3;
		case FORMAT_X32B32G32R32UI: return component < 3;
		case FORMAT_A32B32G32R32I:  return component < 3;
		case FORMAT_A32B32G32R32UI: return component < 3;
		case FORMAT_G8R8:           return component < 2;
		case FORMAT_X8R8G8B8:       return component < 3;
		case FORMAT_X8B8G8R8:       return component < 3;
		case FORMAT_A8R8G8B8:       return component < 3;
		case FORMAT_A8B8G8R8:       return component < 3;
		case FORMAT_SRGB8_X8:       return component < 3;
		case FORMAT_SRGB8_A8:       return component < 3;
		case FORMAT_V8U8:           return false;
		case FORMAT_Q8W8V8U8:       return false;
		case FORMAT_X8L8V8U8:       return false;
		case FORMAT_R32F:           return component < 1;
		case FORMAT_G32R32F:        return component < 2;
		case FORMAT_X32B32G32R32F:  return component < 3;
		case FORMAT_A32B32G32R32F:  return component < 3;
		case FORMAT_A8:             return false;
		case FORMAT_R8:             return component < 1;
		case FORMAT_L8:             return component < 1;
		case FORMAT_A8L8:           return component < 1;
		case FORMAT_D32F:           return false;
		case FORMAT_D32F_LOCKABLE:  return false;
		case FORMAT_D32FS8_TEXTURE: return false;
		case FORMAT_D32FS8_SHADOW:  return false;
		case FORMAT_L16:            return component < 1;
		case FORMAT_G16R16:         return component < 2;
		case FORMAT_A16B16G16R16:   return component < 3;
		case FORMAT_R16I:           return component < 1;
		case FORMAT_R16UI:          return component < 1;
		case FORMAT_G16R16I:        return component < 2;
		case FORMAT_G16R16UI:       return component < 2;
		case FORMAT_X16B16G16R16I:  return component < 3;
		case FORMAT_X16B16G16R16UI: return component < 3;
		case FORMAT_A16B16G16R16I:  return component < 3;
		case FORMAT_A16B16G16R16UI: return component < 3;
		case FORMAT_V16U16:         return false;
		case FORMAT_A16W16V16U16:   return false;
		case FORMAT_Q16W16V16U16:   return false;
		case FORMAT_YV12_BT601:     return component < 3;
		case FORMAT_YV12_BT709:     return component < 3;
		case FORMAT_YV12_JFIF:      return component < 3;
		default:
			ASSERT(false);
		}

		return false;
	}
}
