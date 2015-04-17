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

#ifndef SharedLibrary_hpp
#define SharedLibrary_hpp

#if defined(_WIN32)
	#include <Windows.h>
	#include <Psapi.h>
#else
	#include <dlfcn.h>
#endif

void *loadLibrary(const char *path);
void freeLibrary(void *library);
void *getProcAddress(void *library, const char *name);

template<int n>
void *loadLibrary(const char *(&names)[n], const char *mustContainSymbol = nullptr)
{
	HANDLE process = GetCurrentProcess();
	DWORD count = 0;
	EnumProcessModules(process, nullptr, 0, &count);
	HMODULE *modules = new HMODULE[count];
	EnumProcessModules(process, modules, count * sizeof(HMODULE), &count);

	for(int i = 0; i < count; i++)
	{
		if(GetProcAddress(modules[i], mustContainSymbol))
		{
			HMODULE winner;

			GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS , (LPCSTR)modules[i], &winner);

			delete[] modules;

			return winner;
		}
	}

	delete[] modules;

	//////////////////////////////////////////////////////////////

	

	for(int i = 0; i < n; i++)
	{
		void *library = loadLibrary(names[i]);

		if(library)
		{
			if(!mustContainSymbol || getProcAddress(library, mustContainSymbol))
			{
				return library;
			}

			freeLibrary(library);
		}
	}

	return 0;
}

#if defined(_WIN32)
	inline void *loadLibrary(const char *path)
	{
		return (void*)LoadLibrary(path);
	}

	inline void freeLibrary(void *library)
	{
		FreeLibrary((HMODULE)library);
	}

	inline void *getProcAddress(void *library, const char *name)
	{
		return (void*)GetProcAddress((HMODULE)library, name);
	}
#else
	inline void *loadLibrary(const char *path)
	{
		return dlopen(path, RTLD_LAZY);
	}

    inline void freeLibrary(void *library)
    {
        if(library)
        {
            dlclose(library);
        }
    }

	inline void *getProcAddress(void *library, const char *name)
	{
		return library ? dlsym(library, name) : 0;
	}
#endif

#endif   // SharedLibrary_hpp
