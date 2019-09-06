// Copyright 2018 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "VkDebug.hpp"

#include <string>
#include <stdarg.h>

#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
#define PTRACE
#include <sys/types.h>
#include <sys/ptrace.h>
#elif defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

namespace vk
{

void tracev(const char *format, va_list args)
{
#ifndef SWIFTSHADER_DISABLE_TRACE
	if(false)
	{
		FILE *file = fopen(TRACE_OUTPUT_FILE, "a");

		if(file)
		{
			vfprintf(file, format, args);
			fclose(file);
		}
	}
#endif
}

void trace(const char *format, ...)
{
	va_list vararg;
	va_start(vararg, format);
	tracev(format, vararg);
	va_end(vararg);
}

void warn(const char *format, ...)
{
	va_list vararg;
	va_start(vararg, format);
	tracev(format, vararg);
	va_end(vararg);

	va_start(vararg, format);
	vfprintf(stderr, format, vararg);
	va_end(vararg);
}

void abort(const char *format, ...)
{
	va_list vararg;

	va_start(vararg, format);
	tracev(format, vararg);
	va_end(vararg);

	va_start(vararg, format);
	vfprintf(stderr, format, vararg);
	va_end(vararg);

	::abort();
}

bool IsUnderDebugger();

void trace_assert(const char *format, ...)
{
	static bool asserted = false;
	va_list vararg;
	va_start(vararg, format);

	if (!asserted && IsUnderDebugger())
	{
		asserted = true;
		warn(format, vararg);
	}
	else if (!asserted)
	{
		tracev(format, vararg);
	}

	va_end(vararg);
}


bool IsUnderDebugger()
{
#if defined(PTRACE)
	static bool checked = false;
	static bool res = false;

	if (!checked)
	{
		// If a debugger is attached then we're already being ptraced and ptrace
		// will return a non-zero value.
		checked = true;
		if (ptrace(PTRACE_TRACEME, 0, 1, 0) != 0)
		{
			res = true;
		}
		else
		{
			ptrace(PTRACE_DETACH, 0, 1, 0);
		}
	}

	return res;
#elif defined(_WIN32) || defined(_WIN64)
	return IsDebuggerPresent() != 0;
#else
	return false;
#endif
}

}
