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

#include <stdio.h>
#include <stdarg.h>

static bool fileInit;
static FILE* file = NULL;

namespace es
{
	static bool maxInit;
	static FILE* outfile;

	static void outputMax(const char *format, va_list vararg)
	{
		if(!maxInit)
		{
			outfile = fopen("C:\\Users\\mgregoire\\list_time.txt", "w");
			maxInit = true;
		}

		vfprintf(outfile, format, vararg);
	}

	static void output(const char *format, va_list vararg)
	{
		if(false)
		{
			if(file)
			{
				vfprintf(file, format, vararg);
			}
			else
			{
				file = fopen(TRACE_OUTPUT_FILE, "w");
				fileInit = true;
			}
		}
	}

	void trace(const char *format, ...)
	{
		va_list vararg;
		va_start(vararg, format);
		output(format, vararg);
		va_end(vararg);
	}

	void maxTrace(const char *format, ...)
	{
		va_list vararg;
		va_start(vararg, format);
		outputMax(format, vararg);
		va_end(vararg);
	}
}
