// Copyright 2019 The SwiftShader Authors. All Rights Reserved.
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

#include "Thread.hpp"

#include "Trace.hpp"

#include <cstdarg>

#if defined(_WIN32)
#   ifndef WIN32_LEAN_AND_MEAN
#       define WIN32_LEAN_AND_MEAN
#   endif
#   include <windows.h>
#else
#   include <pthread.h>
#endif

namespace yarn {

#if defined(_WIN32)

void Thread::setName(const char* fmt, ...)
{
    char name[1024];
    va_list vararg;
    va_start(vararg, fmt);
    vsnprintf(name, sizeof(name), fmt, vararg);
    va_end(vararg);

	wchar_t wname[1024];
	mbstowcs(wname, name, 1024);
	SetThreadDescription(GetCurrentThread(), wname);
    NAME_THREAD("%s", name);
}

#else

void Thread::setName(const char* fmt, ...)
{
    char name[1024];
    va_list vararg;
    va_start(vararg, fmt);
    vsnprintf(name, sizeof(name), fmt, vararg);
    va_end(vararg);

#if defined(__APPLE__)
    pthread_setname_np(name);
#else
    pthread_setname_np(pthread_self(), name);
#endif

    NAME_THREAD("%s", name);
}

void Thread::setAffinity(uint64_t mask)
{
    cpu_set_t set;
    CPU_ZERO(&set);
    for (uint64_t i = 0; i < 64; i++)
    {
        if (mask >> i != 0)
        {
            CPU_SET(i, &set);
        }
    }
    sched_setaffinity(0, sizeof(cpu_set_t), &set);
}

#endif

} // namespace yarn
