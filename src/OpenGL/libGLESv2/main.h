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
#include "libEGL/Display.h"
#include "libEGL/main.h"

#define GL_APICALL
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

namespace es2
{
	Context *getContext();
	egl::Display *getDisplay();
	Device *getDevice();
}

namespace egl
{
	GLint getClientVersion();
}

void error(GLenum errorCode);

template<class T>
const T &error(GLenum errorCode, const T &returnValue)
{
    error(errorCode);

    return returnValue;
}

class LibEGL
{
public:
	LibEGL();
	~LibEGL();

	egl::LibEGL *operator->();

private:
	egl::LibEGL *libEGL;
	void *libEGLlibrary;
};

extern LibEGL libEGL;

class LibGLES_CMdependencies
{
public:
	static LibGLES_CMdependencies *getSingleton();

	PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES;

private:
	LibGLES_CMdependencies();
	~LibGLES_CMdependencies();

	void *libGLES_CM;
};

class LibGLES_CM
{
public:
	LibGLES_CMdependencies *operator->()
	{
		return LibGLES_CMdependencies::getSingleton();
	}
};

extern LibGLES_CM libGLES_CM;

#endif   // LIBGLESV2_MAIN_H_
