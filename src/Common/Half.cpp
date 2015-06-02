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

#include "Half.hpp"
#include "Types.hpp"

namespace sw
{
	class Float16Compressor
	{
		union Bits
		{
			float f;
			int32_t si;
			uint32_t ui;
		};

		static int const shift = 13;
		static int const shiftSign = 16;

		static int32_t const infN = 0x7F800000; // flt32 infinity
		static int32_t const maxN = 0x477FE000; // max flt16 normal as a flt32
		static int32_t const minN = 0x38800000; // min flt16 normal as a flt32
		static int32_t const signN = 0x80000000; // flt32 sign bit

		static int32_t const infC = infN >> shift;
		static int32_t const nanN = (infC + 1) << shift; // minimum flt16 nan as a flt32
		static int32_t const maxC = maxN >> shift;
		static int32_t const minC = minN >> shift;
		static int32_t const signC = signN >> shiftSign; // flt16 sign bit

		static int32_t const mulN = 0x52000000; // (1 << 23) / minN
		static int32_t const mulC = 0x33800000; // minN / (1 << (23 - shift))

		static int32_t const subC = 0x003FF; // max flt32 subnormal down shifted
		static int32_t const norC = 0x00400; // min flt32 normal down shifted

		static int32_t const maxD = infC - maxC - 1;
		static int32_t const minD = minC - subC - 1;

	public:

		static uint16_t compress(float value)
		{
			Bits v, s;
			v.f = value;
			uint32_t sign = v.si & signN;
			v.si ^= sign;
			sign >>= shiftSign; // logical shift
			s.si = mulN;
			s.si = static_cast<int32_t>(s.f * v.f); // correct subnormals
			v.si ^= (s.si ^ v.si) & -(minN > v.si);
			v.si ^= (infN ^ v.si) & -((infN > v.si) & (v.si > maxN));
			v.si ^= (nanN ^ v.si) & -((nanN > v.si) & (v.si > infN));
			v.ui >>= shift; // logical shift
			v.si ^= ((v.si - maxD) ^ v.si) & -(v.si > maxC);
			v.si ^= ((v.si - minD) ^ v.si) & -(v.si > subC);
			return v.ui | sign;
		}

		static float decompress(uint16_t value)
		{
			Bits v;
			v.ui = value;
			int32_t sign = v.si & signC;
			v.si ^= sign;
			sign <<= shiftSign;
			v.si ^= ((v.si + minD) ^ v.si) & -(v.si > subC);
			v.si ^= ((v.si + maxD) ^ v.si) & -(v.si > maxC);
			Bits s;
			s.si = mulC;
			s.f *= v.si;
			int32_t mask = -(norC > v.si);
			v.si <<= shift;
			v.si ^= (s.si ^ v.si) & mask;
			v.si |= sign;
			return v.f;
		}
	};

	bool half::TestConversionEquivalence()
	{
		for(uint32_t i = 0; i < 0xFFFF; ++i)
		{
			uint16_t value = static_cast<uint16_t>(i);
			float result1 = Float16Compressor::decompress(value);
			half h = reinterpret_cast<half&>(value);
			float result2 = float(h);
			if(reinterpret_cast<uint32_t&>(result1) != reinterpret_cast<uint32_t&>(result2))
			{
				if(!isnan(result1) && !isinf(result1))
				{
					return false;
				}
			}

			half half1(result1);
			uint16_t half2 = Float16Compressor::compress(result1);
			if(reinterpret_cast<uint16_t&>(half1) != half2)
			{
				if(!isnan(result1) && !isinf(result1))
				{
					return false;
				}
			}
		}

		return true;
	}

	half::half(float fp32)
	{
        unsigned int fp32i = *(unsigned int*)&fp32;
        unsigned int sign = (fp32i & 0x80000000) >> 16;
        unsigned int abs = fp32i & 0x7FFFFFFF;

        if(abs > 0x47FFEFFF)   // Infinity
        {
            fp16i = sign | 0x7FFF;
        }
        else if(abs < 0x38800000)   // Denormal
        {
            unsigned int mantissa = (abs & 0x007FFFFF) | 0x00800000;   
            int e = 113 - (abs >> 23);

            if(e < 24)
            {
                abs = mantissa >> e;
            }
            else
            {
                abs = 0;
            }

            fp16i = sign | (abs + 0x00000FFF + ((abs >> 13) & 1)) >> 13;
        }
        else
        {
            fp16i = sign | (abs + 0xC8000000 + 0x00000FFF + ((abs >> 13) & 1)) >> 13;
        }
	}

	half::operator float() const
	{
		unsigned int fp32i;

		int s = (fp16i >> 15) & 0x00000001;
		int e = (fp16i >> 10) & 0x0000001F;
		int m =  fp16i        & 0x000003FF;

		if(e == 0)
		{
			if(m == 0)
			{
				fp32i = s << 31;
			
				return (float&)fp32i;
			}
			else
			{
				while(!(m & 0x00000400))
				{
					m <<= 1;
					e -=  1;
				}

				e += 1;
				m &= ~0x00000400;
			}
		}

		e = e + (127 - 15);
		m = m << 13;

		fp32i = (s << 31) | (e << 23) | m;

		return (float&)fp32i;
	}

	half &half::operator=(half h)
	{
		fp16i = h.fp16i;
		
		return *this;
	}


	half &half::operator=(float f)
	{
		*this = half(f);

		return *this;
	}
}
