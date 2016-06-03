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

// debug.cpp: Debugging utilities.

#include "common/debug.h"

#ifdef  __ANDROID__
#include <utils/String8.h>
#include <cutils/log.h>
#endif

 #include <sys/types.h>
       #include <unistd.h>

#include <stdio.h>
#include <stdarg.h>
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))




#include <mutex>



namespace es
{
	static std::mutex m;

	static void output(const char *format, va_list vararg)
	{
		if(true)
		{
			m.lock();
			static FILE* file = nullptr;
			if(!file)
			{
				file = fopen("/usr/local/google/home/nicolascapens/tmp/debug.txt", "a");
				if(!file)printf("Noooo, cannot open debug.txt!!!!\n");
			}

			if(file)
			{
				vfprintf(file, format, vararg);

				fflush(file);
			}
			m.unlock();
		}
	}

	void trace(const char *format, ...)
	{
		va_list vararg;
		va_start(vararg, format);
		output(format, vararg);
		va_end(vararg);
	}
}
