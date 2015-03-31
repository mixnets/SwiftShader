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

// debug.cpp: Debugging utilities.

#include "common/debug.h"

#ifdef  __ANDROID__
#include <utils/String8.h>
#include <cutils/log.h>
#endif

#if defined(_WIN32)
#include <Windows.h>
#else
#include <signal.h>
#endif

#include <stdio.h>
#include <stdarg.h>

namespace gl
{
#ifdef __ANDROID__
	void output(const char *format, va_list vararg)
	{
		ALOGI("%s", android::String8::formatV(format, vararg).string());
	}
#else
	static void output(const char *format, va_list vararg)
	{
		if(false)
		{
			static FILE* file = nullptr;
			if(!file)
			{
				file = fopen(TRACE_OUTPUT_FILE, "w");
			}

			if(file)
			{
				vfprintf(file, format, vararg);
			}
		}
	}
#endif

	void log(const char *format, ...)
	{
		va_list vararg;
		va_start(vararg, format);
		output(format, vararg);
		va_end(vararg);
	}
}

#ifndef NDEBUG
#if defined(_WIN32)
void HaltDebugger()
{
	if(IsDebuggerPresent())
	{
		DebugBreak();
	}
}
#else   // Assume POSIX
static void ResetSignalHandler(int signum)
{
    // Set the signal handler back to the default handler
    signal(SIGTRAP, SIG_DFL);
}
 
void HaltDebugger()
{
    // Raise a trap signal, using a bening handler to avoid aborting
    signal(SIGTRAP, ResetSignalHandler);
    raise(SIGTRAP);
}
#endif
#endif   // !NDEBUG