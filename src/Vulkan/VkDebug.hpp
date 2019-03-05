// Copyright 2016 The SwiftShader Authors. All Rights Reserved.
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

// debug.h: Debugging utilities.

#ifndef VK_DEBUG_H_
#define VK_DEBUG_H_

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#if !defined(TRACE_OUTPUT_FILE)
#define TRACE_OUTPUT_FILE "debug.txt"
#endif

namespace vk
{
// Outputs text to the debugging log
void trace(const char *format, ...);
inline void trace() {}

// Outputs the fatal message to the debugging log and stderr, and calls abort().
void fatal(const char *format, ...);
}

// A macro to output a trace of a function call and its arguments to the debugging log
#if defined(SWIFTSHADER_DISABLE_TRACE)
#define TRACE(message, ...) (void(0))
#else
#define TRACE(message, ...) vk::trace("%s:%d TRACE: " message "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#endif

// A macro to output a function call and its arguments to the debugging log, to denote an item in need of fixing.
#if defined(SWIFTSHADER_DISABLE_TRACE)
#define FIXME(message, ...) (void(0))
#else
#define FIXME(message, ...) vk::trace("%s:%d FIXME: " message "\n", __FILE__, __LINE__, ##__VA_ARGS__);
#endif

// A macro to output a function call and its arguments to the debugging log, in case of error.
#if defined(SWIFTSHADER_DISABLE_TRACE)
#define ERR(message, ...) (void(0))
#else
#define ERR(message, ...) vk::trace("%s:%d ERR: " message "\n", __FILE__, __LINE__, ##__VA_ARGS__);
#endif

// A macro that prints the fatal message and immediately aborts execution of the application.
#undef FATAL
#define FATAL(message, ...) vk::fatal("%s:%d FATAL: " message "\n", __FILE__, __LINE__, ##__VA_ARGS__)

// A macro asserting a condition. If the condition fails, the condition and
// provided printf message is printed to both the log and stderr, and the
// application is aborted.
// ASSERT_MSG is a no-op if NDEBUG is defined and DCHECK_ALWAYS_ON is not
// defined.
#undef ASSERT_MSG
#if !defined(NDEBUG) || defined(DCHECK_ALWAYS_ON)
#define ASSERT_MSG(expression, format, ...) do { \
	if(!(expression)) { \
		vk::fatal("%s:%d ASSERT(" #expression "): " format "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
	} } while(0)
#else
#define ASSERT_MSG(...) (void(0))
#endif

// A macro asserting a condition. If the condition fails, the condition is
// printed to both the log and stderr, and the application is aborted.
// ASSERT is a no-op if NDEBUG is defined and DCHECK_ALWAYS_ON is not defined.
#undef ASSERT
#if !defined(NDEBUG) || defined(DCHECK_ALWAYS_ON)
#define ASSERT(expression) do { \
	if(!(expression)) { \
		vk::fatal("%s:%d ASSERT(" #expression ")\n", __FILE__, __LINE__); \
	} } while(0)
#else
#define ASSERT(...) (void(0))
#endif

// A macro to indicate unimplemented functionality
#undef UNIMPLEMENTED
#if !defined(NDEBUG) || defined(DCHECK_ALWAYS_ON)
	#define UNIMPLEMENTED(...) FATAL("UNIMPLEMENTED: " __VA_ARGS__)
#else
	#define UNIMPLEMENTED(...) FIXME("UNIMPLEMENTED: " __VA_ARGS__)
#endif

// A macro for code which is not expected to be reached under valid assumptions
#undef UNREACHABLE
#if !defined(NDEBUG) || defined(DCHECK_ALWAYS_ON)
	#define UNREACHABLE(value) FATAL("UNREACHABLE: %s: %d" #value, value)
#else
	#define UNREACHABLE(value) ERR("UNREACHABLE: %s: %d", #value, value)
#endif

// A macro asserting a condition and outputting failures to the debug log, or return when in release mode.
#undef ASSERT_OR_RETURN
#if !defined(NDEBUG) || defined(DCHECK_ALWAYS_ON)
#define ASSERT_OR_RETURN(expression) ASSERT(expression)
#else
#define ASSERT_OR_RETURN(expression) do { \
	if(!(expression)) { \
		return; \
	} } while(0)
#endif

#endif   // VK_DEBUG_H_
