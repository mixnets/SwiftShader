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

#ifndef LIBEGL_MAIN_H_
#define LIBEGL_MAIN_H_

#define EGLAPI
#include <EGL/egl.h>
#include <EGL/eglext.h>

namespace egl
{
	struct Current
	{
		EGLint error;
		EGLenum API;
		EGLDisplay display;
		EGLContext context;
		EGLSurface drawSurface;
		EGLSurface readSurface;
	};

	void setCurrentError(EGLint error);
	EGLint getCurrentError();

	void setCurrentAPI(EGLenum API);
	EGLenum getCurrentAPI();

	void setCurrentDisplay(EGLDisplay dpy);
	EGLDisplay getCurrentDisplay();

	void setCurrentContext(EGLContext ctx);
	EGLContext getCurrentContext();

	void setCurrentDrawSurface(EGLSurface surface);
	EGLSurface getCurrentDrawSurface();

	void setCurrentReadSurface(EGLSurface surface);
	EGLSurface getCurrentReadSurface();
}

void error(EGLint errorCode);

template<class T>
const T &error(EGLint errorCode, const T &returnValue)
{
    error(errorCode);

    return returnValue;
}

template<class T>
const T &success(const T &returnValue)
{
    egl::setCurrentError(EGL_SUCCESS);

    return returnValue;
}

namespace egl
{
	class Config;
	class Surface;
	class Display;
	class Context;
	class Image;
}

namespace sw
{
	class FrameBuffer;
	enum Format : unsigned char;
}

class LibGLES_CMdependencies
{
public:
	static LibGLES_CMdependencies *getSingleton();
	static bool isResident();
	static const char **libraryNames();

	egl::Context *(*createContext)(const egl::Config *config, const egl::Context *shareContext);
	__eglMustCastToProperFunctionPointerType(*getProcAddress)(const char *procname);

	egl::Image *(*createBackBuffer)(int width, int height, const egl::Config *config);
	egl::Image *(*createDepthStencil)(unsigned int width, unsigned int height, sw::Format format, int multiSampleDepth, bool discard);
	sw::FrameBuffer *(*createFrameBuffer)(EGLNativeDisplayType display, EGLNativeWindowType window, int width, int height);

private:
	LibGLES_CMdependencies();
	~LibGLES_CMdependencies();
};

class LibGLES_CM
{
public:
	LibGLES_CMdependencies *operator->()
	{
		return LibGLES_CMdependencies::getSingleton();
	}

	operator bool()
	{
		return LibGLES_CMdependencies::isResident();
	}
};

extern LibGLES_CM libGLES_CM;

class LibGLESv2dependencies
{
public:
	static LibGLESv2dependencies *getSingleton();
	static bool isResident();
	static const char **libraryNames();

	egl::Context *(*createContext)(const egl::Config *config, const egl::Context *shareContext);
	__eglMustCastToProperFunctionPointerType(*getProcAddress)(const char *procname);

	egl::Image *(*createBackBuffer)(int width, int height, const egl::Config *config);
	egl::Image *(*createDepthStencil)(unsigned int width, unsigned int height, sw::Format format, int multiSampleDepth, bool discard);
	sw::FrameBuffer *(*createFrameBuffer)(EGLNativeDisplayType display, EGLNativeWindowType window, int width, int height);

private:
	LibGLESv2dependencies();
	~LibGLESv2dependencies();
};

class LibGLESv2
{
public:
	LibGLESv2dependencies *operator->()
	{
		return LibGLESv2dependencies::getSingleton();
	}

	operator bool()
	{
		return LibGLESv2dependencies::isResident();
	}
};

extern LibGLESv2 libGLESv2;

#endif  // LIBEGL_MAIN_H_
