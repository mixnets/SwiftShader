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

// Renderbuffer.cpp: the Renderbuffer class and its derived classes
// Colorbuffer, Depthbuffer and Stencilbuffer. Implements GL renderbuffer
// objects and related functionality. [OpenGL ES 2.0.24] section 4.4.3 page 108.

#include "Renderbuffer.h"

#include "main.h"
#include "Texture.h"
#include "utilities.h"

namespace es2
{
RenderbufferInterface::RenderbufferInterface()
{
}

// The default case for classes inherited from RenderbufferInterface is not to
// need to do anything upon the reference count to the parent Renderbuffer incrementing
// or decrementing.
void RenderbufferInterface::addProxyRef(const Renderbuffer *proxy)
{
}

void RenderbufferInterface::releaseProxy(const Renderbuffer *proxy)
{
}

GLuint RenderbufferInterface::getRedSize() const
{
	return GetRedSize(getFormat(0));
}

GLuint RenderbufferInterface::getGreenSize() const
{
	return GetGreenSize(getFormat(0));
}

GLuint RenderbufferInterface::getBlueSize() const
{
	return GetBlueSize(getFormat(0));
}

GLuint RenderbufferInterface::getAlphaSize() const
{
	return GetAlphaSize(getFormat(0));
}

GLuint RenderbufferInterface::getDepthSize() const
{
	return GetDepthSize(getFormat(0));
}

GLuint RenderbufferInterface::getStencilSize() const
{
	return GetStencilSize(getFormat(0));
}

///// RenderbufferTexture2D Implementation ////////

RenderbufferTexture2D::RenderbufferTexture2D(Texture2D *texture)
{
	mTexture2D = texture;
}

RenderbufferTexture2D::~RenderbufferTexture2D()
{
	mTexture2D = NULL;
}

// Textures need to maintain their own reference count for references via
// Renderbuffers acting as proxies. Here, we notify the texture of a reference.
void RenderbufferTexture2D::addProxyRef(const Renderbuffer *proxy)
{
	mTexture2D->addProxyRef(proxy);
}

void RenderbufferTexture2D::releaseProxy(const Renderbuffer *proxy)
{
	mTexture2D->releaseProxy(proxy);
}

// Increments refcount on image.
// caller must release() the returned image
egl::Image *RenderbufferTexture2D::getRenderTarget(GLint level)
{
	return mTexture2D->getRenderTarget(GL_TEXTURE_2D, level);
}

// Increments refcount on image.
// caller must release() the returned image
egl::Image *RenderbufferTexture2D::createSharedImage(GLint level)
{
	return mTexture2D->createSharedImage(GL_TEXTURE_2D, level);
}

bool RenderbufferTexture2D::isShared(GLint level) const
{
	return mTexture2D->isShared(GL_TEXTURE_2D, level);
}

GLsizei RenderbufferTexture2D::getWidth(GLint level) const
{
	return mTexture2D->getWidth(GL_TEXTURE_2D, level);
}

GLsizei RenderbufferTexture2D::getHeight(GLint level) const
{
	return mTexture2D->getHeight(GL_TEXTURE_2D, level);
}

GLint RenderbufferTexture2D::getFormat(GLint level) const
{
	return mTexture2D->getFormat(GL_TEXTURE_2D, level);
}

GLsizei RenderbufferTexture2D::getSamples(GLint level) const
{
	return 0;  // ???
}

///// RenderbufferTexture3D Implementation ////////

RenderbufferTexture3D::RenderbufferTexture3D(Texture3D *texture)//, GLint level, GLint layer) : mLevel(level), mLayer(layer)
{
	mTexture3D = texture;
	/*if(mLayer != 0)
	{
		UNIMPLEMENTED();
	}*/
}

RenderbufferTexture3D::~RenderbufferTexture3D()
{
	mTexture3D = NULL;
}

// Textures need to maintain their own reference count for references via
// Renderbuffers acting as proxies. Here, we notify the texture of a reference.
void RenderbufferTexture3D::addProxyRef(const Renderbuffer *proxy)
{
	mTexture3D->addProxyRef(proxy);
}

void RenderbufferTexture3D::releaseProxy(const Renderbuffer *proxy)
{
	mTexture3D->releaseProxy(proxy);
}

// Increments refcount on image.
// caller must release() the returned image
egl::Image *RenderbufferTexture3D::getRenderTarget(GLint level)
{
	return mTexture3D->getRenderTarget(mTexture3D->getTarget(), level);
}

// Increments refcount on image.
// caller must release() the returned image
egl::Image *RenderbufferTexture3D::createSharedImage(GLint level)
{
	return mTexture3D->createSharedImage(mTexture3D->getTarget(), level);
}

bool RenderbufferTexture3D::isShared(GLint level) const
{
	return mTexture3D->isShared(mTexture3D->getTarget(), level);
}

GLsizei RenderbufferTexture3D::getWidth(GLint level) const
{
	return mTexture3D->getWidth(mTexture3D->getTarget(), level);
}

GLsizei RenderbufferTexture3D::getHeight(GLint level) const
{
	return mTexture3D->getHeight(mTexture3D->getTarget(), level);
}

GLsizei RenderbufferTexture3D::getDepth(GLint level) const
{
	return mTexture3D->getDepth(mTexture3D->getTarget(), level);
}

GLint RenderbufferTexture3D::getFormat(GLint level) const
{
	return mTexture3D->getFormat(mTexture3D->getTarget(), level);
}

GLsizei RenderbufferTexture3D::getSamples(GLint level) const
{
	return 0;
}

///// RenderbufferTextureCubeMap Implementation ////////

RenderbufferTextureCubeMap::RenderbufferTextureCubeMap(TextureCubeMap *texture, GLenum target)/*, GLint level)*/ : mTarget(target)//, mLevel(level)
{
	mTextureCubeMap = texture;
}

RenderbufferTextureCubeMap::~RenderbufferTextureCubeMap()
{
	mTextureCubeMap = NULL;
}

// Textures need to maintain their own reference count for references via
// Renderbuffers acting as proxies. Here, we notify the texture of a reference.
void RenderbufferTextureCubeMap::addProxyRef(const Renderbuffer *proxy)
{
	mTextureCubeMap->addProxyRef(proxy);
}

void RenderbufferTextureCubeMap::releaseProxy(const Renderbuffer *proxy)
{
	mTextureCubeMap->releaseProxy(proxy);
}

// Increments refcount on image.
// caller must release() the returned image
egl::Image *RenderbufferTextureCubeMap::getRenderTarget(GLint level)
{
	return mTextureCubeMap->getRenderTarget(mTarget, level);
}

// Increments refcount on image.
// caller must release() the returned image
egl::Image *RenderbufferTextureCubeMap::createSharedImage(GLint level)
{
	return mTextureCubeMap->createSharedImage(mTarget, level);
}

bool RenderbufferTextureCubeMap::isShared(GLint level) const
{
	return mTextureCubeMap->isShared(mTarget, level);
}

GLsizei RenderbufferTextureCubeMap::getWidth(GLint level) const
{
	return mTextureCubeMap->getWidth(mTarget, level);
}

GLsizei RenderbufferTextureCubeMap::getHeight(GLint level) const
{
	return mTextureCubeMap->getHeight(mTarget, level);
}

GLint RenderbufferTextureCubeMap::getFormat(GLint level) const
{
	return mTextureCubeMap->getFormat(mTarget, level);
}

GLsizei RenderbufferTextureCubeMap::getSamples(GLint level) const
{
	return 0;
}

////// Renderbuffer Implementation //////

Renderbuffer::Renderbuffer() : NamedObject(0)
{
	//ASSERT(instance);
	mInstance = nullptr;//instance;
}

Renderbuffer::Renderbuffer(GLuint name, RenderbufferInterface *instance) : NamedObject(name)
{
	ASSERT(instance);
	mInstance = instance;
}

Renderbuffer::~Renderbuffer()
{
	delete mInstance;
}

// The RenderbufferInterface contained in this Renderbuffer may need to maintain
// its own reference count, so we pass it on here.
void Renderbuffer::addRef()
{
	mInstance->addProxyRef(this);

	Object::addRef();
}

void Renderbuffer::release()
{
	mInstance->releaseProxy(this);

	Object::release();
}

// Increments refcount on image.
// caller must Release() the returned image
egl::Image *Renderbuffer::getRenderTarget()
{
	return mInstance->getRenderTarget(mLevel);
}

// Increments refcount on image.
// caller must Release() the returned image
egl::Image *Renderbuffer::createSharedImage()
{
	return mInstance->createSharedImage(mLevel);
}

bool Renderbuffer::isShared() const
{
	return mInstance->isShared(mLevel);
}

GLsizei Renderbuffer::getWidth() const
{
	return mInstance->getWidth(mLevel);
}

GLsizei Renderbuffer::getHeight() const
{
	return mInstance->getHeight(mLevel);
}

GLsizei Renderbuffer::getDepth() const
{
	return mInstance->getDepth(mLevel);
}
/*
GLint Renderbuffer::getLayer() const
{
	return mInstance->getLayer(mLevel);
}

GLint Renderbuffer::getLevel() const
{
	return mInstance->getLevel(mLevel);
}
*/
GLint Renderbuffer::getFormat() const
{
	return mInstance->getFormat(mLevel);
}

GLuint Renderbuffer::getRedSize() const
{
	return mInstance->getRedSize();
}

GLuint Renderbuffer::getGreenSize() const
{
	return mInstance->getGreenSize();
}

GLuint Renderbuffer::getBlueSize() const
{
	return mInstance->getBlueSize();
}

GLuint Renderbuffer::getAlphaSize() const
{
	return mInstance->getAlphaSize();
}

GLuint Renderbuffer::getDepthSize() const
{
	return mInstance->getDepthSize();
}

GLuint Renderbuffer::getStencilSize() const
{
	return mInstance->getStencilSize();
}

GLsizei Renderbuffer::getSamples() const
{
	return mInstance->getSamples(mLevel);
}
/*
void Renderbuffer::setLayer(GLint layer)
{
	return mInstance->setLayer(layer);
}

void Renderbuffer::setLevel(GLint level)
{
	return mInstance->setLevel(level);
}
*/
void Renderbuffer::setStorage(RenderbufferStorage *newStorage)
{
	ASSERT(newStorage != NULL);

	delete mInstance;
	mInstance = newStorage;
}

RenderbufferStorage::RenderbufferStorage()
{
	mWidth = 0;
	mHeight = 0;
	format = GL_NONE;
	mSamples = 0;
}

RenderbufferStorage::~RenderbufferStorage()
{
}

GLsizei RenderbufferStorage::getWidth(GLint level) const
{
	return mWidth;
}

GLsizei RenderbufferStorage::getHeight(GLint level) const
{
	return mHeight;
}

GLint RenderbufferStorage::getFormat(GLint level) const
{
	return format;
}

GLsizei RenderbufferStorage::getSamples(GLint level) const
{
	return mSamples;
}

Colorbuffer::Colorbuffer(egl::Image *renderTarget) : mRenderTarget(renderTarget)
{
	if(renderTarget)
	{
		renderTarget->addRef();

		sw::Format implementationFormat = renderTarget->getInternalFormat();
		format = sw2es::ConvertBackBufferFormat(implementationFormat);

		mWidth = renderTarget->getWidth();
		mHeight = renderTarget->getHeight();
		mSamples = renderTarget->getDepth() & ~1;
	}
}

Colorbuffer::Colorbuffer(int width, int height, GLenum internalformat, GLsizei samples) : mRenderTarget(nullptr)
{
	Device *device = getDevice();

	sw::Format implementationFormat = es2sw::ConvertRenderbufferFormat(internalformat);
	int supportedSamples = Context::getSupportedMultisampleCount(samples);

	if(width > 0 && height > 0)
	{
		mRenderTarget = device->createRenderTarget(width, height, implementationFormat, supportedSamples, false);

		if(!mRenderTarget)
		{
			error(GL_OUT_OF_MEMORY);
			return;
		}
	}

	mWidth = width;
	mHeight = height;
	format = internalformat;
	mSamples = supportedSamples;
}

Colorbuffer::~Colorbuffer()
{
	if(mRenderTarget)
	{
		mRenderTarget->release();
	}
}

// Increments refcount on image.
// caller must release() the returned image
egl::Image *Colorbuffer::getRenderTarget(GLint level)
{
	if(mRenderTarget)
	{
		mRenderTarget->addRef();
	}

	return mRenderTarget;
}

// Increments refcount on image.
// caller must release() the returned image
egl::Image *Colorbuffer::createSharedImage(GLint level)
{
	if(mRenderTarget)
	{
		mRenderTarget->addRef();
		mRenderTarget->markShared();
	}

	return mRenderTarget;
}

bool Colorbuffer::isShared(GLint level) const
{
	return mRenderTarget->isShared();
}

DepthStencilbuffer::DepthStencilbuffer(egl::Image *depthStencil) : mDepthStencil(depthStencil)
{
	if(depthStencil)
	{
		depthStencil->addRef();

		sw::Format implementationFormat = depthStencil->getInternalFormat();
		format = sw2es::ConvertDepthStencilFormat(implementationFormat);

		mWidth = depthStencil->getWidth();
		mHeight = depthStencil->getHeight();
		mSamples = depthStencil->getDepth() & ~1;
	}
}

DepthStencilbuffer::DepthStencilbuffer(int width, int height, GLenum internalformat, GLsizei samples) : mDepthStencil(nullptr)
{
	format = internalformat;
	sw::Format implementationFormat = sw::FORMAT_D24S8;
	switch(internalformat)
	{
	case GL_STENCIL_INDEX8:
	case GL_DEPTH_COMPONENT24:
	case GL_DEPTH24_STENCIL8_OES:
		implementationFormat = sw::FORMAT_D24S8;
		break;
	case GL_DEPTH32F_STENCIL8:
		implementationFormat = sw::FORMAT_D32FS8_TEXTURE;
		break;
	case GL_DEPTH_COMPONENT16:
		implementationFormat = sw::FORMAT_D16;
		break;
	case GL_DEPTH_COMPONENT32_OES:
		implementationFormat = sw::FORMAT_D32;
		break;
	case GL_DEPTH_COMPONENT32F:
		implementationFormat = sw::FORMAT_D32F;
		break;
	default:
		UNREACHABLE(internalformat);
		format = GL_DEPTH24_STENCIL8_OES;
		implementationFormat = sw::FORMAT_D24S8;
	}

	Device *device = getDevice();

	int supportedSamples = Context::getSupportedMultisampleCount(samples);

	if(width > 0 && height > 0)
	{
		mDepthStencil = device->createDepthStencilSurface(width, height, implementationFormat, supportedSamples, false);

		if(!mDepthStencil)
		{
			error(GL_OUT_OF_MEMORY);
			return;
		}
	}

	mWidth = width;
	mHeight = height;
	mSamples = supportedSamples;
}

DepthStencilbuffer::~DepthStencilbuffer()
{
	if(mDepthStencil)
	{
		mDepthStencil->release();
	}
}

// Increments refcount on image.
// caller must release() the returned image
egl::Image *DepthStencilbuffer::getRenderTarget(GLint level)
{
	if(mDepthStencil)
	{
		mDepthStencil->addRef();
	}

	return mDepthStencil;
}

// Increments refcount on image.
// caller must release() the returned image
egl::Image *DepthStencilbuffer::createSharedImage(GLint level)
{
	if(mDepthStencil)
	{
		mDepthStencil->addRef();
		mDepthStencil->markShared();
	}

	return mDepthStencil;
}

bool DepthStencilbuffer::isShared(GLint level) const
{
	return mDepthStencil->isShared();
}

Depthbuffer::Depthbuffer(egl::Image *depthStencil) : DepthStencilbuffer(depthStencil)
{
}

Depthbuffer::Depthbuffer(int width, int height, GLenum internalformat, GLsizei samples) : DepthStencilbuffer(width, height, internalformat, samples)
{
}

Depthbuffer::~Depthbuffer()
{
}

Stencilbuffer::Stencilbuffer(egl::Image *depthStencil) : DepthStencilbuffer(depthStencil)
{
}

Stencilbuffer::Stencilbuffer(int width, int height, GLsizei samples) : DepthStencilbuffer(width, height, GL_STENCIL_INDEX8, samples)
{
}

Stencilbuffer::~Stencilbuffer()
{
}

}
