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

#if defined(_WIN32)
	#include <Windows.h>
#else
	#include <dlfcn.h>
#endif

void *getLibraryHandle(const char *path);
void *loadLibrary(const char *path);

inline void *loadLibrary(const char **names)
{
	for(int i = 0; names[i]; i++)
	{
		void *library = getLibraryHandle(names[i]);

		if(library)
		{
			return loadLibrary(names[i]);
		}
	}

	for(int i = 0; names[i]; i++)
	{
		void *library = loadLibrary(names[i]);

		if(library)
		{
			return library;
		}
	}

	return 0;
}

inline void *getLibraryHandle(const char **names)
{
	for(int i = 0; names[i]; i++)
	{
		void *library = getLibraryHandle(names[i]);

		if(library)
		{
			return library;
		}
	}

	return 0;
}

#if defined(_WIN32)
	inline void *loadLibrary(const char *path)
	{
		return (void*)LoadLibrary(path);
	}

	inline void *getLibraryHandle(const char *path)
	{
		return (void*)GetModuleHandle(path);
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

	inline void *getLibraryHandle(const char *path)
	{
		#if defined(__ANDROID__) || defined(ANDROID)
			// Bionic does not support RTLD_NOLAD before Lollipop.
			// Symbols of resident libraries can be resolved with a null handle
			// if loaded by the linker or if dlopen(lib, RTLD_GLOBAL) is used.
			return NULL;
		#else
			return dlopen(path, RTLD_LAZY | RTLD_NOLOAD);
		#endif
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
		return dlsym(library, name);
	}
#endif
