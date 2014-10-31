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

#if defined(_WIN32)
    template<int n>
	void *loadLibrary(const char *(&names)[n])
	{
		for(int i = 0; i < n; i++)
		{
			HMODULE module = 0;
			GetModuleHandleEx(0, names[i], &module);

			if(module)
            {
                return (void*)module;
            }
		}

		for(int i = 0; i < n; i++)
		{
			HMODULE module = LoadLibrary(names[i]);

			if(module)
            {
                return (void*)module;
            }
		}

		return 0;
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
	template<int n>
	void *loadLibrary(const char *(&names)[n])
	{
		for(int i = 0; i < n; i++)
		{
			void *module = dlopen(names[i], RTLD_NOLOAD);

			if(module)
            {
                return dlopen(names[i], RTLD_LAZY);
            }
		}

		for(int i = 0; i < n; i++)
		{
			void *module = dlopen(names[i], RTLD_LAZY);

			if(module)
            {
                return module;
            }
		}

		return 0;
	}

	inline void freeLibrary(void *library)
	{
		dlclose(library);
	}

	inline void *getProcAddress(void *library, const char *name)
	{
		return dlsym(library, name);
	}
#endif
