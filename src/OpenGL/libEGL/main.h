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
#define EGL_EGLEXT_PROTOTYPES
#include <EGL/eglext.h>

namespace egl
{
	class Display;
	class Context;
	class Surface;

	struct Current
	{
		EGLint error;
		EGLenum API;
		Display *display;
		Context *context;
		Surface *drawSurface;
		Surface *readSurface;
	};

	void setCurrentError(EGLint error);
	EGLint getCurrentError();

	void setCurrentAPI(EGLenum API);
	EGLenum getCurrentAPI();

	void setCurrentDisplay(Display *dpy);
	Display *getCurrentDisplay();

	void setCurrentContext(Context *ctx);
	Context *getCurrentContext();

	void setCurrentDrawSurface(Surface *surface);
	Surface *getCurrentDrawSurface();

	void setCurrentReadSurface(Surface *surface);
	Surface *getCurrentReadSurface();
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

// libGLES_CM dependencies
namespace es1
{
	extern egl::Context *(*createContext)(const egl::Config *config, const egl::Context *shareContext);
	extern __eglMustCastToProperFunctionPointerType (*getProcAddress)(const char *procname);
}

// libGLESv2 dependencies
namespace es2
{
	extern egl::Context *(*createContext)(const egl::Config *config, const egl::Context *shareContext, EGLint clientVersion);
	extern __eglMustCastToProperFunctionPointerType (*getProcAddress)(const char *procname);
}

namespace es
{
	extern egl::Image *(*createBackBuffer)(int width, int height, const egl::Config *config);
	extern egl::Image *(*createDepthStencil)(unsigned int width, unsigned int height, sw::Format format, int multiSampleDepth, bool discard);
	extern sw::FrameBuffer *(*createFrameBuffer)(EGLNativeDisplayType display, EGLNativeWindowType window, int width, int height);
}

extern void *libGLES_CM;   // Handle to the libGLES_CM module
extern void *libGLESv2;    // Handle to the libGLESv2 module

struct LibEGLexports
{
	static EGLint (EGLAPIENTRY *eglGetError)(void);
	static EGLDisplay (EGLAPIENTRY *eglGetDisplay)(EGLNativeDisplayType display_id);
	static EGLBoolean (EGLAPIENTRY *eglInitialize)(EGLDisplay dpy, EGLint *major, EGLint *minor);
	static EGLBoolean (EGLAPIENTRY *eglTerminate)(EGLDisplay dpy);
	static const char *(EGLAPIENTRY *eglQueryString)(EGLDisplay dpy, EGLint name);
	static EGLBoolean (EGLAPIENTRY *eglGetConfigs)(EGLDisplay dpy, EGLConfig *configs, EGLint config_size, EGLint *num_config);
	static EGLBoolean (EGLAPIENTRY *eglChooseConfig)(EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config);
	static EGLBoolean (EGLAPIENTRY *eglGetConfigAttrib)(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint *value);
	static EGLSurface (EGLAPIENTRY *eglCreateWindowSurface)(EGLDisplay dpy, EGLConfig config, EGLNativeWindowType window, const EGLint *attrib_list);
	static EGLSurface (EGLAPIENTRY *eglCreatePbufferSurface)(EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list);
	static EGLSurface (EGLAPIENTRY *eglCreatePixmapSurface)(EGLDisplay dpy, EGLConfig config, EGLNativePixmapType pixmap, const EGLint *attrib_list);
	static EGLBoolean (EGLAPIENTRY *eglDestroySurface)(EGLDisplay dpy, EGLSurface surface);
	static EGLBoolean (EGLAPIENTRY *eglQuerySurface)(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint *value);
	static EGLBoolean (EGLAPIENTRY *eglBindAPI)(EGLenum api);
	static EGLenum (EGLAPIENTRY *eglQueryAPI)(void);
	static EGLBoolean (EGLAPIENTRY *eglWaitClient)(void);
	static EGLBoolean (EGLAPIENTRY *eglReleaseThread)(void);
	static EGLSurface (EGLAPIENTRY *eglCreatePbufferFromClientBuffer)(EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer, EGLConfig config, const EGLint *attrib_list);
	static EGLBoolean (EGLAPIENTRY *eglSurfaceAttrib)(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint value);
	static EGLBoolean (EGLAPIENTRY *eglBindTexImage)(EGLDisplay dpy, EGLSurface surface, EGLint buffer);
	static EGLBoolean (EGLAPIENTRY *eglReleaseTexImage)(EGLDisplay dpy, EGLSurface surface, EGLint buffer);
	static EGLBoolean (EGLAPIENTRY *eglSwapInterval)(EGLDisplay dpy, EGLint interval);
	static EGLContext (EGLAPIENTRY *eglCreateContext)(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint *attrib_list);
	static EGLBoolean (EGLAPIENTRY *eglDestroyContext)(EGLDisplay dpy, EGLContext ctx);
	static EGLBoolean (EGLAPIENTRY *eglMakeCurrent)(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx);
	static EGLContext (EGLAPIENTRY *eglGetCurrentContext)(void);
	static EGLSurface (EGLAPIENTRY *eglGetCurrentSurface)(EGLint readdraw);
	static EGLDisplay (EGLAPIENTRY *eglGetCurrentDisplay)(void);
	static EGLBoolean (EGLAPIENTRY *eglQueryContext)(EGLDisplay dpy, EGLContext ctx, EGLint attribute, EGLint *value);
	static EGLBoolean (EGLAPIENTRY *eglWaitGL)(void);
	static EGLBoolean (EGLAPIENTRY *eglWaitNative)(EGLint engine);
	static EGLBoolean (EGLAPIENTRY *eglSwapBuffers)(EGLDisplay dpy, EGLSurface surface);
	static EGLBoolean (EGLAPIENTRY *eglCopyBuffers)(EGLDisplay dpy, EGLSurface surface, EGLNativePixmapType target);
	static EGLImageKHR (EGLAPIENTRY *eglCreateImageKHR)(EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint *attrib_list);
	static EGLBoolean (EGLAPIENTRY *eglDestroyImageKHR)(EGLDisplay dpy, EGLImageKHR image);
	static __eglMustCastToProperFunctionPointerType (EGLAPIENTRY *eglGetProcAddress)(const char*);

	static egl::Context *(*getCurrentContext)();
	static egl::Display *(*getCurrentDisplay)();
};

#endif  // LIBEGL_MAIN_H_
