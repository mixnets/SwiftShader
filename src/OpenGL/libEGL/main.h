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

#include "libGLES_CM/main.h"

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

	void error(EGLint errorCode);
	
	template<class T>
	const T &error(EGLint errorCode, const T &returnValue)
	{
		egl::error(errorCode);

		return returnValue;
	}

	template<class T>
	const T &success(const T &returnValue)
	{
		egl::setCurrentError(EGL_SUCCESS);

		return returnValue;
	}

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

class LibEGLexports
{
public:
	static LibEGLexports *getSingleton();

	EGLint (EGLAPIENTRY *eglGetError)(void);
	EGLDisplay (EGLAPIENTRY *eglGetDisplay)(EGLNativeDisplayType display_id);
	EGLBoolean (EGLAPIENTRY *eglInitialize)(EGLDisplay dpy, EGLint *major, EGLint *minor);
	EGLBoolean (EGLAPIENTRY *eglTerminate)(EGLDisplay dpy);
	const char *(EGLAPIENTRY *eglQueryString)(EGLDisplay dpy, EGLint name);
	EGLBoolean (EGLAPIENTRY *eglGetConfigs)(EGLDisplay dpy, EGLConfig *configs, EGLint config_size, EGLint *num_config);
	EGLBoolean (EGLAPIENTRY *eglChooseConfig)(EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config);
	EGLBoolean (EGLAPIENTRY *eglGetConfigAttrib)(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint *value);
	EGLSurface (EGLAPIENTRY *eglCreateWindowSurface)(EGLDisplay dpy, EGLConfig config, EGLNativeWindowType window, const EGLint *attrib_list);
	EGLSurface (EGLAPIENTRY *eglCreatePbufferSurface)(EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list);
	EGLSurface (EGLAPIENTRY *eglCreatePixmapSurface)(EGLDisplay dpy, EGLConfig config, EGLNativePixmapType pixmap, const EGLint *attrib_list);
	EGLBoolean (EGLAPIENTRY *eglDestroySurface)(EGLDisplay dpy, EGLSurface surface);
	EGLBoolean (EGLAPIENTRY *eglQuerySurface)(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint *value);
	EGLBoolean (EGLAPIENTRY *eglBindAPI)(EGLenum api);
	EGLenum (EGLAPIENTRY *eglQueryAPI)(void);
	EGLBoolean (EGLAPIENTRY *eglWaitClient)(void);
	EGLBoolean (EGLAPIENTRY *eglReleaseThread)(void);
	EGLSurface (EGLAPIENTRY *eglCreatePbufferFromClientBuffer)(EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer, EGLConfig config, const EGLint *attrib_list);
	EGLBoolean (EGLAPIENTRY *eglSurfaceAttrib)(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint value);
	EGLBoolean (EGLAPIENTRY *eglBindTexImage)(EGLDisplay dpy, EGLSurface surface, EGLint buffer);
	EGLBoolean (EGLAPIENTRY *eglReleaseTexImage)(EGLDisplay dpy, EGLSurface surface, EGLint buffer);
	EGLBoolean (EGLAPIENTRY *eglSwapInterval)(EGLDisplay dpy, EGLint interval);
	EGLContext (EGLAPIENTRY *eglCreateContext)(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint *attrib_list);
	EGLBoolean (EGLAPIENTRY *eglDestroyContext)(EGLDisplay dpy, EGLContext ctx);
	EGLBoolean (EGLAPIENTRY *eglMakeCurrent)(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx);
	EGLContext (EGLAPIENTRY *eglGetCurrentContext)(void);
	EGLSurface (EGLAPIENTRY *eglGetCurrentSurface)(EGLint readdraw);
	EGLDisplay (EGLAPIENTRY *eglGetCurrentDisplay)(void);
	EGLBoolean (EGLAPIENTRY *eglQueryContext)(EGLDisplay dpy, EGLContext ctx, EGLint attribute, EGLint *value);
	EGLBoolean (EGLAPIENTRY *eglWaitGL)(void);
	EGLBoolean (EGLAPIENTRY *eglWaitNative)(EGLint engine);
	EGLBoolean (EGLAPIENTRY *eglSwapBuffers)(EGLDisplay dpy, EGLSurface surface);
	EGLBoolean (EGLAPIENTRY *eglCopyBuffers)(EGLDisplay dpy, EGLSurface surface, EGLNativePixmapType target);
	EGLImageKHR (EGLAPIENTRY *eglCreateImageKHR)(EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint *attrib_list);
	EGLBoolean (EGLAPIENTRY *eglDestroyImageKHR)(EGLDisplay dpy, EGLImageKHR image);
	__eglMustCastToProperFunctionPointerType (EGLAPIENTRY *eglGetProcAddress)(const char*);

	// Functions that don't change the error code, for use by client APIs
	egl::Context *(*clientGetCurrentContext)();
	egl::Display *(*clientGetCurrentDisplay)();

private:
	LibEGLexports();
};

class LibGLES_CMexports;

class LibGLES_CM
{
public:
	~LibGLES_CM();

	LibGLES_CMexports *operator->();

private:
	static void *libGLES_CM;
};

extern LibGLES_CM libGLES_CM;

#endif  // LIBEGL_MAIN_H_
