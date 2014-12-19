// SwiftShader Software Renderer
//
// Copyright(c) 2005-2013 TransGaming Inc.
//
// All rights reserved. No part of this software may be copied, distributed, transmitted,
// transcribed, stored in a retrieval system, translated into any human or computer
// language by any means, or disclosed to third parties without the explicit written
// agreement of TransGaming Inc. Without such an agreement, no rights or licenses, express
// or implied, including but not limited to any patent rights, are granted to you.
//

// main.cpp: DLL entry point and management of thread-local data.

#include "main.h"

#include "Framebuffer.h"
#include "libEGL/Surface.h"
#include "Common/Thread.hpp"
#include "Common/SharedLibrary.hpp"
#include "common/debug.h"

#if !defined(_MSC_VER)
#define CONSTRUCTOR __attribute__((constructor))
#define DESTRUCTOR __attribute__((destructor))
#else
#define CONSTRUCTOR
#define DESTRUCTOR
#endif

static void glAttachThread()
{
    TRACE("()");
}

static void glDetachThread()
{
    TRACE("()");
}

CONSTRUCTOR static void glAttachProcess()
{
    TRACE("()");

    glAttachThread();
}

DESTRUCTOR static void glDetachProcess()
{
    TRACE("()");

	glDetachThread();
}

#if defined(_WIN32)
extern "C" BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
    switch(reason)
    {
    case DLL_PROCESS_ATTACH:
        glAttachProcess();
        break;
    case DLL_THREAD_ATTACH:
        glAttachThread();
        break;
    case DLL_THREAD_DETACH:
        glDetachThread();
        break;
    case DLL_PROCESS_DETACH:
        glDetachProcess();
        break;
    default:
        break;
    }

    return TRUE;
}
#endif

namespace es1
{
es1::Context *getContext()
{
	egl::Context *context = libEGL->getCurrentContext();

	if(context && context->getClientVersion() == 1)
	{
		return static_cast<es1::Context*>(context);
	}

	return 0;
}

egl::Display *getDisplay()
{
    return libEGL->getCurrentDisplay();
}

Device *getDevice()
{
    Context *context = getContext();

    return context ? context->getDevice() : 0;
}
}

// Records an error code
void error(GLenum errorCode)
{
    es1::Context *context = es1::getContext();

    if(context)
    {
        switch(errorCode)
        {
        case GL_INVALID_ENUM:
            context->recordInvalidEnum();
            TRACE("\t! Error generated: invalid enum\n");
            break;
        case GL_INVALID_VALUE:
            context->recordInvalidValue();
            TRACE("\t! Error generated: invalid value\n");
            break;
        case GL_INVALID_OPERATION:
            context->recordInvalidOperation();
            TRACE("\t! Error generated: invalid operation\n");
            break;
        case GL_OUT_OF_MEMORY:
            context->recordOutOfMemory();
            TRACE("\t! Error generated: out of memory\n");
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION_OES:
            context->recordInvalidFramebufferOperation();
            TRACE("\t! Error generated: invalid framebuffer operation\n");
            break;
        default: UNREACHABLE();
        }
    }
}

LibEGLdependencies::LibEGLdependencies()
{
	#if defined(_WIN32)
	const char *libEGL_lib[] = {"libEGL.dll", "libEGL_translator.dll", 0};
	#elif defined(__LP64__)
	const char *libEGL_lib[] = {"libEGL.so.1", "libEGL.so", "lib64EGL_translator.so", 0};
	#else
	const char *libEGL_lib[] = {"libEGL.so.1", "libEGL.so", "libEGL_translator.so", 0};
	#endif

	libEGL = loadLibrary(libEGL_lib);

	eglGetError = (EGLint(EGLAPIENTRY*)(void))getProcAddress(libEGL, "eglGetError");
	eglGetDisplay = (EGLDisplay(EGLAPIENTRY*)(EGLNativeDisplayType display_id))getProcAddress(libEGL, "eglGetDisplay");
	eglInitialize = (EGLBoolean(EGLAPIENTRY*)(EGLDisplay dpy, EGLint *major, EGLint *minor))getProcAddress(libEGL, "eglInitialize");
	eglTerminate = (EGLBoolean(EGLAPIENTRY*)(EGLDisplay dpy))getProcAddress(libEGL, "eglTerminate");
	eglQueryString = (const char *(EGLAPIENTRY*)(EGLDisplay dpy, EGLint name))getProcAddress(libEGL, "eglQueryString");
	eglGetConfigs = (EGLBoolean(EGLAPIENTRY*)(EGLDisplay dpy, EGLConfig *configs, EGLint config_size, EGLint *num_config))getProcAddress(libEGL, "eglGetConfigs");
	eglChooseConfig = (EGLBoolean(EGLAPIENTRY*)(EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config))getProcAddress(libEGL, "eglChooseConfig");
	eglGetConfigAttrib = (EGLBoolean(EGLAPIENTRY*)(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint *value))getProcAddress(libEGL, "eglGetConfigAttrib");
	eglCreateWindowSurface = (EGLSurface(EGLAPIENTRY*)(EGLDisplay dpy, EGLConfig config, EGLNativeWindowType window, const EGLint *attrib_list))getProcAddress(libEGL, "eglCreateWindowSurface");
	eglCreatePbufferSurface = (EGLSurface(EGLAPIENTRY*)(EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list))getProcAddress(libEGL, "eglCreatePbufferSurface");
	eglCreatePixmapSurface = (EGLSurface(EGLAPIENTRY*)(EGLDisplay dpy, EGLConfig config, EGLNativePixmapType pixmap, const EGLint *attrib_list))getProcAddress(libEGL, "eglCreatePixmapSurface");
	eglDestroySurface = (EGLBoolean(EGLAPIENTRY*)(EGLDisplay dpy, EGLSurface surface))getProcAddress(libEGL, "eglDestroySurface");
	eglQuerySurface = (EGLBoolean(EGLAPIENTRY*)(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint *value))getProcAddress(libEGL, "eglQuerySurface");
	eglBindAPI = (EGLBoolean(EGLAPIENTRY*)(EGLenum api))getProcAddress(libEGL, "eglBindAPI");
	eglQueryAPI = (EGLenum(EGLAPIENTRY*)(void))getProcAddress(libEGL, "eglQueryAPI");
	eglWaitClient = (EGLBoolean(EGLAPIENTRY*)(void))getProcAddress(libEGL, "eglWaitClient");
	eglReleaseThread = (EGLBoolean(EGLAPIENTRY*)(void))getProcAddress(libEGL, "eglReleaseThread");
	eglCreatePbufferFromClientBuffer = (EGLSurface(EGLAPIENTRY*)(EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer, EGLConfig config, const EGLint *attrib_list))getProcAddress(libEGL, "eglCreatePbufferFromClientBuffer");
	eglSurfaceAttrib = (EGLBoolean(EGLAPIENTRY*)(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint value))getProcAddress(libEGL, "eglSurfaceAttrib");
	eglBindTexImage = (EGLBoolean(EGLAPIENTRY*)(EGLDisplay dpy, EGLSurface surface, EGLint buffer))getProcAddress(libEGL, "eglBindTexImage");
	eglReleaseTexImage = (EGLBoolean(EGLAPIENTRY*)(EGLDisplay dpy, EGLSurface surface, EGLint buffer))getProcAddress(libEGL, "eglReleaseTexImage");
	eglSwapInterval = (EGLBoolean(EGLAPIENTRY*)(EGLDisplay dpy, EGLint interval))getProcAddress(libEGL, "eglSwapInterval");
	eglCreateContext = (EGLContext(EGLAPIENTRY*)(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint *attrib_list))getProcAddress(libEGL, "eglCreateContext");
	eglDestroyContext = (EGLBoolean(EGLAPIENTRY*)(EGLDisplay dpy, EGLContext ctx))getProcAddress(libEGL, "eglDestroyContext");
	eglMakeCurrent = (EGLBoolean(EGLAPIENTRY*)(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx))getProcAddress(libEGL, "eglMakeCurrent");
	eglGetCurrentContext = (EGLContext(EGLAPIENTRY*)(void))getProcAddress(libEGL, "eglGetCurrentContext");
	eglGetCurrentSurface = (EGLSurface(EGLAPIENTRY*)(EGLint readdraw))getProcAddress(libEGL, "eglGetCurrentSurface");
	eglGetCurrentDisplay = (EGLDisplay(EGLAPIENTRY*)(void))getProcAddress(libEGL, "eglGetCurrentDisplay");
	eglQueryContext = (EGLBoolean(EGLAPIENTRY*)(EGLDisplay dpy, EGLContext ctx, EGLint attribute, EGLint *value))getProcAddress(libEGL, "eglQueryContext");
	eglWaitGL = (EGLBoolean(EGLAPIENTRY*)(void))getProcAddress(libEGL, "eglWaitGL");
	eglWaitNative = (EGLBoolean(EGLAPIENTRY*)(EGLint engine))getProcAddress(libEGL, "eglWaitNative");
	eglSwapBuffers = (EGLBoolean(EGLAPIENTRY*)(EGLDisplay dpy, EGLSurface surface))getProcAddress(libEGL, "eglSwapBuffers");
	eglCopyBuffers = (EGLBoolean(EGLAPIENTRY*)(EGLDisplay dpy, EGLSurface surface, EGLNativePixmapType target))getProcAddress(libEGL, "eglCopyBuffers");
	eglCreateImageKHR = (EGLImageKHR(EGLAPIENTRY*)(EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint *attrib_list))getProcAddress(libEGL, "eglCreateImageKHR");
	eglDestroyImageKHR = (EGLBoolean(EGLAPIENTRY*)(EGLDisplay dpy, EGLImageKHR image))getProcAddress(libEGL, "eglDestroyImageKHR");
	eglGetProcAddress = (__eglMustCastToProperFunctionPointerType(EGLAPIENTRY*)(const char*))getProcAddress(libEGL, "eglGetProcAddress");

	getCurrentContext = (egl::Context *(*)())getProcAddress(libEGL, "clientGetCurrentContext");
	getCurrentDisplay = (egl::Display *(*)())getProcAddress(libEGL, "clientGetCurrentDisplay");
}

LibEGLdependencies::~LibEGLdependencies()
{
	freeLibrary(libEGL);
}

LibEGLdependencies *LibEGLdependencies::getSingleton()
{
	static LibEGLdependencies singleton;

	return &singleton;
}

LibEGL libEGL;

namespace es1
{
LibGLES_CM *LibGLES_CM::getSingleton()
{
	static LibGLES_CM singleton;

	return &singleton;
}

egl::Context *LibGLES_CM::createContext(const egl::Config *config, const egl::Context *shareContext)
{
	return glCreateContext(config, shareContext);
}

__eglMustCastToProperFunctionPointerType LibGLES_CM::getProcAddress(const char *proc)
{
	return es1GetProcAddress(proc);
}

egl::Image *LibGLES_CM::createBackBuffer(int width, int height, const egl::Config*);
{
	return ::createBackBuffer(width, height, config);
}

egl::Image *LibGLES_CM::createDepthStencil(unsigned int, unsigned int, sw::Format, int, bool);
{
	return ::createDepthStencil(width, height, format, samples, target);
}

sw::FrameBuffer *LibGLES_CM::createFrameBuffer(EGLNativeDisplayType display, EGLNativeWindowType window, int width, int height)
{
	return createFrameBuffer(display, window, width, height);
}
}
