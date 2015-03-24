#ifndef egl_Image_hpp
#define egl_Image_hpp

#include "Renderer/Surface.hpp"

#include <assert.h>
#if defined(HAVE_ANDROID_OS)
	#include <hardware/gralloc.h>
	#include <system/window.h>
#endif

namespace egl
{
// Types common between gl.h and gl2.h
// We can't include either header in EGL
typedef unsigned int GLenum;
typedef int GLint;
typedef int GLsizei;

class Texture;

class Image : public sw::Surface
{
public:
	Image(sw::Resource *resource, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, sw::Format internalFormat)
		: width(width), height(height), format(format), type(type), internalFormat(internalFormat), depth(depth)
		, sw::Surface(resource, width, height, depth, internalFormat, true, true)
	{
		shared = false;
		#if defined(HAVE_ANDROID_OS)
			nativeBuffer = NULL;
			hw_module_t const* pModule;
			hw_get_module(GRALLOC_HARDWARE_MODULE_ID, &pModule);
			module = reinterpret_cast<gralloc_module_t const*>(pModule);
		#endif
	}

	Image(sw::Resource *resource, int width, int height, int depth, sw::Format internalFormat, bool lockable, bool renderTarget)
		: width(width), height(height), format(0 /*GL_NONE*/), type(0 /*GL_NONE*/), internalFormat(internalFormat), depth(depth)
		, sw::Surface(resource, width, height, depth, internalFormat, lockable, renderTarget)
	{
		shared = false;
		#if defined(HAVE_ANDROID_OS)
			nativeBuffer = NULL;
			hw_module_t const* pModule;
			hw_get_module(GRALLOC_HARDWARE_MODULE_ID, &pModule);
			module = reinterpret_cast<gralloc_module_t const*>(pModule);
		#endif
	}

	GLsizei getWidth() const
	{
		return width;
	}

	GLsizei getHeight() const
	{
		return height;
	}

	int getDepth() const
	{
		// FIXME: add member if the depth dimension (for 3D textures or 2D testure arrays)
		// and multi sample depth are ever simultaneously required.
		return depth;
	}

	GLenum getFormat() const
	{
		return format;
	}

	GLenum getType() const
	{
		return type;
	}

	sw::Format getInternalFormat() const
	{
		return internalFormat;
	}

	bool isShared() const
    {
        return shared;
    }

    void markShared()
    {
        shared = true;
    }

	void *lock(unsigned int left, unsigned int top, sw::Lock lock)
	{
		#if defined(HAVE_ANDROID_OS)
			// Lock the buffer from ANativeWindowBuffer
			if (nativeBuffer)
			{
				void* buffer = NULL;
				nativeBuffer->common.incRef(&nativeBuffer->common);
				module->lock(module, nativeBuffer->handle, GRALLOC_USAGE_SW_READ_OFTEN, 0, 0, nativeBuffer->width, nativeBuffer->height, &buffer);
				return buffer;
			}
		#endif
		return lockExternal(left, top, 0, lock, sw::PUBLIC);
	}

	unsigned int getPitch() const
	{
		return getExternalPitchB();
	}

	void unlock()
	{
		#if defined(HAVE_ANDROID_OS)
			// Unlock the buffer from ANativeWindowBuffer
			if (nativeBuffer)
			{
				module->unlock(module, nativeBuffer->handle);
				nativeBuffer->common.decRef(&nativeBuffer->common);
				return;
			}
		#endif
		unlockExternal();
	}

	virtual void addRef() = 0;
	virtual void release() = 0;
	virtual void unbind(const Texture *parent) = 0;   // Break parent ownership and release

	void destroyShared()   // Release a shared image
	{
		#if defined(HAVE_ANDROID_OS)
			if (nativeBuffer)
			{
				nativeBuffer->common.decRef(&nativeBuffer->common);
			}
		#else
			assert(shared);
		#endif
		shared = false;
		release();
	}

	virtual void loadImageData(GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, GLint unpackAlignment, const void *input) = 0;
	virtual void loadCompressedData(GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLsizei imageSize, const void *pixels) = 0;

	#if defined(HAVE_ANDROID_OS)
	void setNativeBuffer(ANativeWindowBuffer* buffer)
	{
		nativeBuffer = buffer;
		nativeBuffer->common.incRef(&nativeBuffer->common);
	}

	virtual void *lockInternal(int x, int y, int z, sw::Lock lock, sw::Accessor client)
	{
		// Lock the buffer from ANativeWindowBuffer
		if (nativeBuffer)
		{
			void* buffer = NULL;
			nativeBuffer->common.incRef(&nativeBuffer->common);
			module->lock(module, nativeBuffer->handle, GRALLOC_USAGE_SW_READ_OFTEN, 0, 0, nativeBuffer->width, nativeBuffer->height, &buffer);
			return buffer;
		}
		return sw::Surface::lockInternal(x, y, z, lock, client);
	}

	virtual void unlockInternal()
	{
		// Unlock the buffer from ANativeWindowBuffer
		if (nativeBuffer)
		{
			module->unlock(module, nativeBuffer->handle);
			nativeBuffer->common.decRef(&nativeBuffer->common);
			return;
		}
		return sw::Surface::unlockInternal();
	}

	#endif

protected:
	virtual ~Image()
	{
	}

	const GLsizei width;
	const GLsizei height;
	const GLenum format;
	const GLenum type;
	const sw::Format internalFormat;
	const int depth;

	bool shared;   // Used as an EGLImage

	#if defined(HAVE_ANDROID_OS)
	ANativeWindowBuffer *nativeBuffer;
	gralloc_module_t const* module;
	#endif
};
}

#endif   // egl_Image_hpp
