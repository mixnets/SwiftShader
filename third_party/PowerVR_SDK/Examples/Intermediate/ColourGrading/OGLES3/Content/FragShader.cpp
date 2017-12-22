// This file was created by Filewrap 1.1
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: FragShader.fsh ********

// File data
static const char _FragShader_fsh[] = 
	"#version 300 es\n"
	"\n"
	"uniform  sampler2D     sTexture;\n"
	"uniform  mediump sampler3D\t\tsColourLUT;\n"
	"\n"
	"in mediump vec2 texCoords;\n"
	"layout(location = 0) out lowp vec4 oFragColour;\n"
	"\n"
	"void main()\n"
	"{\n"
	"const highp float array[16] = float[](\r\n"
	"\t0.820844,\r\n"
	"\t-0.419144,\r\n"
	"\t-0.977806,\r\n"
	"\t0.625848,\r\n"
	"\t-0.879658,\r\n"
	"\t0.886325,\r\n"
	"\t0.920155,\r\n"
	"\t-0.916329,\r\n"
	"\t0.844853,\r\n"
	"\t-0.818481,\r\n"
	"\t-0.430844,\r\n"
	"\t-0.102321,\r\n"
	"\t-0.549422,\r\n"
	"\t0.468418,\r\n"
	"\t0.091143,\r\n"
	"\t-0.391751);\n"
	"\n"
	"    highp vec3 vCol = texture(sTexture, texCoords).rgb;\n"
	"\tlowp vec3 vAlteredCol = texture(sColourLUT, vCol.rgb).rgb;\n"
	"    oFragColour = vec4(vAlteredCol.xy, array[int(vAlteredCol.z*15.0)], 1.0);\n"
	"}\n";

// Register FragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_FragShader_fsh("FragShader.fsh", _FragShader_fsh, 618);

// ******** End: FragShader.fsh ********

