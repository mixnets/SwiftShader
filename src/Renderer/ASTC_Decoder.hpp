// SwiftShader Software Renderer
//
// Copyright(c) 2015 Google Inc.
//
// All rights reserved. No part of this software may be copied, distributed, transmitted,
// transcribed, stored in a retrieval system, translated into any human or computer
// language by any means, or disclosed to third parties without the explicit written
// agreement of Google Inc. Without such an agreement, no rights or licenses, express
// or implied, including but not limited to any patent rights, are granted to you.
//

struct ASTC_Decoder
{
	static void DecodeBlock(const unsigned char *source, unsigned char* dest,
							int destWidth, int destHeight, int destDepth,
							int destPitchB, int destSliceB,
							int xBlockSize, int yBlockSize, int zBlockSize,
							int x, int y, int z);
};
