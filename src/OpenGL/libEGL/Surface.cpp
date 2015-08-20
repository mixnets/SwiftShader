// SwiftShader Software Renderer
//
// Copyright(c) 2005-2013 TransGaming Inc.
//
// All rights reserved. No part of this software may be copied, distributed, transmitted,
// transcribed, stored in a retrieval system, translated into any human or computer
// language by any means, or disclosed to third parties without the explicit written
// agreement of TransGaming Inc. Without such an agreement, no rights or licenses, express
// or implied, including but not limited to any patent rights, are granted to you.
//

// Surface.cpp: Implements the egl::Surface class, representing a drawing surface
// such as the client area of a window, including any back buffers.
// Implements EGLSurface and related functionality. [EGL 1.4] section 2.2 page 3.

#include "Surface.h"

#include "main.h"
#include "Display.h"
#include "Texture.hpp"
#include "common/Image.hpp"
#include "Context.hpp"
#include "common/debug.h"
#include "Main/FrameBuffer.hpp"

#if defined(__unix__) && !defined(__ANDROID__)
#include "Main/libX11.hpp"
#endif

#if defined(_WIN32)
#include <tchar.h>
#endif

#include <algorithm>

namespace egl
{

Surface::Surface(Display *display) : display(display)
{
}

Surface::~Surface()
{
    deleteResources();
}

bool Surface::initialize()
{
    ASSERT(!frameBuffer && !backBuffer && !depthStencil);

    return reset();
}

void Surface::deleteResources()
{
    if(depthStencil)
    {
        depthStencil->release();
        depthStencil = nullptr;
    }

    if(texture)
    {
        texture->releaseTexImage();
        texture = nullptr;
    }

	if(backBuffer)
	{
		backBuffer->release();
		backBuffer = 0;
	}

	delete frameBuffer;
	frameBuffer = 0;
}

bool Surface::reset()
{
    if(!window)
    {
        return reset(width, height);
    }

	// FIXME: Wrap into an abstract Window class
	#if defined(_WIN32)
		RECT windowRect;
		GetClientRect(window, &windowRect);

		return reset(windowRect.right - windowRect.left, windowRect.bottom - windowRect.top);
	#elif defined(__ANDROID__)
		return reset(ANativeWindow_getWidth(window), ANativeWindow_getHeight(window));
	#else
		XWindowAttributes windowAttributes;
		libX11->XGetWindowAttributes(display->getNativeDisplay(), window, &windowAttributes);

		return reset(windowAttributes.width, windowAttributes.height);
	#endif
}

bool Surface::reset(int backBufferWidth, int backBufferHeight)
{
    deleteResources();

    if(window)
    {
		if(libGLES_CM)
		{
			frameBuffer = libGLES_CM->createFrameBuffer(display->getNativeDisplay(), window, backBufferWidth, backBufferHeight);
		}
		else if(libGLESv2)
		{
			frameBuffer = libGLESv2->createFrameBuffer(display->getNativeDisplay(), window, backBufferWidth, backBufferHeight);
		}

		if(!frameBuffer)
		{
			ERR("Could not create frame buffer");
			deleteResources();
			return error(EGL_BAD_ALLOC, false);
		}
    }

	if(libGLES_CM)
	{
		backBuffer = libGLES_CM->createBackBuffer(backBufferWidth, backBufferHeight, config);
	}
	else if(libGLESv2)
	{
		backBuffer = libGLESv2->createBackBuffer(backBufferWidth, backBufferHeight, config);
	}

    if(!backBuffer)
    {
        ERR("Could not create back buffer");
        deleteResources();
        return error(EGL_BAD_ALLOC, false);
    }

    if(config->mDepthStencilFormat != sw::FORMAT_NULL)
    {
		if(libGLES_CM)
		{
			depthStencil = libGLES_CM->createDepthStencil(backBufferWidth, backBufferHeight, config->mDepthStencilFormat, 1, false);
		}
		else if(libGLESv2)
		{
			depthStencil = libGLESv2->createDepthStencil(backBufferWidth, backBufferHeight, config->mDepthStencilFormat, 1, false);
		}

		if(!depthStencil)
		{
			ERR("Could not create depth/stencil buffer for surface");
			deleteResources();
			return error(EGL_BAD_ALLOC, false);
		}
    }

    width = backBufferWidth;
    height = backBufferHeight;

    return true;
}

EGLNativeWindowType Surface::getWindowHandle()
{
    return window;
}

void Surface::swap()
{
	if(backBuffer)
    {
		void *source = backBuffer->lockInternal(0, 0, 0, sw::LOCK_READONLY, sw::PUBLIC);
		frameBuffer->flip(source, backBuffer->Surface::getInternalFormat());
		backBuffer->unlockInternal();

        checkForResize();
	}
}

egl::Image *Surface::getRenderTarget()
{
    if(backBuffer)
    {
        backBuffer->addRef();
    }

    return backBuffer;
}

egl::Image *Surface::getDepthStencil()
{
    if(depthStencil)
    {
        depthStencil->addRef();
    }

    return depthStencil;
}

void Surface::setSwapBehavior(EGLenum swapBehavior)
{
	swapBehavior = swapBehavior;
}

void Surface::setSwapInterval(EGLint interval)
{
    if(swapInterval == interval)
    {
        return;
    }

    swapInterval = interval;
    swapInterval = std::max(swapInterval, display->getMinSwapInterval());
    swapInterval = std::min(swapInterval, display->getMaxSwapInterval());
}

EGLint Surface::getConfigID() const
{
    return config->mConfigID;
}

EGLenum Surface::getSurfaceType() const
{
    return config->mSurfaceType;
}

sw::Format Surface::getInternalFormat() const
{
    return config->mRenderTargetFormat;
}

EGLint Surface::getWidth() const
{
    return width;
}

EGLint Surface::getHeight() const
{
    return height;
}

EGLint Surface::getPixelAspectRatio() const
{
    return pixelAspectRatio;
}

EGLenum Surface::getRenderBuffer() const
{
    return renderBuffer;
}

EGLenum Surface::getSwapBehavior() const
{
    return swapBehavior;
}

EGLenum Surface::getTextureFormat() const
{
    return textureFormat;
}

EGLenum Surface::getTextureTarget() const
{
    return textureTarget;
}

void Surface::setBoundTexture(egl::Texture *texture)
{
    this->texture = texture;
}

egl::Texture *Surface::getBoundTexture() const
{
    return texture;
}

bool Surface::checkForResize()
{
    #if defined(_WIN32)
		RECT client;
		if(!GetClientRect(window, &client))
		{
			ASSERT(false);
			return false;
		}

		int clientWidth = client.right - client.left;
		int clientHeight = client.bottom - client.top;
	#elif defined(__ANDROID__)
		int clientWidth = ANativeWindow_getWidth(window);
		int clientHeight = ANativeWindow_getHeight(window);
	#else
		XWindowAttributes windowAttributes;
		libX11->XGetWindowAttributes(display->getNativeDisplay(), window, &windowAttributes);

		int clientWidth = windowAttributes.width;
		int clientHeight = windowAttributes.height;
	#endif

	bool sizeDirty = clientWidth != getWidth() || clientHeight != getHeight();

    if(sizeDirty)
    {
        reset(clientWidth, clientHeight);

        if(static_cast<egl::Surface*>(getCurrentDrawSurface()) == this)
        {
			static_cast<egl::Context*>(getCurrentContext())->makeCurrent(this);
        }

        return true;
    }

    return false;
}

WindowSurface::WindowSurface(Display *display, const Config *config, EGLNativeWindowType window)
    : Surface(display), config(config), window(window)
{
    frameBuffer = 0;
	backBuffer = 0;

    depthStencil = nullptr;
    texture = nullptr;
    textureFormat = EGL_NO_TEXTURE;
    textureTarget = EGL_NO_TEXTURE;

    pixelAspectRatio = (EGLint)(1.0 * EGL_DISPLAY_SCALING);   // FIXME: Determine actual pixel aspect ratio
    renderBuffer = EGL_BACK_BUFFER;
    swapBehavior = EGL_BUFFER_PRESERVED;
    swapInterval = -1;
    setSwapInterval(1);
}

PBufferSurface::PBufferSurface(Display *display, const Config *config, EGLint width, EGLint height, EGLenum textureFormat, EGLenum textureType, EGLBoolean largestPBuffer)
    : Surface(display), window(nullptr), config(config), width(width), height(height), largestPBuffer(largestPBuffer)
{
	frameBuffer = 0;
	backBuffer = 0;

    depthStencil = nullptr;
    windowSubclassed = false;
    texture = nullptr;
    textureFormat = textureFormat;
    textureTarget = textureType;

    pixelAspectRatio = (EGLint)(1.0 * EGL_DISPLAY_SCALING);   // FIXME: Determine actual pixel aspect ratio
    renderBuffer = EGL_BACK_BUFFER;
    swapBehavior = EGL_BUFFER_PRESERVED;
    swapInterval = -1;
    setSwapInterval(1);
}

}
