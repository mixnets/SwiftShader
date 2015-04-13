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

// main.h: Management of thread-local data.

#ifndef LIBGLESV2_MAIN_H_
#define LIBGLESV2_MAIN_H_

#include "Context.h"
#include "Device.hpp"
#include "common/debug.h"
#include "libEGL/libEGL.hpp"
#include "libEGL/Display.h"
#include "libGLES_CM/main.h"

#define GL_APICALL
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES3/gl3.h>

namespace es2
{
	Context *getContext();
	egl::Display *getDisplay();
	Device *getDevice();

	void error(GLenum errorCode);

	template<class T>
	const T &error(GLenum errorCode, const T &returnValue)
	{
		error(errorCode);

		return returnValue;
	}
}

namespace egl
{
	GLint getClientVersion();
}

namespace sw
{
class FrameBuffer;
}

class LibGLESv2exports
{
public:
	static LibGLESv2exports *getSingleton();

	es2::Context *(*es2CreateContext)(const egl::Config *config, const egl::Context *shareContext, int clientVersion);
	__eglMustCastToProperFunctionPointerType (*es2GetProcAddress)(const char *procname);
	egl::Image *(*createBackBuffer)(int width, int height, const egl::Config *config);
	egl::Image *(*createDepthStencil)(unsigned int width, unsigned int height, sw::Format format, int multiSampleDepth, bool discard);
	sw::FrameBuffer *(*createFrameBuffer)(EGLNativeDisplayType display, EGLNativeWindowType window, int width, int height);

private:
	LibGLESv2exports();
};

class LibGLESv2
{
public:
	~LibGLESv2();

	LibGLESv2exports *operator->();

private:
	static void *libGLESv2;
};

extern LibEGL libEGL;
extern LibGLES_CM libGLES_CM;

#endif   // LIBGLESV2_MAIN_H_
