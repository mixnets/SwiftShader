/*----------------------------------------------------------------------------*/  
/**
 *	This confidential and proprietary software may be used only as
 *	authorised by a licensing agreement from ARM Limited
 *	(C) COPYRIGHT 2011-2012 ARM Limited
 *	ALL RIGHTS RESERVED
 *
 *	The entire notice above must be reproduced on all authorised
 *	copies and copies may only be made to the extent permitted
 *	by a licensing agreement from ARM Limited.
 *
 *	@brief	Decompress a block of colors, expressed as a symbolic block,
 *			for ASTC.
 */ 
/*----------------------------------------------------------------------------*/ 

#include "ASTC_Decoder.hpp"
#include "astc_codec_internals.h"

void write_imageblock32(unsigned char* img, const imageblock * pb,	// picture-block to initialize with image data. We assume that orig_data is valid.
	// output dimensions
	int xsize, int ysize, int zsize, int destPitchB, int destSliceB,
	// block dimensions
	int xdim, int ydim, int zdim,
	// position to write the block to
	int xpos, int ypos, int zpos)
{
	const float *fptr = pb->orig_data;
	const uint8_t *nptr = pb->nan_texel;

	union pixelData
	{
		unsigned int i;
		float f;
	};

	for(int z = 0; z < zdim; z++)
	{
		for(int y = 0; y < ydim; y++)
		{
			for(int x = 0; x < xdim; x++)
			{
				int xi = xpos + x;
				int yi = ypos + y;
				int zi = zpos + z;

				if(xi >= 0 && yi >= 0 && zi >= 0 && xi < xsize && yi < ysize && zi < zsize)
				{
					pixelData* pix = (pixelData*)(&img[zi * destSliceB + yi * destPitchB + xi * 16]);

					if(*nptr)
					{
						pix[0].i = pix[1].i = pix[2].i = pix[3].i = 0xFFFFFFFF;
					}
					else
					{
						pix[0].f = fptr[0];
						pix[1].f = fptr[1];
						pix[2].f = fptr[2];
						pix[3].f = fptr[3];
					}
				}
				fptr += 4;
				nptr++;
			}
		}
	}
}

void ASTC_Decoder::DecodeBlock(const unsigned char *source, unsigned char* dest,
                               int destWidth, int destHeight, int destDepth,
                               int destPitchB, int destSliceB,
                               int xBlockSize, int yBlockSize, int zBlockSize,
                               int x, int y, int z)
{
	build_quantization_mode_table();

	imageblock pb;
	symbolic_compressed_block scb;
	physical_compressed_block pcb = *(physical_compressed_block *)source;
	physical_to_symbolic(xBlockSize, yBlockSize, zBlockSize, pcb, &scb);
	decompress_symbolic_block(DECODE_HDR, xBlockSize, yBlockSize, zBlockSize, x * xBlockSize, y * yBlockSize, z * zBlockSize, &scb, &pb);
	write_imageblock32(dest, &pb, destWidth, destHeight, destDepth, destPitchB, destSliceB, xBlockSize, yBlockSize, zBlockSize, x * xBlockSize, y * yBlockSize, z * zBlockSize);
}