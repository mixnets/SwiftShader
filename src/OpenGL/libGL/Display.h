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

// Display.h: Defines the egl::Display class, representing the abstract
// display on which graphics are drawn. Implements EGLDisplay.
// [EGL 1.4] section 2.1.2 page 3.

#ifndef INCLUDE_DISPLAY_H_
#define INCLUDE_DISPLAY_H_

#include "Surface.h"
#include "Context.h"
#include "Device.hpp"

#include <set>

namespace egl
{
	struct DisplayMode
	{
		unsigned int width;
		unsigned int height;
		sw::Format format;
	};

	class Display
	{
	public:
		~Display();

		static egl::Display *getDisplay(NativeDisplayType displayId);

		bool initialize();
		void terminate();

		gl::Context *createContext(const gl::Context *shareContext);

		void destroySurface(egl::Surface *surface);
		void destroyContext(gl::Context *context);

		bool isInitialized() const;
		bool isValidContext(gl::Context *context);
		bool isValidSurface(egl::Surface *surface);
		bool isValidWindow(NativeWindowType window);

		GLint getMinSwapInterval();
		GLint getMaxSwapInterval();

        virtual egl::Surface *getPrimarySurface();

		NativeDisplayType getNativeDisplay() const;

	private:
		Display(NativeDisplayType displayId);
		
		DisplayMode getDisplayMode() const;

		const NativeDisplayType displayId;

		GLint mMaxSwapInterval;
		GLint mMinSwapInterval;
    
		typedef std::set<Surface*> SurfaceSet;
		SurfaceSet mSurfaceSet;

		typedef std::set<gl::Context*> ContextSet;
		ContextSet mContextSet;
	};
}

#endif   // INCLUDE_DISPLAY_H_
