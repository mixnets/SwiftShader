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

// mathutil.h: Math and bit manipulation functions.

#ifndef LIBGLESV2_MATHUTIL_H_
#define LIBGLESV2_MATHUTIL_H_

#include "common/debug.h"
#include "Common/Math.hpp"

namespace es2
{
inline bool isPow2(int x)
{
    return (x & (x - 1)) == 0 && (x != 0);
}

inline int log2(int x)
{
    int r = 0;
    while((x >> r) > 1) r++;
    return r;
}

inline unsigned int ceilPow2(unsigned int x)
{
    if(x != 0) x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x++;

    return x;
}

using sw::swap;
using sw::clamp;
using sw::clamp01;

template<const int n>
inline unsigned int unorm(float x)
{
    const unsigned int max = 0xFFFFFFFF >> (32 - n);

    if(x > 1)
    {
        return max;
    }
    else if(x < 0)
    {
        return 0;
    }
    else
    {
        return (unsigned int)(max * x + 0.5f);
    }
}

// Returns the smallest floating-point value for which conversion to integer is 0x7FFFFFFF
inline float max_float_int()
{
	int imax = 0x7FFFFFFF - 1;

	while(static_cast<int>(roundf(static_cast<float>(imax))) != imax)
	{
		imax--;
	}

	float max = static_cast<float>(imax);

	do
	{
		max = nextafterf(max, 1e10f);
	}
	while(static_cast<int>(roundf(max)) == imax);

	return max;
}

// Returns the largest floating-point value for which conversion to integer is 0x80000000
inline float min_float_int()
{
	int imin = 0x80000000 + 1;

	while(static_cast<int>(roundf(static_cast<float>(imin))) != imin)
	{
		imin++;
	}

	float min = static_cast<float>(imin);

	do
	{
		min = nextafterf(min, -1e10f);
	}
	while(static_cast<int>(roundf(min)) == imin);

	return min;
}

// Returns the smallest floating-point value for which conversion to integer is 0xFFFFFFFF
inline float max_float_uint()
{
	unsigned int imax = 0xFFFFFFFFu - 1;

	while(static_cast<unsigned int>(roundf(static_cast<float>(imax))) != imax)
	{
		imax--;
	}

	float max = static_cast<float>(imax);

	do
	{
		max = nextafterf(max, 1e10f);
	}
	while(static_cast<unsigned int>(roundf(max)) == imax);

	return max;
}

inline unsigned int convert_float_uint(float x)
{
	static const float max = max_float_uint();

	if(x >= max)
	{
		return 0xFFFFFFFF;
	}

	return static_cast<unsigned int>(roundf(x));
}

inline int convert_float_int(float x)
{
	static const float min = min_float_int();
	static const float max = max_float_int();

	if(x >= max)
	{
		return 0x7FFFFFFF;
	}

	if(x <= min)
	{
		return 0x80000000;
	}

	return static_cast<int>(roundf(x));
}

template<int b>
unsigned int convert_float_unorm(float x)
{
	const unsigned int scale = 0xFFFFFFFF >> (32 - b);

	return convert_float_uint(sw::clamp(x, 0.0f, 1.0f) * scale);
}

template<int b>
int convert_float_norm(float x)
{
	const unsigned int scale = 0x7FFFFFFF >> (32 - b);

	return convert_float_int(sw::clamp(x, -1.0f, 1.0f) * scale);
}

}

#endif   // LIBGLESV2_MATHUTIL_H_
