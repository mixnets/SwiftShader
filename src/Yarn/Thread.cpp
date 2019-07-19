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
#elif defined(__APPLE__)
#   include <pthread.h>
#   include <mach/thread_act.h>
#   include <unistd.h>
#else
#   include <pthread.h>
#   include <unistd.h>
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

void Thread::setAffinity(const AffinityMask& mask)
{
    DWORD_PTR imask = 0;
    for (uint64_t i = 0; i < mask.size(); i++)
    {
        if (mask[i])
        {
            imask |= 1ULL << i;
        }
    }
    SetThreadAffinityMask(GetCurrentThread(), imask);
}

unsigned int Thread::numLogicalCPUs()
{
    DWORD_PTR processAffinityMask = 1;
    DWORD_PTR systemAffinityMask = 1;

    GetProcessAffinityMask(GetCurrentProcess(), &processAffinityMask, &systemAffinityMask);

    auto count = 0;
    while (processAffinityMask > 0)
    {
        if (processAffinityMask & 1)
        {
            count++;
        }

        processAffinityMask >>= 1;
    }
    return count;
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

void Thread::setAffinity(const AffinityMask& mask)
{
#if defined(__APPLE__)
    integer_t imask = 0;
    for (uint64_t i = 0; i < mask.size(); i++)
    {
        if (mask[i])
        {
            imask |= 1 << i;
        }
    }
    thread_affinity_policy_data_t policy = { imask };
    auto thread = pthread_mach_thread_np(pthread_self());
    thread_policy_set(thread, THREAD_AFFINITY_POLICY, (thread_policy_t)&policy, 1);
#else
    cpu_set_t set;
    CPU_ZERO(&set);
    for (uint64_t i = 0; i < mask.size(); i++)
    {
        if (mask[i])
        {
            CPU_SET(i, &set);
        }
    }
    sched_setaffinity(0, sizeof(cpu_set_t), &set);
#endif
}

unsigned int Thread::numLogicalCPUs()
{
    return sysconf(_SC_NPROCESSORS_ONLN);
}

#endif

} // namespace yarn
