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

#define GL_GLEXT_PROTOTYPES
#include <GLES/glext.h>

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
	egl::Context *context = libEGL->clientGetCurrentContext();

	if(context && context->getClientVersion() == 1)
	{
		return static_cast<es1::Context*>(context);
	}

	return 0;
}

egl::Display *getDisplay()
{
    return libEGL->clientGetCurrentDisplay();
}

Device *getDevice()
{
    Context *context = getContext();

    return context ? context->getDevice() : 0;
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
}

LibGLES_CMexports::LibGLES_CMexports()
{
	this->es1CreateContext = ::es1CreateContext;
	this->es1GetProcAddress = ::es1GetProcAddress;
	this->createBackBuffer = ::createBackBuffer;
	this->createDepthStencil = ::createDepthStencil;
	this->createFrameBuffer = ::createFrameBuffer;
	this->glEGLImageTargetTexture2DOES = ::glEGLImageTargetTexture2DOES;
}

LibGLES_CMexports *LibGLES_CMexports::getSingleton()
{
	static LibGLES_CMexports libGLES_CM;
	return &libGLES_CM;
}

extern "C" LibGLES_CMexports *libGLES_CMexports()
{
	return LibGLES_CMexports::getSingleton();
}

LibEGL libEGL;
void *LibEGL::libEGL = nullptr;

LibEGL::~LibEGL()
{
	freeLibrary(libEGL);
}

LibEGLexports *LibEGL::operator->()
{
	static LibEGLexports *(*libEGLexports)() = nullptr;

	if(!libEGL)
	{
		#if defined(_WIN32)
		const char *libEGL_lib[] = {"libEGL.dll", "libEGL_translator.dll"};
		#elif defined(__ANDROID__)
		const char *libEGL_lib[] = {"/vendor/lib/egl/libEGL_swiftshader.so"};
		#elif defined(__LP64__)
		const char *libEGL_lib[] = {"lib64EGL_translator.so", "libEGL.so.1", "libEGL.so"};
		#else
		const char *libEGL_lib[] = {"libEGL_translator.so", "libEGL.so.1", "libEGL.so"};
		#endif

		libEGL = loadLibrary(libEGL_lib);
		libEGLexports = (LibEGLexports *(*)())getProcAddress(libEGL, "libEGLexports");
	}

	return libEGLexports();
}