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

#include "ASTC_Decoder.hpp"
#include "astc_codec_internals.h"

void ASTC_Decoder::DecodeBlock(const unsigned char *source, uint16_t*** dest,
                               int destWidth, int destHeight, int destDepth,
                               int xBlockSize, int yBlockSize, int zBlockSize,
                               int x, int y, int z)
{
	build_quantization_mode_table();

	imageblock pb;
	symbolic_compressed_block scb;
	physical_compressed_block pcb = *(physical_compressed_block *)source;
	physical_to_symbolic(xBlockSize, yBlockSize, zBlockSize, pcb, &scb);
	decompress_symbolic_block(DECODE_HDR, xBlockSize, yBlockSize, zBlockSize, x * xBlockSize, y * yBlockSize, z * zBlockSize, &scb, &pb);

	astc_codec_image img;
	img.imagedata8 = nullptr;
	img.imagedata16 = dest;
	img.xsize = destWidth;
	img.ysize = destHeight;
	img.zsize = destDepth;
	img.padding = 0;
	swizzlepattern swz;
	swz.r = 0;
	swz.g = 1;
	swz.b = 2;
	swz.a = 3;
	write_imageblock(&img, &pb, xBlockSize, yBlockSize, zBlockSize, 0, 0, 0, swz);
}
