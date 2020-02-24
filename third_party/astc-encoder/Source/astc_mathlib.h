// SPDX-License-Identifier: Apache-2.0
// ----------------------------------------------------------------------------
// Copyright 2011-2020 Arm Limited
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not
// use this file except in compliance with the License. You may obtain a copy
// of the License at:
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
// License for the specific language governing permissions and limitations
// under the License.
// ----------------------------------------------------------------------------

/*
 * This module implements a variety of mathematical data types and library
 * functions used by the codec.
 */

#ifndef ASTC_MATHLIB_H_INCLUDED
#define ASTC_MATHLIB_H_INCLUDED

#include <cstdint>
#include <cmath>

#include <immintrin.h>

#ifndef M_PI
	#define M_PI 3.14159265358979323846
#endif

/* ============================================================================
  Fast math library; note that many of the higher-order functions in this set
  use approximations which are less accurate, but faster, than <cmath> standard
  library equivalents.

  Note: Many of these are not necessarily faster than simple C versions when
  used on a single scalar value, but are included for testing purposes as most
  have an option based on SSE intrinsics and therefore provide an obvious route
  to future vectorization.
============================================================================ */

// We support scalar versions of many maths functions which use SSE intrinsics
// as an "optimized" path, using just one lane from the SIMD hardware. In
// reality these are often slower than standard C due to setup and scheduling
// overheads, and the fact that we're not offsetting that cost with any actual
// vectorization.
//
// These variants are only included as a means to test that the accuracy of an
// SSE implementation would be acceptable before refactoring code paths to use
// an actual vectorized implementation which gets some advantage from SSE. It
// is therefore expected that the code will go *slower* with this macro
// set to 1 ...
#define USE_SCALAR_SSE 0

// These are namespaced to avoid colliding with C standard library functions.
namespace astc
{

/**
 * @brief Test if a float value is a nan.
 *
 * @param val The value test.
 *
 * @return Zero is not a NaN, non-zero otherwise.
 */
static inline int isnan(float val)
{
	return val != val;
}

/**
 * @brief Initialize the seed structure for a random number generator.
 *
 * Important note: For the purposes of ASTC we want sets of random numbers to
 * use the codec, but we want the same seed value across instances and threads
 * to ensure that image output is stable across compressor runs and across
 * platforms. Every PRNG created by this call will therefore return the same
 * sequence of values ...
 *
 * @param state The state structure to initialize.
 */
void rand_init(uint64_t state[2]);

/**
 * @brief Return the next random number from the generator.
 *
 * This RNG is an implementation of the "xoroshoro-128+ 1.0" PRNG, based on the
 * public-domain implementation given by David Blackman & Sebastiano Vigna at
 * http://vigna.di.unimi.it/xorshift/xoroshiro128plus.c
 *
 * @param state The state structure to use/update.
 */
uint64_t rand(uint64_t state[2]);

}

/* ============================================================================
  Utility vector template classes with basic operations
============================================================================ */

template <typename T> class vtype2
{
public:
	T x, y;
	vtype2() {}
	vtype2(T p, T q)         : x(p),   y(q)   {}
	vtype2(const vtype2 & p) : x(p.x), y(p.y) {}
};

template <typename T> class vtype3
{
public:
	T x, y, z;
	vtype3() {}
	vtype3(T p, T q, T r)    : x(p),   y(q),   z(r)   {}
	vtype3(const vtype3 & p) : x(p.x), y(p.y), z(p.z) {}
};

template <typename T> class vtype4
{
public:
	T x, y, z, w;
	vtype4() {}
	vtype4(T p, T q, T r, T s) : x(p),   y(q),   z(r),   w(s)   {}
	vtype4(const vtype4 & p)   : x(p.x), y(p.y), z(p.z), w(p.w) {}
};

typedef vtype2<float>        float2;
typedef vtype3<float>        float3;
typedef vtype4<float>        float4;
typedef vtype3<int>          int3;
typedef vtype4<int>          int4;
typedef vtype4<unsigned int> uint4;

static inline float2  operator+(float2 p,  float2 q)   { return float2(  p.x + q.x, p.y + q.y ); }
static inline float3  operator+(float3 p,  float3 q)   { return float3(  p.x + q.x, p.y + q.y, p.z + q.z ); }
static inline float4  operator+(float4 p,  float4 q)   { return float4(  p.x + q.x, p.y + q.y, p.z + q.z, p.w + q.w ); }
static inline int4    operator+(int4 p,    int4 q)     { return int4(    p.x + q.x, p.y + q.y, p.z + q.z, p.w + q.w ); }
static inline uint4   operator+(uint4 p,   uint4 q)    { return uint4(   p.x + q.x, p.y + q.y, p.z + q.z, p.w + q.w ); }

static inline float2  operator-(float2 p,  float2 q)   { return float2(  p.x - q.x, p.y - q.y ); }
static inline float3  operator-(float3 p,  float3 q)   { return float3(  p.x - q.x, p.y - q.y, p.z - q.z ); }
static inline float4  operator-(float4 p,  float4 q)   { return float4(  p.x - q.x, p.y - q.y, p.z - q.z, p.w - q.w ); }
static inline int4    operator-(int4 p,    int4 q)     { return int4(    p.x - q.x, p.y - q.y, p.z - q.z, p.w - q.w ); }
static inline uint4   operator-(uint4 p,   uint4 q)    { return uint4(   p.x - q.x, p.y - q.y, p.z - q.z, p.w - q.w ); }

static inline float2  operator*(float2 p,  float2 q)   { return float2(  p.x * q.x, p.y * q.y ); }
static inline float3  operator*(float3 p,  float3 q)   { return float3(  p.x * q.x, p.y * q.y, p.z * q.z ); }
static inline float4  operator*(float4 p,  float4 q)   { return float4(  p.x * q.x, p.y * q.y, p.z * q.z, p.w * q.w ); }
static inline int4    operator*(int4 p,    int4 q)     { return int4(    p.x * q.x, p.y * q.y, p.z * q.z, p.w * q.w ); }
static inline uint4   operator*(uint4 p,   uint4 q)    { return uint4(   p.x * q.x, p.y * q.y, p.z * q.z, p.w * q.w ); }

static inline float2  operator*(float2 p,  float q)    { return float2(  p.x * q, p.y * q ); }
static inline float3  operator*(float3 p,  float q)    { return float3(  p.x * q, p.y * q, p.z * q ); }
static inline float4  operator*(float4 p,  float q)    { return float4(  p.x * q, p.y * q, p.z * q, p.w * q ); }
static inline int4    operator*(int4 p,    int q)      { return int4(    p.x * q, p.y * q, p.z * q, p.w * q ); }
static inline uint4   operator*(uint4 p,   uint32_t q) { return uint4(   p.x * q, p.y * q, p.z * q, p.w * q ); }

static inline float2  operator*(float p,    float2 q)  { return q * p; }
static inline float3  operator*(float p,    float3 q)  { return q * p; }
static inline float4  operator*(float p,    float4 q)  { return q * p; }
static inline int4    operator*(int p,      int4 q)    { return q * p; }
static inline uint4   operator*(uint32_t p, uint4 q)   { return q * p; }

static inline float dot(float2 p, float2 q)  { return p.x * q.x + p.y * q.y; }
static inline float dot(float3 p, float3 q)  { return p.x * q.x + p.y * q.y + p.z * q.z; }
static inline float dot(float4 p, float4 q)  { return p.x * q.x + p.y * q.y + p.z * q.z + p.w * q.w; }

#ifndef MIN
	#define MIN(x,y) ((x)<(y)?(x):(y))
#endif

#ifndef MAX
	#define MAX(x,y) ((x)>(y)?(x):(y))
#endif

/* ============================================================================
  Softfloat library with fp32 and fp16 conversion functionality.
============================================================================ */
typedef union if32_
{
	uint32_t u;
	int32_t s;
	float f;
} if32;

uint32_t clz32(uint32_t p);

/*	sized soft-float types. These are mapped to the sized integer
    types of C99, instead of C's floating-point types; this is because
    the library needs to maintain exact, bit-level control on all
    operations on these data types. */
typedef uint16_t sf16;
typedef uint32_t sf32;

/* the five rounding modes that IEEE-754r defines */
typedef enum
{
	SF_UP = 0,				/* round towards positive infinity */
	SF_DOWN = 1,			/* round towards negative infinity */
	SF_TOZERO = 2,			/* round towards zero */
	SF_NEARESTEVEN = 3,		/* round toward nearest value; if mid-between, round to even value */
	SF_NEARESTAWAY = 4		/* round toward nearest value; if mid-between, round away from zero */
} roundmode;

/* narrowing float->float conversions */
sf16 sf32_to_sf16(sf32, roundmode);

/* widening float->float conversions */
sf32 sf16_to_sf32(sf16);

sf16 float_to_sf16(float, roundmode);

float sf16_to_float(sf16);

#endif
