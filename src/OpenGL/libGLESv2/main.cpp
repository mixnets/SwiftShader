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

namespace es2
{
es2::Context *getContext()
{
	egl::Context *context = libEGL->getCurrentContext();

	if(context && context->getClientVersion() == 2)
	{
		return static_cast<es2::Context*>(context);
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

namespace egl
{
GLint getClientVersion()
{
	Context *context = libEGL->getCurrentContext();

    return context ? context->getClientVersion() : 0;
}
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

LibEGL::LibEGL()
{
	libEGL = 0;
	libEGLlibrary = 0;
}

LibEGL::~LibEGL()
{
	freeLibrary(libEGLlibary);
}

egl::LibEGL *LibEGL::operator->()
{
	return load();
}

egl::LibEGL *LibEGL::operator egl::LibEGL*()
{
	return load();
}

egl::LibEGL *LibEGL::load()
{
	if(!libEGL)
	{
		egl::LibEGL *(*libEGLexport()) = (egl::LibEGL*(*())getProcAddress(NULL, "libEGL");

		if(!libEGLexport && !libEGLlib)
		{
			#if defined(_WIN32)
			const char *libEGLnames[] = {"libEGL.dll", "libEGL_translator.dll", 0};
			#elif defined(__LP64__)
			const char *libEGLnames[] = {"libEGL.so.1", "libEGL.so", "lib64EGL_translator.so", 0};
			#else
			const char *libEGLnames[] = {"libEGL.so.1", "libEGL.so", "libEGL_translator.so", 0};
			#endif

			libEGLlib = loadLibrary(libEGLnames);
			libEGLexport = getProcAddress(libEGLlibrary, "libEGL");
		}

		libEGL = libEGLexport();
	}

	return libEGL;
}

LibEGL libEGL;

LibGLES_CMdependencies::LibGLES_CMdependencies()
{
	#if defined(_WIN32)
	const char *libGLES_CM_lib[] = {"libGLES_CM.dll", "libGLES_CM_translator.dll", 0};
	#elif defined(__LP64__)
	const char *libGLES_CM_lib[] = {"libGLES_CM.so.1", "libGLES_CM.so", "lib64GLES_CM_translator.so", 0};
	#else
	const char *libGLES_CM_lib[] = {"libGLES_CM.so.1", "libGLES_CM.so", "libGLES_CM_translator.so", 0};
	#endif

	libGLES_CM = loadLibrary(libGLES_CM_lib);

	glEGLImageTargetTexture2DOES = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)getProcAddress(libGLES_CM, "glEGLImageTargetTexture2DOES");
}

LibGLES_CMdependencies::~LibGLES_CMdependencies()
{
	freeLibrary(libGLES_CM);
}

LibGLES_CMdependencies *LibGLES_CMdependencies::getSingleton()
{
	static LibGLES_CMdependencies singleton;

	return &singleton;
}

LibGLES_CM libGLES_CM;
