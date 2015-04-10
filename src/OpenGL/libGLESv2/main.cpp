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
#include "libEGL/main.h"
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

namespace es2
{
es2::Context *getContext()
{
	egl::Context *context = libEGL->clientGetCurrentContext();

	if(context && (context->getClientVersion() == 2 ||
	               context->getClientVersion() == 3))
	{
		return static_cast<es2::Context*>(context);
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
    es2::Context *context = es2::getContext();

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
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            context->recordInvalidFramebufferOperation();
            TRACE("\t! Error generated: invalid framebuffer operation\n");
            break;
        default: UNREACHABLE();
        }
    }
}
}

namespace egl
{
GLint getClientVersion()
{
	Context *context = libEGL->clientGetCurrentContext();

    return context ? context->getClientVersion() : 0;
}
}

LibGLESv2exports::LibGLESv2exports()
{
	this->es2CreateContext = ::es2CreateContext;
	this->es2GetProcAddress = ::es2GetProcAddress;
	this->createBackBuffer = ::createBackBuffer;
	this->createDepthStencil = ::createDepthStencil;
	this->createFrameBuffer = ::createFrameBuffer;
}

LibGLESv2exports *LibGLESv2exports::getSingleton()
{
	static LibGLESv2exports libGLESv2;
	return &libGLESv2;
}

extern "C" LibGLESv2exports *libGLESv2exports()
{
	return LibGLESv2exports::getSingleton();
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

LibGLES_CM libGLES_CM;
void *LibGLES_CM::libGLES_CM = nullptr;

LibGLES_CM::~LibGLES_CM()
{
	freeLibrary(libGLES_CM);
}

LibGLES_CMexports *LibGLES_CM::operator->()
{
	static LibGLES_CMexports *(*libGLES_CMexports)() = nullptr;

	if(!libGLES_CM)
	{
		#if defined(_WIN32)
		const char *libGLES_CM_lib[] = {"libGLES_CM.dll", "libGLES_CM_translator.dll"};
		#elif defined(__ANDROID__)
		const char *libGLES_CM_lib[] = {"/vendor/lib/egl/libGLESv1_CM_swiftshader.so"};
		#elif defined(__LP64__)
		const char *libGLES_CM_lib[] = {"lib64GLES_CM_translator.so", "libGLES_CM.so.1", "libGLES_CM.so"};
		#else
		const char *libGLES_CM_lib[] = {"libGLES_CM_translator.so", "libGLES_CM.so.1", "libGLES_CM.so"};
		#endif

		libGLES_CM = loadLibrary(libGLES_CM_lib);
		libGLES_CMexports = (LibGLES_CMexports *(*)())getProcAddress(libGLES_CM, "libGLES_CMexports");
	}

	return libGLES_CMexports();
}
