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

#ifndef Debug_hpp
#define Debug_hpp

#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <d3d9.h>
#include <stdio.h>
#include <guiddef.h>
#include <assert.h>

#define APPEND(x, y) x ## y
#define MACRO_APPEND(x, y) APPEND(x, y)
#define UNIQUE_IDENTIFIER(prefix) MACRO_APPEND(prefix, __COUNTER__)

class Trace
{
public:
	Trace(const char *function, const void *object, const char *format, ...);

	~Trace();

	static void nextFrame();

private:
	static void log(char type, const char *function, const void *object, const char *arguments);

	bool tracing;

	static FILE *trace;
	static int indent;
	static bool comma;

	static int frame;
	static double frequency;
};

#ifndef NDEBUG
	#define TRACE(format, ...) Trace UNIQUE_IDENTIFIER(_tracer_)(__FUNCTION__, this, format, ##__VA_ARGS__)
	#define GTRACE(format, ...) Trace(__FUNCTION__, 0, format, ##__VA_ARGS__)
#else
	#define TRACE(...) ((void)0)
	#define GTRACE(...) ((void)0)
#endif

#ifndef NDEBUG
	#define ASSERT(expression) {if(!(expression)) Trace(__FUNCTION__, "! Assert failed at %s(%d): "#expression, __FILE__, __LINE__); assert(expression);}
#else
	#define ASSERT assert
#endif

#ifndef NDEBUG
	#define UNIMPLEMENTED() {Trace(__FUNCTION__, "! Unimplemented: %s(%d)", __FILE__, __LINE__); ASSERT(false);}
#else
	#define UNIMPLEMENTED() ((void)0)
#endif

#ifndef NDEBUG
	#define NOINTERFACE(iid) _NOINTERFACE(__FUNCTION__, iid)

	inline long _NOINTERFACE(const char *function, const IID &iid)
	{
		Trace(__FUNCTION__, 0, "! No interface {0x%0.8X, 0x%0.4X, 0x%0.4X, 0x%0.2X, 0x%0.2X, 0x%0.2X, 0x%0.2X, 0x%0.2X, 0x%0.2X, 0x%0.2X, 0x%0.2X} for %s", iid.Data1, iid.Data2, iid.Data3, iid.Data4[0], iid.Data4[1], iid.Data4[2], iid.Data4[3], iid.Data4[4], iid.Data4[5], iid.Data4[6], iid.Data4[7], function);

		return E_NOINTERFACE;
	}
#else
	#define NOINTERFACE(iid) E_NOINTERFACE
#endif

#ifndef NDEBUG
	inline long INVALIDCALL()
	{
		Trace(__FUNCTION__, 0, "! D3DERR_INVALIDCALL");

		return D3DERR_INVALIDCALL;
	}
#else
	#define INVALIDCALL() D3DERR_INVALIDCALL
#endif

#ifndef NDEBUG
	inline long OUTOFMEMORY()
	{
		Trace(__FUNCTION__, 0, "! E_OUTOFMEMORY");

		return E_OUTOFMEMORY;
	}
#else
	#define OUTOFMEMORY() E_OUTOFMEMORY
#endif

#ifndef NDEBUG
	inline long OUTOFVIDEOMEMORY()
	{
		Trace(__FUNCTION__, 0, "! D3DERR_OUTOFVIDEOMEMORY");

		return D3DERR_OUTOFVIDEOMEMORY;
	}
#else
	#define OUTOFVIDEOMEMORY() D3DERR_OUTOFVIDEOMEMORY
#endif

#ifndef NDEBUG
	inline long NOTAVAILABLE()
	{
		Trace(__FUNCTION__, 0, "! D3DERR_NOTAVAILABLE");

		return D3DERR_NOTAVAILABLE;
	}
#else
	#define NOTAVAILABLE() D3DERR_NOTAVAILABLE
#endif

#ifndef NDEBUG
	inline long NOTFOUND()
	{
		Trace(__FUNCTION__, 0, "! D3DERR_NOTFOUND");

		return D3DERR_NOTFOUND;
	}
#else
	#define NOTFOUND() D3DERR_NOTFOUND
#endif

#ifndef NDEBUG
	inline long MOREDATA()
	{
		Trace(__FUNCTION__, 0, "! D3DERR_MOREDATA");

		return D3DERR_MOREDATA;
	}
#else
	#define MOREDATA() D3DERR_MOREDATA
#endif

#ifndef NDEBUG
	inline long FAIL()
	{
		Trace(__FUNCTION__, 0, "! E_FAIL");

		return E_FAIL;
	}
#else
	#define FAIL() E_FAIL
#endif

#endif   // Debug_hpp
