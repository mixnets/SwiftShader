// Copyright 2019 The SwiftShader Authors. All Rights Reserved.
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

#include "BC_Decoder.hpp"

#include "Common/Math.hpp"
#include "System/Debug.hpp"

#include <algorithm>
#include <array>
#include <cstddef>

#include <assert.h>
#include <stdint.h>

namespace {
static constexpr int BlockWidth = 4;
static constexpr int BlockHeight = 4;

struct BC_color
{
	void decode(uint8_t *dst, int x, int y, int dstW, int dstH, int dstPitch, int dstBpp, bool hasAlphaChannel, bool hasSeparateAlpha) const
	{
		Color c[4];
		c[0].extract565(c0);
		c[1].extract565(c1);
		if(hasSeparateAlpha || (c0 > c1))
		{
			c[2] = ((c[0] * 2) + c[1]) / 3;
			c[3] = ((c[1] * 2) + c[0]) / 3;
		}
		else
		{
			c[2] = (c[0] + c[1]) >> 1;
			if(hasAlphaChannel)
			{
				c[3].clearAlpha();
			}
		}

		for(int j = 0; j < BlockHeight && (y + j) < dstH; j++)
		{
			int dstOffset = j * dstPitch;
			int idxOffset = j * BlockHeight;
			for(int i = 0; i < BlockWidth && (x + i) < dstW; i++, idxOffset++, dstOffset += dstBpp)
			{
				*reinterpret_cast<unsigned int *>(dst + dstOffset) = c[getIdx(idxOffset)].pack8888();
			}
		}
	}

private:
	struct Color
	{
		Color()
		{
			c[0] = c[1] = c[2] = 0;
			c[3] = 0xFF000000;
		}

		void extract565(const unsigned int c565)
		{
			c[0] = ((c565 & 0x0000001F) << 3) | ((c565 & 0x0000001C) >> 2);
			c[1] = ((c565 & 0x000007E0) >> 3) | ((c565 & 0x00000600) >> 9);
			c[2] = ((c565 & 0x0000F800) >> 8) | ((c565 & 0x0000E000) >> 13);
		}

		unsigned int pack8888() const
		{
			return ((c[2] & 0xFF) << 16) | ((c[1] & 0xFF) << 8) | (c[0] & 0xFF) | c[3];
		}

		void clearAlpha()
		{
			c[3] = 0;
		}

		Color operator*(int factor) const
		{
			Color res;
			for(int i = 0; i < 4; ++i)
			{
				res.c[i] = c[i] * factor;
			}
			return res;
		}

		Color operator/(int factor) const
		{
			Color res;
			for(int i = 0; i < 4; ++i)
			{
				res.c[i] = c[i] / factor;
			}
			return res;
		}

		Color operator>>(int shift) const
		{
			Color res;
			for(int i = 0; i < 4; ++i)
			{
				res.c[i] = c[i] >> shift;
			}
			return res;
		}

		Color operator+(Color const &obj) const
		{
			Color res;
			for(int i = 0; i < 4; ++i)
			{
				res.c[i] = c[i] + obj.c[i];
			}
			return res;
		}

	private:
		int c[4];
	};

	unsigned int getIdx(int i) const
	{
		int offset = i << 1;  // 2 bytes per index
		return (idx & (0x3 << offset)) >> offset;
	}

	unsigned short c0;
	unsigned short c1;
	unsigned int idx;
};

struct BC_channel
{
	void decode(uint8_t *dst, int x, int y, int dstW, int dstH, int dstPitch, int dstBpp, int channel, bool isSigned) const
	{
		int c[8] = { 0 };

		if(isSigned)
		{
			c[0] = static_cast<signed char>(data & 0xFF);
			c[1] = static_cast<signed char>((data & 0xFF00) >> 8);
		}
		else
		{
			c[0] = static_cast<uint8_t>(data & 0xFF);
			c[1] = static_cast<uint8_t>((data & 0xFF00) >> 8);
		}

		if(c[0] > c[1])
		{
			for(int i = 2; i < 8; ++i)
			{
				c[i] = ((8 - i) * c[0] + (i - 1) * c[1]) / 7;
			}
		}
		else
		{
			for(int i = 2; i < 6; ++i)
			{
				c[i] = ((6 - i) * c[0] + (i - 1) * c[1]) / 5;
			}
			c[6] = isSigned ? -128 : 0;
			c[7] = isSigned ? 127 : 255;
		}

		for(int j = 0; j < BlockHeight && (y + j) < dstH; j++)
		{
			for(int i = 0; i < BlockWidth && (x + i) < dstW; i++)
			{
				dst[channel + (i * dstBpp) + (j * dstPitch)] = static_cast<uint8_t>(c[getIdx((j * BlockHeight) + i)]);
			}
		}
	}

private:
	uint8_t getIdx(int i) const
	{
		int offset = i * 3 + 16;
		return static_cast<uint8_t>((data & (0x7ull << offset)) >> offset);
	}

	uint64_t data;
};

struct BC_alpha
{
	void decode(uint8_t *dst, int x, int y, int dstW, int dstH, int dstPitch, int dstBpp) const
	{
		dst += 3;  // Write only to alpha (channel 3)
		for(int j = 0; j < BlockHeight && (y + j) < dstH; j++, dst += dstPitch)
		{
			uint8_t *dstRow = dst;
			for(int i = 0; i < BlockWidth && (x + i) < dstW; i++, dstRow += dstBpp)
			{
				*dstRow = getAlpha(j * BlockHeight + i);
			}
		}
	}

private:
	uint8_t getAlpha(int i) const
	{
		int offset = i << 2;
		int alpha = (data & (0xFull << offset)) >> offset;
		return static_cast<uint8_t>(alpha | (alpha << 4));
	}

	uint64_t data;
};

namespace BC6h {

static constexpr int MaxPartitions = 64;
static constexpr int MaxSubsets = 2;

static constexpr uint8_t PartitionTable2[MaxPartitions][16] = {
	{ 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1 },
	{ 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1 },
	{ 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1 },
	{ 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 1 },
	{ 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1 },
	{ 0, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1 },
	{ 0, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1 },
	{ 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 1 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1 },
	{ 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
	{ 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1 },
	{ 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1 },
	{ 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1 },
	{ 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1 },
	{ 0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0 },
	{ 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 },
	{ 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0 },
	{ 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1 },
	{ 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0 },
	{ 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0 },
	{ 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 0 },
	{ 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0 },
	{ 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0 },
	{ 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 0 },
	{ 0, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0 },
	{ 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1 },
	{ 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1 },
	{ 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0 },
	{ 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0 },
	{ 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0 },
	{ 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0 },
	{ 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1 },
	{ 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1 },
	{ 0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 0 },
	{ 0, 0, 0, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0 },
	{ 0, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 0, 0 },
	{ 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0 },
	{ 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0 },
	{ 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1 },
	{ 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1 },
	{ 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0 },
	{ 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0 },
	{ 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0 },
	{ 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1 },
	{ 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1 },
	{ 0, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0 },
	{ 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0 },
	{ 0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 1 },
	{ 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1 },
	{ 0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1 },
	{ 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1 },
	{ 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1 },
	{ 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0 },
	{ 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0 },
	{ 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1 },
};

static constexpr uint8_t AnchorTable2[MaxPartitions] = {
	// clang-format off
	0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf,
	0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf,
	0xf, 0x2, 0x8, 0x2, 0x2, 0x8, 0x8, 0xf,
	0x2, 0x8, 0x2, 0x2, 0x8, 0x8, 0x2, 0x2,
	0xf, 0xf, 0x6, 0x8, 0x2, 0x8, 0xf, 0xf,
	0x2, 0x8, 0x2, 0x2, 0x2, 0xf, 0xf, 0x6,
	0x6, 0x2, 0x6, 0x8, 0xf, 0xf, 0x2, 0x2,
	0xf, 0xf, 0xf, 0xf, 0xf, 0x2, 0x2, 0xf,
	// clang-format on
};

const uint16_t halfFloat1 = 0x3C00;

struct Color
{
    struct RGBA {
        uint16_t r;
        uint16_t g;
        uint16_t b;
        // Should always be 1.0 in half-float precision (or 0x3C00)
        uint16_t a;

        RGBA(uint16_t r, uint16_t g, uint16_t b)
            : r(r)
            , g(g)
            , b(b)
            , a(halfFloat1)
        {
        }

        RGBA()
            : r(0)
            , g(0)
            , b(0)
            , a(halfFloat1)
        {
        }
		RGBA(int r, int g, int b)
		    : r(static_cast<uint16_t>(r))
		    , g(static_cast<uint16_t>(g))
		    , b(static_cast<uint16_t>(b))
            , a(halfFloat1)
		{}

        RGBA(int r, int g, int b, int a)
		    : r(static_cast<uint16_t>(r))
		    , g(static_cast<uint16_t>(g))
		    , b(static_cast<uint16_t>(b))
            , a(static_cast<uint16_t>(a))
        {
        }

    };

    Color(uint16_t r, uint16_t g, uint16_t b) :
        rgba(r, g, b)
    {
    }

    Color() = default;

    RGBA rgba;
};

struct RGBf
{
    uint16_t r;
    uint16_t g;
    uint16_t b;

    size_t rSize;
    size_t gSize;
    size_t bSize;

    RGBf() = default;


    void ExtendSign()
    {
        // Suppose we have a 2-bit integer being stored in 4 bit variable:
        //    x = 0b00AB
        //
        // In order to sign extend x, we need to turn the 0s into As:
        //    x_extend = 0bAAAB
        //
        // We can do that by flipping A in x then subtracting 0b10 from x.
        // Suppose A is 1:
        //    x       = 0b001B
        //    x_flip  = 0b000B
        //    x_minus = 0b000B - 0b0010 = 0b111B
        // Since A is flipped to 0, subtracting the mask sets it and all the bits above it to 1.
        // And if A is 0:
        //    x       = 0b000B
        //    x_flip  = 0b001B
        //    x_minus = 0b001B - 0b0010 = 0b000B
        // All we do is unset the bit we flipped, and we're left with an unchanged number.
        uint16_t rMask = 1u << (rSize - 1);
        uint16_t gMask = 1u << (gSize - 1);
        uint16_t bMask = 1u << (bSize - 1);

        r = (r ^ rMask) - rMask;
        g = (g ^ gMask) - gMask;
        b = (b ^ bMask) - bMask;
    }

    // Assuming this is an endpoint, take a delta and calculate its proper endpoint.
    // This function assumes both the endpoint and delta have been properly sign extended.
    //
    // The final computed endpoint is truncated to the base endpoint's size;
    RGBf ResolveDelta(RGBf delta)
    {
        RGBf ret;

        ret.rSize = rSize;
        ret.gSize = gSize;
        ret.bSize = bSize;

        ret.r = (r + delta.r) & ((1 << rSize) - 1);
        ret.g = (g + delta.g) & ((1 << gSize) - 1);
        ret.b = (b + delta.b) & ((1 << bSize) - 1);

        return ret;
    }

    void UnquantizeUnsigned()
    {
        auto UnquantizeValue = [](uint16_t value, size_t size)
        {
            uint32_t ret = 0;
            if (size >= 15 || value == 0)
            {
                ret = value;
            }
            else if (value == ((1 << size) - 1))
            {
                ret = 0xFFFF;
            }
            else
            {
                // Need 32 bits to avoid overflow.
                uint32_t tmp = value;
                ret = ((tmp << 15) + 0x4000) >> (size - 1);
            }

            // Truncate top 16 bits when returning.
            return (uint16_t) ret;
        };

        r = UnquantizeValue(r, rSize);
        g = UnquantizeValue(g, rSize);
        b = UnquantizeValue(b, rSize);
    }

    void UnquantizeSigned()
    {
        auto UnquantizeValue = [](uint16_t value, size_t size)
        {
            int32_t ret = 0;

            if (size >= 16 || value == 0)
            {
                ret = value;
            }
            else
            {
                int16_t sValue = sw::bit_cast<int16_t>(value);
                bool signBit = false;
                if (sValue < 0)
                {
                    signBit = true;
                    sValue = -sValue;
                }

                if (sValue >= ((1 << (size - 1)) - 1))
                {
                    ret = 0x7FFF;
                }
                else
                {
                    // Need 32-bits of space to avoid overflow.
                    int32_t tmp = sValue;
                    ret = ((tmp << 15) + 0x4000) >> (size - 1);
                }

                if (signBit)
                {
                    ret = -ret;
                }
            }

            // Truncate top 16 bits when returning.
            return (uint16_t) ret;
        };

        r = UnquantizeValue(r, rSize);
        g = UnquantizeValue(g, rSize);
        b = UnquantizeValue(b, rSize);
    }
};

struct Data
{
    uint64_t low64;
    uint64_t high64;

    Data() = default;
    Data(uint64_t low64, uint64_t high64) :
        low64(low64), high64(high64)
    {
    }

    // Consumes the lowest N bits from from low64 and high64 where N is:
    //      abs(MSB - LSB)
    // MSB and LSB come from the block description of the BC6h spec and specify
    // the location of the bits in the returned bitstring.
    //
    // If MSB < LSB, then the bits are reversed. Otherwise, the bitstring is read and
    // shifted without further modification.
    //
    uint32_t ConsumeBits(uint32_t MSB, uint32_t LSB)
    {
        uint32_t bits = 0;
        uint32_t numBits = 0;
        uint32_t shift;
        bool reversed = MSB < LSB;

        if (reversed)
        {
            std::swap(MSB, LSB);
        }

        numBits = MSB - LSB + 1;
        shift = LSB;

        if (numBits >= 8*sizeof(bits) - 1)
        {
            numBits = 8*sizeof(bits) - 1;
        }

        uint32_t mask = (1 << numBits) - 1;
        // Read the low N bits
        bits = (low64 & mask);
        // Shift the 128-bit number down N bits
        low64 >>= numBits;
        low64 |= (high64 & mask) << (sizeof(high64) * 8 - numBits);
        high64 >>= numBits;

        if (reversed)
        {
            uint32_t tmp = 0;
            for (uint32_t numSwaps = numBits; numSwaps > 0; numSwaps--)
            {
                tmp <<= 1;
                tmp |= (bits & 1);
                bits >>= 1;
            }

            bits = tmp;
        }
        return bits << shift;
    }
};

struct IndexInfo
{
    uint64_t value;
    int numBits;
};

uint16_t interpolate(uint16_t e0, uint16_t e1, const IndexInfo &index, bool isSigned)
{
    static constexpr uint16_t weights3[] = { 0, 9, 18, 27, 37, 46, 55, 64 };
    static constexpr uint16_t weights4[] = { 0, 4, 9, 13, 17, 21, 26, 30,
                                             34, 38, 43, 47, 51, 55, 60, 64 };
    static constexpr uint16_t const *weightsN[] = {
        nullptr, nullptr, nullptr, weights3, weights4
    };
    auto weights = weightsN[index.numBits];
    ASSERT_MSG(weights != nullptr, "Unexpected number of index bits: %d", (int)index.numBits);
    uint16_t value = (uint16_t)(((64 - weights[index.value]) * uint16_t(e0) + weights[index.value] * uint16_t(e1) + 32) >> 6);

    // Need to unquantize value to limit it to the legal range of half-precision float values
    // We do this by scaling by 31/32 or 31/64 depending on if the value is signed or unsigned.
    // In order to prevent loss of precision, we need to do these calculations in 32-bit.
    if (isSigned)
    {
        int16_t sValue = sw::bit_cast<int16_t>(value);
        int32_t tmp = std::abs(sValue);
        int signBit = value >> 15;
        // Scale abs(tmp) by 31/32, then restore the sign-bit
        tmp = ((tmp * 31) >> 5) | (signBit << 15);
        return (uint16_t) tmp;
    }
    else
    {
        uint32_t tmp = value;
        // Scale unsigned values by 31/64
        return (uint16_t) (tmp * 31) >> 6;
    }
}

struct Block
{
    // BC6h blocks are composed of 128 bits in little endian order per-the-spec:
    // https://www.khronos.org/registry/DataFormat/specs/1.3/dataformat.1.3.html#_bc6h
    uint64_t low64;
    uint64_t high64;

	void decode(uint8_t *dst, int dstX, int dstY, int dstWidth, int dstHeight, size_t dstPitch, bool isSigned) const
	{
        uint8_t mode = 0;
        Data data(low64, high64);

        if ((data.low64 & 0x2) == 0)
        {
            mode = data.ConsumeBits(2, 0);
        }
        else
        {
            mode = data.ConsumeBits(5, 0);
        }

        // The 4 potential endpoints we'll be using
        RGBf E0, E1, E2, E3;
        int partitionCount = 2;
        int partition = 0;
        bool hasDeltaBits = true;
        switch (mode)
        {
            // These numbers come from the block descriptions for BC6h
            case 0:
                E0.rSize = E0.gSize = E0.bSize = 10;
                E1.rSize = E2.rSize = E3.rSize = 5;
                E1.gSize = E2.gSize = E3.gSize = 5;
                E1.bSize = E2.bSize = E3.bSize = 5;

                E2.g |= data.ConsumeBits(4, 4);
                E2.b |= data.ConsumeBits(4, 4);
                E3.b |= data.ConsumeBits(4, 4);
                E0.r |= data.ConsumeBits(9, 0);
                E0.g |= data.ConsumeBits(9, 0);

                E0.b |= data.ConsumeBits(9, 0);
                E1.r |= data.ConsumeBits(4, 0);
                E3.g |= data.ConsumeBits(4, 4);
                E2.g |= data.ConsumeBits(3, 0);
                E1.g |= data.ConsumeBits(4, 0);

                E3.b |= data.ConsumeBits(0, 0);
                E3.g |= data.ConsumeBits(3, 0);
                E1.b |= data.ConsumeBits(4, 0);
                E3.b |= data.ConsumeBits(1, 1);
                E2.b |= data.ConsumeBits(3, 0);

                E2.r |= data.ConsumeBits(4, 0);
                E3.b |= data.ConsumeBits(2, 2);
                E3.r |= data.ConsumeBits(4, 0);
                E3.b |= data.ConsumeBits(3, 3);
                partition |= data.ConsumeBits(4, 0);
                break;
            case 1:
                E0.rSize = E0.gSize = E0.bSize = 7;
                E1.rSize = E2.rSize = E3.rSize = 6;
                E1.gSize = E2.gSize = E3.gSize = 6;
                E1.bSize = E2.bSize = E3.bSize = 6;

                E2.g |= data.ConsumeBits(5, 5);
                E3.g |= data.ConsumeBits(4, 5);
                E0.r |= data.ConsumeBits(6, 0);
                E3.b |= data.ConsumeBits(0, 1);
                E2.b |= data.ConsumeBits(4, 4);

                E0.g |= data.ConsumeBits(6, 0);
                E2.b |= data.ConsumeBits(5, 5);
                E3.b |= data.ConsumeBits(2, 2);
                E2.g |= data.ConsumeBits(4, 4);
                E0.b |= data.ConsumeBits(6, 0);

                E3.b |= data.ConsumeBits(3, 3);
                E3.b |= data.ConsumeBits(5, 4);
                E1.r |= data.ConsumeBits(5, 0);
                E2.g |= data.ConsumeBits(3, 0);
                E1.g |= data.ConsumeBits(5, 0);

                E3.g |= data.ConsumeBits(3, 0);
                E1.b |= data.ConsumeBits(5, 0);
                E2.b |= data.ConsumeBits(3, 0);
                E2.r |= data.ConsumeBits(5, 0);
                E3.r |= data.ConsumeBits(5, 0);

                partition |= data.ConsumeBits(4, 0);
                break;
            case 2:
                E0.rSize = E0.gSize = E0.bSize = 11;
                E1.rSize = E2.rSize = E3.rSize = 5;
                E1.gSize = E2.gSize = E3.gSize = 4;
                E1.bSize = E2.bSize = E3.bSize = 4;

                E0.r |= data.ConsumeBits(9, 0);
                E0.g |= data.ConsumeBits(9, 0);
                E0.b |= data.ConsumeBits(9, 0);
                E1.r |= data.ConsumeBits(4, 0);
                E0.r |= data.ConsumeBits(10, 10);

                E2.g |= data.ConsumeBits(3, 0);
                E1.g |= data.ConsumeBits(3, 0);
                E0.g |= data.ConsumeBits(10, 10);
                E3.b |= data.ConsumeBits(0, 0);
                E3.g |= data.ConsumeBits(3, 0);

                E1.b |= data.ConsumeBits(3, 0);
                E0.b |= data.ConsumeBits(10, 10);
                E3.b |= data.ConsumeBits(1, 1);
                E2.b |= data.ConsumeBits(3, 0);
                E2.r |= data.ConsumeBits(4, 0);

                E3.b |= data.ConsumeBits(2, 2);
                E3.r |= data.ConsumeBits(4, 0);
                E3.b |= data.ConsumeBits(3, 3);
                partition |= data.ConsumeBits(4, 0);
                break;
            case 6:
                E0.rSize = E0.gSize = E0.bSize = 11;
                E1.rSize = E2.rSize = E3.rSize = 4;
                E1.gSize = E2.gSize = E3.gSize = 5;
                E1.bSize = E2.bSize = E3.bSize = 4;

                E0.r |= data.ConsumeBits(9, 0);
                E0.g |= data.ConsumeBits(9, 0);
                E0.b |= data.ConsumeBits(9, 0);
                E1.r |= data.ConsumeBits(3, 0);
                E0.r |= data.ConsumeBits(10, 10);

                E3.g |= data.ConsumeBits(4, 4);
                E2.g |= data.ConsumeBits(3, 0);
                E1.g |= data.ConsumeBits(4, 0);
                E0.g |= data.ConsumeBits(10, 10);
                E3.g |= data.ConsumeBits(3, 0);

                E1.b |= data.ConsumeBits(3, 0);
                E0.b |= data.ConsumeBits(10, 10);
                E3.b |= data.ConsumeBits(1, 1);
                E2.b |= data.ConsumeBits(3, 0);
                E2.r |= data.ConsumeBits(3, 0);

                E3.b |= data.ConsumeBits(0, 0);
                E3.b |= data.ConsumeBits(2, 2);
                E3.r |= data.ConsumeBits(3, 0);
                E2.g |= data.ConsumeBits(4, 4);
                E3.b |= data.ConsumeBits(3, 3);

                partition |= data.ConsumeBits(4, 0);
                break;
            case 10:
                E0.rSize = E0.gSize = E0.bSize = 11;
                E1.rSize = E2.rSize = E3.rSize = 4;
                E1.gSize = E2.gSize = E3.gSize = 4;
                E1.bSize = E2.bSize = E3.bSize = 5;

                E0.r |= data.ConsumeBits(9, 0);
                E0.g |= data.ConsumeBits(9, 0);
                E0.b |= data.ConsumeBits(9, 0);
                E1.r |= data.ConsumeBits(3, 0);
                E0.r |= data.ConsumeBits(10, 10);

                E2.b |= data.ConsumeBits(4, 0);
                E2.g |= data.ConsumeBits(3, 0);
                E1.g |= data.ConsumeBits(3, 0);
                E0.g |= data.ConsumeBits(10, 10);
                E3.b |= data.ConsumeBits(0, 0);

                E3.g |= data.ConsumeBits(3, 0);
                E1.b |= data.ConsumeBits(4, 0);
                E0.b |= data.ConsumeBits(10, 10);
                E2.b |= data.ConsumeBits(3, 0);
                E2.r |= data.ConsumeBits(3, 0);

                E3.b |= data.ConsumeBits(1, 2);
                E3.r |= data.ConsumeBits(3, 0);
                E3.b |= data.ConsumeBits(4, 4);
                E3.b |= data.ConsumeBits(3, 3);
                partition |= data.ConsumeBits(4, 0);
                break;
            case 14:
                E0.rSize = E0.gSize = E0.bSize = 9;
                E1.rSize = E2.rSize = E3.rSize = 5;
                E1.gSize = E2.gSize = E3.gSize = 5;
                E1.bSize = E2.bSize = E3.bSize = 5;

                E0.r |= data.ConsumeBits(8, 0);
                E2.b |= data.ConsumeBits(4, 4);
                E0.g |= data.ConsumeBits(8, 0);
                E2.g |= data.ConsumeBits(4, 4);
                E0.b |= data.ConsumeBits(8, 0);

                E3.b |= data.ConsumeBits(4, 4);
                E1.r |= data.ConsumeBits(4, 0);
                E3.g |= data.ConsumeBits(4, 0);
                E2.g |= data.ConsumeBits(3, 0);
                E1.g |= data.ConsumeBits(4, 0);

                E3.b |= data.ConsumeBits(0, 0);
                E3.g |= data.ConsumeBits(3, 0);
                E1.b |= data.ConsumeBits(4, 0);
                E3.b |= data.ConsumeBits(1, 1);
                E2.b |= data.ConsumeBits(3, 0);

                E2.r |= data.ConsumeBits(4, 0);
                E3.b |= data.ConsumeBits(2, 2);
                E3.r |= data.ConsumeBits(4, 0);
                E3.b |= data.ConsumeBits(3, 3);
                partition |= data.ConsumeBits(4, 0);
                break;
            case 18:
                E0.rSize = E0.gSize = E0.bSize = 8;
                E1.rSize = E2.rSize = E3.rSize = 6;
                E1.gSize = E2.gSize = E3.gSize = 5;
                E1.bSize = E2.bSize = E3.bSize = 5;

                E0.r |= data.ConsumeBits(7, 0);
                E3.g |= data.ConsumeBits(4, 4);
                E2.b |= data.ConsumeBits(4, 4);
                E0.g |= data.ConsumeBits(7, 0);
                E3.b |= data.ConsumeBits(2, 2);

                E2.g |= data.ConsumeBits(4, 4);
                E0.b |= data.ConsumeBits(7, 0);
                E3.b |= data.ConsumeBits(3, 4);
                E1.r |= data.ConsumeBits(5, 0);
                E2.g |= data.ConsumeBits(3, 0);

                E1.g |= data.ConsumeBits(4, 0);
                E3.b |= data.ConsumeBits(3, 3);
                E3.g |= data.ConsumeBits(3, 0);
                E1.b |= data.ConsumeBits(4, 0);
                E3.b |= data.ConsumeBits(1, 1);

                E2.b |= data.ConsumeBits(3, 0);
                E2.r |= data.ConsumeBits(5, 0);
                E3.r |= data.ConsumeBits(5, 0);
                partition |= data.ConsumeBits(4, 0);
                break;
            case 22:
                E0.rSize = E0.gSize = E0.bSize = 8;
                E1.rSize = E2.rSize = E3.rSize = 5;
                E1.gSize = E2.gSize = E3.gSize = 6;
                E1.bSize = E2.bSize = E3.bSize = 5;

                E0.r |= data.ConsumeBits(7, 0);
                E3.b |= data.ConsumeBits(0, 0);
                E2.b |= data.ConsumeBits(4, 4);
                E0.g |= data.ConsumeBits(7, 0);
                E2.g |= data.ConsumeBits(5, 4);

                E0.b |= data.ConsumeBits(7, 0);
                E3.g |= data.ConsumeBits(5, 5);
                E3.b |= data.ConsumeBits(4, 4);
                E1.r |= data.ConsumeBits(4, 0);
                E3.g |= data.ConsumeBits(4, 4);

                E2.g |= data.ConsumeBits(3, 0);
                E1.g |= data.ConsumeBits(5, 0);
                E3.g |= data.ConsumeBits(3, 0);
                E1.b |= data.ConsumeBits(4, 0);
                E3.b |= data.ConsumeBits(1, 1);

                E2.b |= data.ConsumeBits(3, 0);
                E2.r |= data.ConsumeBits(4, 0);
                E3.b |= data.ConsumeBits(2, 2);
                E3.r |= data.ConsumeBits(4, 0);
                E3.b |= data.ConsumeBits(3, 3);

                partition |= data.ConsumeBits(4, 0);
                break;
            case 26:
                E0.rSize = E0.gSize = E0.bSize = 8;
                E1.rSize = E2.rSize = E3.rSize = 5;
                E1.gSize = E2.gSize = E3.gSize = 5;
                E1.bSize = E2.bSize = E3.bSize = 6;

                E0.r |= data.ConsumeBits(7, 0);
                E3.b |= data.ConsumeBits(1, 1);
                E2.b |= data.ConsumeBits(4, 4);
                E0.g |= data.ConsumeBits(7, 0);
                E2.b |= data.ConsumeBits(5, 5);

                E2.g |= data.ConsumeBits(4, 4);
                E0.b |= data.ConsumeBits(7, 0);
                E3.b |= data.ConsumeBits(5, 4);
                E1.r |= data.ConsumeBits(4, 0);
                E3.g |= data.ConsumeBits(4, 4);

                E2.g |= data.ConsumeBits(3, 0);
                E1.g |= data.ConsumeBits(4, 0);
                E3.b |= data.ConsumeBits(0, 0);
                E3.g |= data.ConsumeBits(3, 0);
                E1.b |= data.ConsumeBits(5, 0);

                E2.b |= data.ConsumeBits(3, 0);
                E2.r |= data.ConsumeBits(4, 0);
                E3.b |= data.ConsumeBits(2, 2);
                E3.r |= data.ConsumeBits(4, 0);
                E3.b |= data.ConsumeBits(3, 3);

                partition |= data.ConsumeBits(4, 0);
                break;
            case 30:
                hasDeltaBits = false;
                E0.rSize = E0.gSize = E0.bSize = 6;
                E1.rSize = E2.rSize = E3.rSize = 6;
                E1.gSize = E2.gSize = E3.gSize = 6;
                E1.bSize = E2.bSize = E3.bSize = 6;

                hasDeltaBits = false;
                E0.r |= data.ConsumeBits(5, 0);
                E3.g |= data.ConsumeBits(4, 4);
                E3.b |= data.ConsumeBits(0, 1);
                E2.b |= data.ConsumeBits(4, 4);
                E0.g |= data.ConsumeBits(5, 0);

                E2.g |= data.ConsumeBits(5, 5);
                E2.b |= data.ConsumeBits(5, 5);
                E3.b |= data.ConsumeBits(2, 2);
                E2.g |= data.ConsumeBits(4, 4);
                E0.b |= data.ConsumeBits(5, 0);

                E3.g |= data.ConsumeBits(5, 5);
                E3.b |= data.ConsumeBits(3, 3);
                E3.b |= data.ConsumeBits(5, 4);
                E1.r |= data.ConsumeBits(5, 0);
                E2.g |= data.ConsumeBits(3, 0);

                E1.g |= data.ConsumeBits(5, 0);
                E3.g |= data.ConsumeBits(3, 0);
                E1.b |= data.ConsumeBits(5, 0);
                E2.b |= data.ConsumeBits(3, 0);
                E2.r |= data.ConsumeBits(5, 0);

                E3.r |= data.ConsumeBits(5, 0);
                partition |= data.ConsumeBits(4, 0);
                break;
            case 3:
                hasDeltaBits = false;
                partitionCount = 1;
                E0.rSize = E0.gSize = E0.bSize = 10;
                E1.rSize = E1.gSize = E1.bSize = 10;

                E0.r |= data.ConsumeBits(9, 0);
                E0.g |= data.ConsumeBits(9, 0);
                E0.b |= data.ConsumeBits(9, 0);

                E1.r |= data.ConsumeBits(9, 0);
                E1.g |= data.ConsumeBits(9, 0);
                E1.b |= data.ConsumeBits(9, 0);
                break;
            case 7:
                partitionCount = 1;
                E0.rSize = E0.gSize = E0.bSize = 11;
                E1.rSize = E1.gSize = E1.bSize = 9;

                E0.r |= data.ConsumeBits(9, 0);
                E0.g |= data.ConsumeBits(9, 0);
                E0.b |= data.ConsumeBits(9, 0);

                E1.r |= data.ConsumeBits(8, 0);
                E0.r |= data.ConsumeBits(10, 10);
                E1.g |= data.ConsumeBits(8, 0);
                E0.g |= data.ConsumeBits(10, 10);
                E1.b |= data.ConsumeBits(8, 0);
                E0.b |= data.ConsumeBits(10, 10);
                break;
            case 11:
                partitionCount = 1;
                E0.rSize = E0.gSize = E0.bSize = 12;
                E1.rSize = E1.gSize = E1.bSize = 8;

                E0.r |= data.ConsumeBits(9, 0);
                E0.g |= data.ConsumeBits(9, 0);
                E0.b |= data.ConsumeBits(9, 0);

                E1.r |= data.ConsumeBits(7, 0);
                E0.r |= data.ConsumeBits(10, 11);
                E1.g |= data.ConsumeBits(7, 0);
                E0.g |= data.ConsumeBits(10, 11);
                E1.b |= data.ConsumeBits(7, 0);
                E0.b |= data.ConsumeBits(10, 11);
                break;
            case 15:
                partitionCount = 1;
                E0.rSize = E0.gSize = E0.bSize = 16;
                E1.rSize = E1.gSize = E1.bSize = 4;

                E0.r |= data.ConsumeBits(9, 0);
                E0.g |= data.ConsumeBits(9, 0);
                E0.b |= data.ConsumeBits(9, 0);

                E1.r |= data.ConsumeBits(3, 0);
                E0.r |= data.ConsumeBits(10, 15);
                E1.g |= data.ConsumeBits(3, 0);
                E0.g |= data.ConsumeBits(10, 15);
                E1.b |= data.ConsumeBits(3, 0);
                E0.b |= data.ConsumeBits(10, 15);
                break;
            default:
                // Reserved or invalid mode
                for(int y = 0; y < 4 && y + dstY < dstHeight; y++)
                {
                    for(int x = 0; x < 4 && x + dstX < dstWidth; x++)
                    {
                        auto out = reinterpret_cast<Color *>(dst + sizeof(Color) * x + dstPitch * y);
                        out->rgba = { 0, 0, 0, halfFloat1 };
                    }
                }
                return;
        }

        // Sign extension
        if (isSigned)
        {
            E0.ExtendSign();
            E1.ExtendSign();
            if (partitionCount == 2)
            {
                E2.ExtendSign();
                E3.ExtendSign();
            }
        }
        else if (hasDeltaBits)
        {
            E1.ExtendSign();
            if (partitionCount == 2)
            {
                E2.ExtendSign();
                E3.ExtendSign();
            }
        }

        // Resolving deltas into endpoints
        if (hasDeltaBits)
        {
            E1 = E0.ResolveDelta(E1);
            if (partitionCount == 2)
            {
                E2 = E0.ResolveDelta(E2);
                E3 = E0.ResolveDelta(E3);
            }
        }

        // Unquantizing endpoints
        if (isSigned)
        {
            E0.UnquantizeSigned();
            E1.UnquantizeSigned();
            if (partitionCount == 2)
            {
                E2.UnquantizeSigned();
                E3.UnquantizeSigned();
            }
        }
        else
        {
            E0.UnquantizeUnsigned();
            E1.UnquantizeUnsigned();
            if (partitionCount == 2)
            {
                E2.UnquantizeUnsigned();
                E3.UnquantizeUnsigned();
            }
        }

        // Get the indices, calculate final colors, and output
        for (int y = 0; y < 4; y++)
        {
            for (int x = 0; x < 4; x++)
            {
                size_t bitsToConsume = 0;
                IndexInfo idx;
                // Get the next index
                if (partitionCount == 1)
                {
                    idx.numBits = 4;
                    bitsToConsume = idx.numBits;
                    // There's an implicit leading 0 bit for the first idx
                    if (x == 0 && y == 0)
                    {
                        bitsToConsume -= 1;
                    }
                }
                else
                {
                    idx.numBits = 3;
                    bitsToConsume = idx.numBits;
                    // There are 2 indices with implicit leading 0-bits.
                    if ((x == 0 && y == 0) || ((x + 4 * y) == AnchorTable2[partition]))
                    {
                        bitsToConsume -= 1;
                    }
                }

                idx.value = data.ConsumeBits(bitsToConsume, 0);

                Color color;
                if (partitionCount == 1 || PartitionTable2[partition][idx.value] == 0)
                {
                    color.rgba.r = interpolate(E0.r, E1.r, idx, isSigned);
                    color.rgba.g = interpolate(E0.g, E1.g, idx, isSigned);
                    color.rgba.b = interpolate(E0.b, E1.b, idx, isSigned);
                }
                else // if (partitionCount == 2 && PartitionTable2[...][...] == 1)
                {
                    color.rgba.r = interpolate(E2.r, E3.r, idx, isSigned);
                    color.rgba.g = interpolate(E2.g, E3.g, idx, isSigned);
                    color.rgba.b = interpolate(E2.b, E3.b, idx, isSigned);
                }

                auto out = reinterpret_cast<Color *>(dst + sizeof(Color) * x + dstPitch * y);
                *out = color;
            }
        }
	}

};

} // namespace BC6h

namespace BC7 {
// https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_texture_compression_bptc.txt
// https://docs.microsoft.com/en-us/windows/win32/direct3d11/bc7-format

struct Bitfield
{
	int offset;
	int count;
	constexpr Bitfield Then(const int bits) { return { offset + count, bits }; }
	constexpr bool operator==(const Bitfield &rhs)
	{
		return offset == rhs.offset && count == rhs.count;
	}
};

struct Mode
{
	const int IDX;  // Mode index
	const int NS;   // Number of subsets in each partition
	const int PB;   // Partition bits
	const int RB;   // Rotation bits
	const int ISB;  // Index selection bits
	const int CB;   // Color bits
	const int AB;   // Alpha bits
	const int EPB;  // Endpoint P-bits
	const int SPB;  // Shared P-bits
	const int IB;   // Primary index bits per element
	const int IBC;  // Primary index bits total
	const int IB2;  // Secondary index bits per element

	constexpr int NumColors() const { return NS * 2; }
	constexpr Bitfield Partition() const { return { IDX + 1, PB }; }
	constexpr Bitfield Rotation() const { return Partition().Then(RB); }
	constexpr Bitfield IndexSelection() const { return Rotation().Then(ISB); }
	constexpr Bitfield Red(int idx) const
	{
		return IndexSelection().Then(CB * idx).Then(CB);
	}
	constexpr Bitfield Green(int idx) const
	{
		return Red(NumColors() - 1).Then(CB * idx).Then(CB);
	}
	constexpr Bitfield Blue(int idx) const
	{
		return Green(NumColors() - 1).Then(CB * idx).Then(CB);
	}
	constexpr Bitfield Alpha(int idx) const
	{
		return Blue(NumColors() - 1).Then(AB * idx).Then(AB);
	}
	constexpr Bitfield EndpointPBit(int idx) const
	{
		return Alpha(NumColors() - 1).Then(EPB * idx).Then(EPB);
	}
	constexpr Bitfield SharedPBit0() const
	{
		return EndpointPBit(NumColors() - 1).Then(SPB);
	}
	constexpr Bitfield SharedPBit1() const
	{
		return SharedPBit0().Then(SPB);
	}
	constexpr Bitfield PrimaryIndex(int offset, int count) const
	{
		return SharedPBit1().Then(offset).Then(count);
	}
	constexpr Bitfield SecondaryIndex(int offset, int count) const
	{
		return SharedPBit1().Then(IBC + offset).Then(count);
	}
};

static constexpr Mode Modes[] = {
	//     IDX  NS   PB   RB   ISB  CB   AB   EPB  SPB  IB   IBC, IB2
	/**/ { 0x0, 0x3, 0x4, 0x0, 0x0, 0x4, 0x0, 0x1, 0x0, 0x3, 0x2d, 0x0 },
	/**/ { 0x1, 0x2, 0x6, 0x0, 0x0, 0x6, 0x0, 0x0, 0x1, 0x3, 0x2e, 0x0 },
	/**/ { 0x2, 0x3, 0x6, 0x0, 0x0, 0x5, 0x0, 0x0, 0x0, 0x2, 0x1d, 0x0 },
	/**/ { 0x3, 0x2, 0x6, 0x0, 0x0, 0x7, 0x0, 0x1, 0x0, 0x2, 0x1e, 0x0 },
	/**/ { 0x4, 0x1, 0x0, 0x2, 0x1, 0x5, 0x6, 0x0, 0x0, 0x2, 0x1f, 0x3 },
	/**/ { 0x5, 0x1, 0x0, 0x2, 0x0, 0x7, 0x8, 0x0, 0x0, 0x2, 0x1f, 0x2 },
	/**/ { 0x6, 0x1, 0x0, 0x0, 0x0, 0x7, 0x7, 0x1, 0x0, 0x4, 0x3f, 0x0 },
	/**/ { 0x7, 0x2, 0x6, 0x0, 0x0, 0x5, 0x5, 0x1, 0x0, 0x2, 0x1e, 0x0 },
	/**/ { -1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x00, 0x0 },
};

static_assert(Modes[0].NumColors() == 6, "BC7 sanity checks failed");
static_assert(Modes[0].Partition() == Bitfield{ 1, 4 }, "BC7 sanity checks failed");
static_assert(Modes[0].Red(0) == Bitfield{ 5, 4 }, "BC7 sanity checks failed");
static_assert(Modes[0].Red(5) == Bitfield{ 25, 4 }, "BC7 sanity checks failed");
static_assert(Modes[0].Green(0) == Bitfield{ 29, 4 }, "BC7 sanity checks failed");
static_assert(Modes[0].Green(5) == Bitfield{ 49, 4 }, "BC7 sanity checks failed");
static_assert(Modes[0].Blue(0) == Bitfield{ 53, 4 }, "BC7 sanity checks failed");
static_assert(Modes[0].Blue(5) == Bitfield{ 73, 4 }, "BC7 sanity checks failed");
static_assert(Modes[0].EndpointPBit(0) == Bitfield{ 77, 1 }, "BC7 sanity checks failed");
static_assert(Modes[0].EndpointPBit(5) == Bitfield{ 82, 1 }, "BC7 sanity checks failed");
static_assert(Modes[0].PrimaryIndex(0, 2) == Bitfield{ 83, 2 }, "BC7 sanity checks failed");
static_assert(Modes[0].PrimaryIndex(43, 1) == Bitfield{ 126, 1 }, "BC7 sanity checks failed");

static constexpr int MaxPartitions = 64;
static constexpr int MaxSubsets = 3;

static constexpr uint8_t PartitionTable2[MaxPartitions][16] = {
	{ 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1 },
	{ 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1 },
	{ 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1 },
	{ 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 1 },
	{ 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1 },
	{ 0, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1 },
	{ 0, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1 },
	{ 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 1 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1 },
	{ 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
	{ 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1 },
	{ 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1 },
	{ 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1 },
	{ 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1 },
	{ 0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0 },
	{ 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 },
	{ 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0 },
	{ 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1 },
	{ 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0 },
	{ 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0 },
	{ 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 0 },
	{ 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0 },
	{ 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0 },
	{ 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 0 },
	{ 0, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0 },
	{ 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1 },
	{ 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1 },
	{ 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0 },
	{ 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0 },
	{ 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0 },
	{ 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0 },
	{ 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1 },
	{ 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1 },
	{ 0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 0 },
	{ 0, 0, 0, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0 },
	{ 0, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 0, 0 },
	{ 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0 },
	{ 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0 },
	{ 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1 },
	{ 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1 },
	{ 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0 },
	{ 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0 },
	{ 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0 },
	{ 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1 },
	{ 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1 },
	{ 0, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0 },
	{ 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0 },
	{ 0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 1 },
	{ 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1 },
	{ 0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1 },
	{ 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1 },
	{ 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1 },
	{ 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0 },
	{ 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0 },
	{ 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1 },
};

static constexpr uint8_t PartitionTable3[MaxPartitions][16] = {
	{ 0, 0, 1, 1, 0, 0, 1, 1, 0, 2, 2, 1, 2, 2, 2, 2 },
	{ 0, 0, 0, 1, 0, 0, 1, 1, 2, 2, 1, 1, 2, 2, 2, 1 },
	{ 0, 0, 0, 0, 2, 0, 0, 1, 2, 2, 1, 1, 2, 2, 1, 1 },
	{ 0, 2, 2, 2, 0, 0, 2, 2, 0, 0, 1, 1, 0, 1, 1, 1 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 2, 1, 1, 2, 2 },
	{ 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 2, 2, 0, 0, 2, 2 },
	{ 0, 0, 2, 2, 0, 0, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1 },
	{ 0, 0, 1, 1, 0, 0, 1, 1, 2, 2, 1, 1, 2, 2, 1, 1 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2 },
	{ 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2 },
	{ 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2 },
	{ 0, 0, 1, 2, 0, 0, 1, 2, 0, 0, 1, 2, 0, 0, 1, 2 },
	{ 0, 1, 1, 2, 0, 1, 1, 2, 0, 1, 1, 2, 0, 1, 1, 2 },
	{ 0, 1, 2, 2, 0, 1, 2, 2, 0, 1, 2, 2, 0, 1, 2, 2 },
	{ 0, 0, 1, 1, 0, 1, 1, 2, 1, 1, 2, 2, 1, 2, 2, 2 },
	{ 0, 0, 1, 1, 2, 0, 0, 1, 2, 2, 0, 0, 2, 2, 2, 0 },
	{ 0, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 2, 1, 1, 2, 2 },
	{ 0, 1, 1, 1, 0, 0, 1, 1, 2, 0, 0, 1, 2, 2, 0, 0 },
	{ 0, 0, 0, 0, 1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2 },
	{ 0, 0, 2, 2, 0, 0, 2, 2, 0, 0, 2, 2, 1, 1, 1, 1 },
	{ 0, 1, 1, 1, 0, 1, 1, 1, 0, 2, 2, 2, 0, 2, 2, 2 },
	{ 0, 0, 0, 1, 0, 0, 0, 1, 2, 2, 2, 1, 2, 2, 2, 1 },
	{ 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 2, 2, 0, 1, 2, 2 },
	{ 0, 0, 0, 0, 1, 1, 0, 0, 2, 2, 1, 0, 2, 2, 1, 0 },
	{ 0, 1, 2, 2, 0, 1, 2, 2, 0, 0, 1, 1, 0, 0, 0, 0 },
	{ 0, 0, 1, 2, 0, 0, 1, 2, 1, 1, 2, 2, 2, 2, 2, 2 },
	{ 0, 1, 1, 0, 1, 2, 2, 1, 1, 2, 2, 1, 0, 1, 1, 0 },
	{ 0, 0, 0, 0, 0, 1, 1, 0, 1, 2, 2, 1, 1, 2, 2, 1 },
	{ 0, 0, 2, 2, 1, 1, 0, 2, 1, 1, 0, 2, 0, 0, 2, 2 },
	{ 0, 1, 1, 0, 0, 1, 1, 0, 2, 0, 0, 2, 2, 2, 2, 2 },
	{ 0, 0, 1, 1, 0, 1, 2, 2, 0, 1, 2, 2, 0, 0, 1, 1 },
	{ 0, 0, 0, 0, 2, 0, 0, 0, 2, 2, 1, 1, 2, 2, 2, 1 },
	{ 0, 0, 0, 0, 0, 0, 0, 2, 1, 1, 2, 2, 1, 2, 2, 2 },
	{ 0, 2, 2, 2, 0, 0, 2, 2, 0, 0, 1, 2, 0, 0, 1, 1 },
	{ 0, 0, 1, 1, 0, 0, 1, 2, 0, 0, 2, 2, 0, 2, 2, 2 },
	{ 0, 1, 2, 0, 0, 1, 2, 0, 0, 1, 2, 0, 0, 1, 2, 0 },
	{ 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 0, 0, 0, 0 },
	{ 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0 },
	{ 0, 1, 2, 0, 2, 0, 1, 2, 1, 2, 0, 1, 0, 1, 2, 0 },
	{ 0, 0, 1, 1, 2, 2, 0, 0, 1, 1, 2, 2, 0, 0, 1, 1 },
	{ 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 0, 0, 0, 0, 1, 1 },
	{ 0, 1, 0, 1, 0, 1, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 2, 1, 2, 1, 2, 1 },
	{ 0, 0, 2, 2, 1, 1, 2, 2, 0, 0, 2, 2, 1, 1, 2, 2 },
	{ 0, 0, 2, 2, 0, 0, 1, 1, 0, 0, 2, 2, 0, 0, 1, 1 },
	{ 0, 2, 2, 0, 1, 2, 2, 1, 0, 2, 2, 0, 1, 2, 2, 1 },
	{ 0, 1, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 0, 1, 0, 1 },
	{ 0, 0, 0, 0, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1 },
	{ 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 2, 2, 2, 2 },
	{ 0, 2, 2, 2, 0, 1, 1, 1, 0, 2, 2, 2, 0, 1, 1, 1 },
	{ 0, 0, 0, 2, 1, 1, 1, 2, 0, 0, 0, 2, 1, 1, 1, 2 },
	{ 0, 0, 0, 0, 2, 1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2 },
	{ 0, 2, 2, 2, 0, 1, 1, 1, 0, 1, 1, 1, 0, 2, 2, 2 },
	{ 0, 0, 0, 2, 1, 1, 1, 2, 1, 1, 1, 2, 0, 0, 0, 2 },
	{ 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 2, 2, 2, 2 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 1, 2, 2, 1, 1, 2 },
	{ 0, 1, 1, 0, 0, 1, 1, 0, 2, 2, 2, 2, 2, 2, 2, 2 },
	{ 0, 0, 2, 2, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 2, 2 },
	{ 0, 0, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2, 0, 0, 2, 2 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 1, 2 },
	{ 0, 0, 0, 2, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 1 },
	{ 0, 2, 2, 2, 1, 2, 2, 2, 0, 2, 2, 2, 1, 2, 2, 2 },
	{ 0, 1, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 },
	{ 0, 1, 1, 1, 2, 0, 1, 1, 2, 2, 0, 1, 2, 2, 2, 0 },
};

static constexpr uint8_t AnchorTable2[MaxPartitions] = {
	// clang-format off
	0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf,
	0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf,
	0xf, 0x2, 0x8, 0x2, 0x2, 0x8, 0x8, 0xf,
	0x2, 0x8, 0x2, 0x2, 0x8, 0x8, 0x2, 0x2,
	0xf, 0xf, 0x6, 0x8, 0x2, 0x8, 0xf, 0xf,
	0x2, 0x8, 0x2, 0x2, 0x2, 0xf, 0xf, 0x6,
	0x6, 0x2, 0x6, 0x8, 0xf, 0xf, 0x2, 0x2,
	0xf, 0xf, 0xf, 0xf, 0xf, 0x2, 0x2, 0xf,
	// clang-format on
};

static constexpr uint8_t AnchorTable3a[MaxPartitions] = {
	// clang-format off
	0x3, 0x3, 0xf, 0xf, 0x8, 0x3, 0xf, 0xf,
	0x8, 0x8, 0x6, 0x6, 0x6, 0x5, 0x3, 0x3,
	0x3, 0x3, 0x8, 0xf, 0x3, 0x3, 0x6, 0xa,
	0x5, 0x8, 0x8, 0x6, 0x8, 0x5, 0xf, 0xf,
	0x8, 0xf, 0x3, 0x5, 0x6, 0xa, 0x8, 0xf,
	0xf, 0x3, 0xf, 0x5, 0xf, 0xf, 0xf, 0xf,
	0x3, 0xf, 0x5, 0x5, 0x5, 0x8, 0x5, 0xa,
	0x5, 0xa, 0x8, 0xd, 0xf, 0xc, 0x3, 0x3,
	// clang-format on
};

static constexpr uint8_t AnchorTable3b[MaxPartitions] = {
	// clang-format off
	0xf, 0x8, 0x8, 0x3, 0xf, 0xf, 0x3, 0x8,
	0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0x8,
	0xf, 0x8, 0xf, 0x3, 0xf, 0x8, 0xf, 0x8,
	0x3, 0xf, 0x6, 0xa, 0xf, 0xf, 0xa, 0x8,
	0xf, 0x3, 0xf, 0xa, 0xa, 0x8, 0x9, 0xa,
	0x6, 0xf, 0x8, 0xf, 0x3, 0x6, 0x6, 0x8,
	0xf, 0x3, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf,
	0xf, 0xf, 0xf, 0xf, 0x3, 0xf, 0xf, 0x8,
	// clang-format on
};

struct Color
{
	struct RGB
	{
		RGB() = default;
		RGB(uint8_t r, uint8_t g, uint8_t b)
		    : b(b)
		    , g(g)
		    , r(r)
		{}
		RGB(int r, int g, int b)
		    : b(static_cast<uint8_t>(b))
		    , g(static_cast<uint8_t>(g))
		    , r(static_cast<uint8_t>(r))
		{}

		RGB operator<<(int shift) const { return { r << shift, g << shift, b << shift }; }
		RGB operator>>(int shift) const { return { r >> shift, g >> shift, b >> shift }; }
		RGB operator|(int bits) const { return { r | bits, g | bits, b | bits }; }
		RGB operator|(const RGB &rhs) const { return { r | rhs.r, g | rhs.g, b | rhs.b }; }
		RGB operator+(const RGB &rhs) const { return { r + rhs.r, g + rhs.g, b + rhs.b }; }

		uint8_t b;
		uint8_t g;
		uint8_t r;
	};

	RGB rgb;
	uint8_t a;
};

static_assert(sizeof(Color) == 4, "Color size must be 4 bytes");

struct Block
{
	constexpr uint64_t Get(const Bitfield &bf) const
	{
		uint64_t mask = (1ULL << bf.count) - 1;
		if(bf.offset + bf.count <= 64)
		{
			return (low >> bf.offset) & mask;
		}
		if(bf.offset >= 64)
		{
			return (high >> (bf.offset - 64)) & mask;
		}
		return ((low >> bf.offset) | (high << (64 - bf.offset))) & mask;
	}

	const Mode &mode() const
	{
		if((low & 0b00000001) != 0) { return Modes[0]; }
		if((low & 0b00000010) != 0) { return Modes[1]; }
		if((low & 0b00000100) != 0) { return Modes[2]; }
		if((low & 0b00001000) != 0) { return Modes[3]; }
		if((low & 0b00010000) != 0) { return Modes[4]; }
		if((low & 0b00100000) != 0) { return Modes[5]; }
		if((low & 0b01000000) != 0) { return Modes[6]; }
		if((low & 0b10000000) != 0) { return Modes[7]; }
		return Modes[8];  // Invalid mode
	}

	struct IndexInfo
	{
		uint64_t value;
		int numBits;
	};

	uint8_t interpolate(uint8_t e0, uint8_t e1, const IndexInfo &index) const
	{
		static constexpr uint16_t weights2[] = { 0, 21, 43, 64 };
		static constexpr uint16_t weights3[] = { 0, 9, 18, 27, 37, 46, 55, 64 };
		static constexpr uint16_t weights4[] = { 0, 4, 9, 13, 17, 21, 26, 30,
			                                     34, 38, 43, 47, 51, 55, 60, 64 };
		static constexpr uint16_t const *weightsN[] = {
			nullptr, nullptr, weights2, weights3, weights4
		};
		auto weights = weightsN[index.numBits];
		ASSERT_MSG(weights != nullptr, "Unexpected number of index bits: %d", (int)index.numBits);
		return (uint8_t)(((64 - weights[index.value]) * uint16_t(e0) + weights[index.value] * uint16_t(e1) + 32) >> 6);
	}

	void decode(uint8_t *dst, int dstX, int dstY, int dstWidth, int dstHeight, size_t dstPitch) const
	{
		auto const &mode = this->mode();

		if(mode.IDX < 0)  // Invalid mode:
		{
			for(int y = 0; y < 4 && y + dstY < dstHeight; y++)
			{
				for(int x = 0; x < 4 && x + dstX < dstWidth; x++)
				{
					auto out = reinterpret_cast<Color *>(dst + sizeof(Color) * x + dstPitch * y);
					out->rgb = { 0, 0, 0 };
					out->a = 0;
				}
			}
			return;
		}

		using Endpoint = std::array<Color, 2>;
		std::array<Endpoint, MaxSubsets> subsets;

		for(int i = 0; i < mode.NS; i++)
		{
			auto &subset = subsets[i];
			subset[0].rgb.r = Get(mode.Red(i * 2 + 0));
			subset[0].rgb.g = Get(mode.Green(i * 2 + 0));
			subset[0].rgb.b = Get(mode.Blue(i * 2 + 0));
			subset[0].a = (mode.AB > 0) ? Get(mode.Alpha(i * 2 + 0)) : 255;

			subset[1].rgb.r = Get(mode.Red(i * 2 + 1));
			subset[1].rgb.g = Get(mode.Green(i * 2 + 1));
			subset[1].rgb.b = Get(mode.Blue(i * 2 + 1));
			subset[1].a = (mode.AB > 0) ? Get(mode.Alpha(i * 2 + 1)) : 255;
		}

		if(mode.SPB > 0)
		{
			auto pbit0 = Get(mode.SharedPBit0());
			auto pbit1 = Get(mode.SharedPBit1());
			subsets[0][0].rgb = (subsets[0][0].rgb << 1) | pbit0;
			subsets[0][1].rgb = (subsets[0][1].rgb << 1) | pbit0;
			subsets[1][0].rgb = (subsets[1][0].rgb << 1) | pbit1;
			subsets[1][1].rgb = (subsets[1][1].rgb << 1) | pbit1;
		}

		if(mode.EPB > 0)
		{
			for(int i = 0; i < mode.NS; i++)
			{
				auto &subset = subsets[i];
				auto pbit0 = Get(mode.EndpointPBit(i * 2 + 0));
				auto pbit1 = Get(mode.EndpointPBit(i * 2 + 1));
				subset[0].rgb = (subset[0].rgb << 1) | pbit0;
				subset[1].rgb = (subset[1].rgb << 1) | pbit1;
				if(mode.AB > 0)
				{
					subset[0].a = (subset[0].a << 1) | pbit0;
					subset[1].a = (subset[1].a << 1) | pbit1;
				}
			}
		}

		auto const colorBits = mode.CB + mode.SPB + mode.EPB;
		auto const alphaBits = mode.AB + mode.SPB + mode.EPB;

		for(int i = 0; i < mode.NS; i++)
		{
			auto &subset = subsets[i];
			subset[0].rgb = subset[0].rgb << (8 - colorBits);
			subset[1].rgb = subset[1].rgb << (8 - colorBits);
			subset[0].rgb = subset[0].rgb | (subset[0].rgb >> colorBits);
			subset[1].rgb = subset[1].rgb | (subset[1].rgb >> colorBits);

			if(mode.AB > 0)
			{
				subset[0].a = subset[0].a << (8 - alphaBits);
				subset[1].a = subset[1].a << (8 - alphaBits);
				subset[0].a = subset[0].a | (subset[0].a >> alphaBits);
				subset[1].a = subset[1].a | (subset[1].a >> alphaBits);
			}
		}

		int colorIndexBitOffset = 0;
		int alphaIndexBitOffset = 0;
		for(int y = 0; y < 4; y++)
		{
			for(int x = 0; x < 4; x++)
			{
				auto texelIdx = y * 4 + x;
				auto partitionIdx = Get(mode.Partition());
				ASSERT(partitionIdx < MaxPartitions);
				auto subsetIdx = subsetIndex(mode, partitionIdx, texelIdx);
				ASSERT(subsetIdx < MaxSubsets);
				auto const &subset = subsets[subsetIdx];

				auto anchorIdx = anchorIndex(mode, partitionIdx, subsetIdx);
				auto isAnchor = anchorIdx == texelIdx;
				auto colorIdx = colorIndex(mode, isAnchor, colorIndexBitOffset);
				auto alphaIdx = alphaIndex(mode, isAnchor, alphaIndexBitOffset);

				if(y + dstY >= dstHeight || x + dstX >= dstWidth)
				{
					// Don't be tempted to skip early at the loops:
					// The calls to colorIndex() and alphaIndex() adjust bit
					// offsets that need to be carefully tracked.
					continue;
				}

				Color output;
				output.rgb.r = interpolate(subset[0].rgb.r, subset[1].rgb.r, colorIdx);
				output.rgb.g = interpolate(subset[0].rgb.g, subset[1].rgb.g, colorIdx);
				output.rgb.b = interpolate(subset[0].rgb.b, subset[1].rgb.b, colorIdx);
				output.a = interpolate(subset[0].a, subset[1].a, alphaIdx);

				switch(Get(mode.Rotation()))
				{
					default:
						break;
					case 1:
						std::swap(output.a, output.rgb.r);
						break;
					case 2:
						std::swap(output.a, output.rgb.g);
						break;
					case 3:
						std::swap(output.a, output.rgb.b);
						break;
				}

				auto out = reinterpret_cast<Color *>(dst + sizeof(Color) * x + dstPitch * y);
				*out = output;
			}
		}
	}

	int subsetIndex(const Mode &mode, int partitionIdx, int texelIndex) const
	{
		switch(mode.NS)
		{
			default:
				return 0;
			case 2:
				return PartitionTable2[partitionIdx][texelIndex];
			case 3:
				return PartitionTable3[partitionIdx][texelIndex];
		}
	}

	int anchorIndex(const Mode &mode, int partitionIdx, int subsetIdx) const
	{
		// ARB_texture_compression_bptc states:
		// "In partition zero, the anchor index is always index zero.
		// In other partitions, the anchor index is specified by tables
		// Table.A2 and Table.A3.""
		// Note: This is really confusing - I believe they meant subset instead
		// of partition here.
		switch(subsetIdx)
		{
			default:
				return 0;
			case 1:
				return mode.NS == 2 ? AnchorTable2[partitionIdx] : AnchorTable3a[partitionIdx];
			case 2:
				return AnchorTable3b[partitionIdx];
		}
	}

	IndexInfo colorIndex(const Mode &mode, bool isAnchor,
	                     int &indexBitOffset) const
	{
		// ARB_texture_compression_bptc states:
		// "The index value for interpolating color comes from the secondary
		// index for the texel if the format has an index selection bit and its
		// value is one and from the primary index otherwise.""
		auto idx = Get(mode.IndexSelection());
		ASSERT(idx <= 1);
		bool secondary = idx == 1;
		auto numBits = secondary ? mode.IB2 : mode.IB;
		auto numReadBits = numBits - (isAnchor ? 1 : 0);
		auto index =
		    Get(secondary ? mode.SecondaryIndex(indexBitOffset, numReadBits)
		                  : mode.PrimaryIndex(indexBitOffset, numReadBits));
		indexBitOffset += numReadBits;
		return { index, numBits };
	}

	IndexInfo alphaIndex(const Mode &mode, bool isAnchor,
	                     int &indexBitOffset) const
	{
		// ARB_texture_compression_bptc states:
		// "The alpha index comes from the secondary index if the block has a
		// secondary index and the block either doesn't have an index selection
		// bit or that bit is zero and the primary index otherwise."
		auto idx = Get(mode.IndexSelection());
		ASSERT(idx <= 1);
		bool secondary = (mode.IB2 != 0) && (idx == 0);
		auto numBits = secondary ? mode.IB2 : mode.IB;
		auto numReadBits = numBits - (isAnchor ? 1 : 0);
		auto index =
		    Get(secondary ? mode.SecondaryIndex(indexBitOffset, numReadBits)
		                  : mode.PrimaryIndex(indexBitOffset, numReadBits));
		indexBitOffset += numReadBits;
		return { index, numBits };
	}

	// Assumes little-endian
	uint64_t low;
	uint64_t high;
};

}  // namespace BC7
}  // anonymous namespace

// Decodes 1 to 4 channel images to 8 bit output
bool BC_Decoder::Decode(const uint8_t *src, uint8_t *dst, int w, int h, int dstPitch, int dstBpp, int n, bool isNoAlphaU)
{
	static_assert(sizeof(BC_color) == 8, "BC_color must be 8 bytes");
	static_assert(sizeof(BC_channel) == 8, "BC_channel must be 8 bytes");
	static_assert(sizeof(BC_alpha) == 8, "BC_alpha must be 8 bytes");

	const int dx = BlockWidth * dstBpp;
	const int dy = BlockHeight * dstPitch;
	const bool isAlpha = (n == 1) && !isNoAlphaU;
	const bool isSigned = ((n == 4) || (n == 5) || (n == 6)) && !isNoAlphaU;

	switch(n)
	{
		case 1:  // BC1
		{
			const BC_color *color = reinterpret_cast<const BC_color *>(src);
			for(int y = 0; y < h; y += BlockHeight, dst += dy)
			{
				uint8_t *dstRow = dst;
				for(int x = 0; x < w; x += BlockWidth, ++color, dstRow += dx)
				{
					color->decode(dstRow, x, y, w, h, dstPitch, dstBpp, isAlpha, false);
				}
			}
		}
		break;
		case 2:  // BC2
		{
			const BC_alpha *alpha = reinterpret_cast<const BC_alpha *>(src);
			const BC_color *color = reinterpret_cast<const BC_color *>(src + 8);
			for(int y = 0; y < h; y += BlockHeight, dst += dy)
			{
				uint8_t *dstRow = dst;
				for(int x = 0; x < w; x += BlockWidth, alpha += 2, color += 2, dstRow += dx)
				{
					color->decode(dstRow, x, y, w, h, dstPitch, dstBpp, isAlpha, true);
					alpha->decode(dstRow, x, y, w, h, dstPitch, dstBpp);
				}
			}
		}
		break;
		case 3:  // BC3
		{
			const BC_channel *alpha = reinterpret_cast<const BC_channel *>(src);
			const BC_color *color = reinterpret_cast<const BC_color *>(src + 8);
			for(int y = 0; y < h; y += BlockHeight, dst += dy)
			{
				uint8_t *dstRow = dst;
				for(int x = 0; x < w; x += BlockWidth, alpha += 2, color += 2, dstRow += dx)
				{
					color->decode(dstRow, x, y, w, h, dstPitch, dstBpp, isAlpha, true);
					alpha->decode(dstRow, x, y, w, h, dstPitch, dstBpp, 3, isSigned);
				}
			}
		}
		break;
		case 4:  // BC4
		{
			const BC_channel *red = reinterpret_cast<const BC_channel *>(src);
			for(int y = 0; y < h; y += BlockHeight, dst += dy)
			{
				uint8_t *dstRow = dst;
				for(int x = 0; x < w; x += BlockWidth, ++red, dstRow += dx)
				{
					red->decode(dstRow, x, y, w, h, dstPitch, dstBpp, 0, isSigned);
				}
			}
		}
		break;
		case 5:  // BC5
		{
			const BC_channel *red = reinterpret_cast<const BC_channel *>(src);
			const BC_channel *green = reinterpret_cast<const BC_channel *>(src + 8);
			for(int y = 0; y < h; y += BlockHeight, dst += dy)
			{
				uint8_t *dstRow = dst;
				for(int x = 0; x < w; x += BlockWidth, red += 2, green += 2, dstRow += dx)
				{
					red->decode(dstRow, x, y, w, h, dstPitch, dstBpp, 0, isSigned);
					green->decode(dstRow, x, y, w, h, dstPitch, dstBpp, 1, isSigned);
				}
			}
		}
		break;
        case 6:  // BC6h
        {
            const BC6h::Block *block = reinterpret_cast<const BC6h::Block *>(src);
            BC6h::Block tmp = *block;
            for (int y = 0; y < h; y += BlockHeight, dst += dy)
            {
                uint8_t *dstRow = dst;
                for (int x = 0; x < w; x += BlockWidth, ++block, dstRow += dx)
                {
                    tmp.decode(dstRow, x, y, w, h, dstPitch, isSigned);
                }
            }
        }
        break;
		case 7:  // BC7
		{
			const BC7::Block *block = reinterpret_cast<const BC7::Block *>(src);
			for(int y = 0; y < h; y += BlockHeight, dst += dy)
			{
				uint8_t *dstRow = dst;
				for(int x = 0; x < w; x += BlockWidth, ++block, dstRow += dx)
				{
					block->decode(dstRow, x, y, w, h, dstPitch);
				}
			}
		}
		break;
		default:
			return false;
	}

	return true;
}
