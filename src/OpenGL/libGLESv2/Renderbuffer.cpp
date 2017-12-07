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

GLuint RenderbufferInterface::getRedSize() const
{
	return GetRedSize(getFormat());
}

GLuint RenderbufferInterface::getGreenSize() const
{
	return GetGreenSize(getFormat());
}

GLuint RenderbufferInterface::getBlueSize() const
{
	return GetBlueSize(getFormat());
}

GLuint RenderbufferInterface::getAlphaSize() const
{
	return GetAlphaSize(getFormat());
}

GLuint RenderbufferInterface::getDepthSize() const
{
	return GetDepthSize(getFormat());
}

GLuint RenderbufferInterface::getStencilSize() const
{
	return GetStencilSize(getFormat());
}

///// RenderbufferTexture Implementation ////////

RenderbufferTexture::RenderbufferTexture(Texture2D *texture, GLint level) : mLevel(level), mLayer(0)
{
	mImage = texture->getImage(level);
}

RenderbufferTexture::RenderbufferTexture(Texture3D *texture, GLint level, GLint layer) : mLevel(level), mLayer(layer)
{
	mImage = texture->getImage(level);
	if(mLayer != 0)
	{
	    	UNIMPLEMENTED();
	}
}

RenderbufferTexture::RenderbufferTexture(TextureCubeMap *texture, int face, GLint level) : mLevel(level), mLayer(0)
{
	mImage = texture->getImage(face, level);
}

RenderbufferTexture::~RenderbufferTexture()
{
	mImage = NULL;
}

// Increments refcount on image.
// caller must release() the returned image
egl::Image *RenderbufferTexture::getRenderTarget()
{
	mImage->addRef();
	return mImage;
}

// Increments refcount on image.
// caller must release() the returned image
egl::Image *RenderbufferTexture::createSharedImage()
{
	mImage->addRef();
	mImage->markShared();

	return mImage;
}

bool RenderbufferTexture::isShared() const
{
	return mImage->isShared();
}

GLsizei RenderbufferTexture::getWidth() const
{
	return mImage->getWidth();
}

GLsizei RenderbufferTexture::getHeight() const
{
	return mImage->getHeight();
}

GLsizei RenderbufferTexture::getDepth() const
{
	return mImage->getDepth();
}

GLint RenderbufferTexture::getFormat() const
{
	return mImage->getFormat();
}

GLsizei RenderbufferTexture::getSamples() const
{
	return 0;
}

////// Renderbuffer Implementation //////

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
	//mInstance->getRenderTarget()->addRef();

	Object::addRef();
}

void Renderbuffer::release()
{
//	mInstance->getRenderTarget()->release();

	Object::release();
}

// Increments refcount on image.
// caller must Release() the returned image
egl::Image *Renderbuffer::getRenderTarget()
{
	return mInstance->getRenderTarget();
}

// Increments refcount on image.
// caller must Release() the returned image
egl::Image *Renderbuffer::createSharedImage()
{
	return mInstance->createSharedImage();
}

bool Renderbuffer::isShared() const
{
	return mInstance->isShared();
}

GLsizei Renderbuffer::getWidth() const
{
	return mInstance->getWidth();
}

GLsizei Renderbuffer::getHeight() const
{
	return mInstance->getHeight();
}

GLsizei Renderbuffer::getDepth() const
{
	return mInstance->getDepth();
}

GLint Renderbuffer::getLayer() const
{
	return mInstance->getLayer();
}

GLint Renderbuffer::getLevel() const
{
	return mInstance->getLevel();
}

GLint Renderbuffer::getFormat() const
{
	return mInstance->getFormat();
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
	return mInstance->getSamples();
}

void Renderbuffer::setLayer(GLint layer)
{
	return mInstance->setLayer(layer);
}

void Renderbuffer::setLevel(GLint level)
{
	return mInstance->setLevel(level);
}

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

GLsizei RenderbufferStorage::getWidth() const
{
	return mWidth;
}

GLsizei RenderbufferStorage::getHeight() const
{
	return mHeight;
}

GLint RenderbufferStorage::getFormat() const
{
	return format;
}

GLsizei RenderbufferStorage::getSamples() const
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
egl::Image *Colorbuffer::getRenderTarget()
{
	if(mRenderTarget)
	{
		mRenderTarget->addRef();
	}

	return mRenderTarget;
}

// Increments refcount on image.
// caller must release() the returned image
egl::Image *Colorbuffer::createSharedImage()
{
	if(mRenderTarget)
	{
		mRenderTarget->addRef();
		mRenderTarget->markShared();
	}

	return mRenderTarget;
}

bool Colorbuffer::isShared() const
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
egl::Image *DepthStencilbuffer::getRenderTarget()
{
	if(mDepthStencil)
	{
		mDepthStencil->addRef();
	}

	return mDepthStencil;
}

// Increments refcount on image.
// caller must release() the returned image
egl::Image *DepthStencilbuffer::createSharedImage()
{
	if(mDepthStencil)
	{
		mDepthStencil->addRef();
		mDepthStencil->markShared();
	}

	return mDepthStencil;
}

bool DepthStencilbuffer::isShared() const
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
