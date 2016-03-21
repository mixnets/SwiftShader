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

// Renderbuffer.h: Defines the wrapper class Renderbuffer, as well as the
// class hierarchy used to store its contents: RenderbufferStorage, Colorbuffer,
// DepthStencilbuffer, Depthbuffer and Stencilbuffer. Implements GL renderbuffer
// objects and related functionality. [OpenGL ES 2.0.24] section 4.4.3 page 108.

#ifndef LIBGLESV2_RENDERBUFFER_H_
#define LIBGLESV2_RENDERBUFFER_H_

#include "common/Object.hpp"
#include "common/Image.hpp"

#include <GLES2/gl2.h>

namespace es2
{
class Texture2D;
class Texture3D;
class TextureCubeMap;
class Renderbuffer;
class Colorbuffer;
class DepthStencilbuffer;

class RenderbufferInterface
{
public:
	RenderbufferInterface();

	virtual ~RenderbufferInterface() {};

	virtual void addProxyRef(const Renderbuffer *proxy);
    virtual void releaseProxy(const Renderbuffer *proxy);

	virtual egl::Image *getRenderTarget() = 0;   // Color or depth
	virtual egl::Image *getStencilBuffer() { return nullptr; }
    virtual egl::Image *createSharedImage() = 0;
    virtual bool isShared() const = 0;

	virtual GLsizei getWidth() const = 0;
	virtual GLsizei getHeight() const = 0;
	virtual GLsizei getDepth() const { return 1; }
	virtual GLint getLayer() const { return 0; }
	virtual GLint getLevel() const { return 0; }
	virtual GLenum getFormat() const = 0;
	virtual sw::Format getInternalFormat() const = 0;
	virtual GLsizei getSamples() const = 0;

	virtual void setLayer(GLint) {}
	virtual void setLevel(GLint) {}

	GLuint getRedSize() const;
	GLuint getGreenSize() const;
	GLuint getBlueSize() const;
	GLuint getAlphaSize() const;
	GLuint getDepthSize() const;
	GLuint getStencilSize() const;
};

class RenderbufferTexture2D : public RenderbufferInterface
{
public:
	RenderbufferTexture2D(Texture2D *texture, GLint level);

	virtual ~RenderbufferTexture2D();

	virtual void addProxyRef(const Renderbuffer *proxy);
    virtual void releaseProxy(const Renderbuffer *proxy);

	egl::Image *getRenderTarget() override;
    egl::Image *createSharedImage() override;
    bool isShared() const override;

	GLsizei getWidth() const override;
	GLsizei getHeight() const override;
	GLint getLevel() const override { return mLevel; }
	GLenum getFormat() const override;
	sw::Format getInternalFormat() const override;
	GLsizei getSamples() const override;

	void setLevel(GLint level) override { mLevel = level; }

private:
	gl::BindingPointer<Texture2D> mTexture2D;
	GLint mLevel;
};

class RenderbufferTexture3D : public RenderbufferInterface
{
public:
	RenderbufferTexture3D(Texture3D *texture, GLint level, GLint layer);

	virtual ~RenderbufferTexture3D();

	void addProxyRef(const Renderbuffer *proxy) override;
	void releaseProxy(const Renderbuffer *proxy) override;

	virtual egl::Image *getRenderTarget() override;
	egl::Image *createSharedImage() override;
	bool isShared() const;

	GLsizei getWidth() const override;
	GLsizei getHeight() const override;
	GLsizei getDepth() const override;
	GLint getLayer() const override { return mLayer; }
	GLint getLevel() const override { return mLevel; }
	GLenum getFormat() const override;
	sw::Format getInternalFormat() const override;
	GLsizei getSamples() const override;

	void setLayer(GLint layer) override { mLayer = layer; }
	void setLevel(GLint level) override { mLevel = level; }

private:
	gl::BindingPointer<Texture3D> mTexture3D;
	GLint mLevel;
	GLint mLayer;
};

class RenderbufferTextureCubeMap : public RenderbufferInterface
{
public:
	RenderbufferTextureCubeMap(TextureCubeMap *texture, GLenum target, GLint level);

	virtual ~RenderbufferTextureCubeMap();

	void addProxyRef(const Renderbuffer *proxy) override;
    void releaseProxy(const Renderbuffer *proxy) override;

	egl::Image *getRenderTarget() override;
    egl::Image *createSharedImage() override;
    bool isShared() const override;

	GLsizei getWidth() const override;
	GLsizei getHeight() const override;
	GLint getLevel() const override { return mLevel; }
	GLenum getFormat() const override;
	sw::Format getInternalFormat() const override;
	GLsizei getSamples() const override;

	void setLevel(GLint level) override { mLevel = level; }

private:
	gl::BindingPointer<TextureCubeMap> mTextureCubeMap;
	GLenum mTarget;
	GLint mLevel;
};

// A class derived from RenderbufferStorage is created whenever glRenderbufferStorage
// is called. The specific concrete type depends on whether the internal format is
// colour depth, stencil or packed depth/stencil.
class RenderbufferStorage : public RenderbufferInterface
{
public:
	RenderbufferStorage();

	virtual ~RenderbufferStorage() = 0;

	egl::Image *getRenderTarget() override = 0;
	egl::Image *getStencilBuffer() override { return nullptr; }
    egl::Image *createSharedImage() override = 0;
    bool isShared() const override = 0;

	GLsizei getWidth() const override;
	GLsizei getHeight() const override;
	GLenum getFormat() const override;
	sw::Format getInternalFormat() const override;
	GLsizei getSamples() const override;

protected:
	GLsizei mWidth;
	GLsizei mHeight;
	GLenum format;
	sw::Format internalFormat;
	GLsizei mSamples;
};

// Renderbuffer implements the GL renderbuffer object.
// It's only a proxy for a RenderbufferInterface instance; the internal object
// can change whenever glRenderbufferStorage is called.
class Renderbuffer : public gl::NamedObject
{
public:
	Renderbuffer(GLuint name, RenderbufferInterface *storage);

	virtual ~Renderbuffer();

	// These functions from Object are overloaded here because
    // Textures need to maintain their own count of references to them via
    // Renderbuffers/RenderbufferTextures. These functions invoke those
    // reference counting functions on the RenderbufferInterface.
    void addRef() override;
    void release() override;

	egl::Image *getRenderTarget();
	egl::Image *getStencilBuffer();
    virtual egl::Image *createSharedImage();
    virtual bool isShared() const;

	GLsizei getWidth() const;
	GLsizei getHeight() const;
	GLsizei getDepth() const;
	GLint getLayer() const;
	GLint getLevel() const;
	GLenum getFormat() const;
	sw::Format getInternalFormat() const;
	GLuint getRedSize() const;
	GLuint getGreenSize() const;
	GLuint getBlueSize() const;
	GLuint getAlphaSize() const;
	GLuint getDepthSize() const;
	GLuint getStencilSize() const;
	GLsizei getSamples() const;

	void setLayer(GLint layer);
	void setLevel(GLint level);
	void setStorage(RenderbufferStorage *newStorage);

private:
	RenderbufferInterface *mInstance;
};

class Colorbuffer : public RenderbufferStorage
{
public:
	explicit Colorbuffer(egl::Image *renderTarget);
	Colorbuffer(GLsizei width, GLsizei height, GLenum format, GLsizei samples);

	virtual ~Colorbuffer();

	egl::Image *getRenderTarget() override;
    egl::Image *createSharedImage() override;
    bool isShared() const override;

private:
	egl::Image *mRenderTarget;
};

class DepthStencilbuffer : public RenderbufferStorage
{
public:
	explicit DepthStencilbuffer(egl::Image *depthStencil);
	DepthStencilbuffer(GLsizei width, GLsizei height, GLsizei samples);

	~DepthStencilbuffer();

	egl::Image *getRenderTarget() override;
	egl::Image *getStencilBuffer() override;
    egl::Image *createSharedImage() override;
    bool isShared() const override;

protected:
	egl::Image *mDepthStencil;
};

class Depthbuffer : public DepthStencilbuffer
{
public:
	explicit Depthbuffer(egl::Image *depthStencil);
	Depthbuffer(GLsizei width, GLsizei height, GLsizei samples);

	virtual ~Depthbuffer();

	egl::Image *getStencilBuffer() override { return nullptr; }
};

class Stencilbuffer : public DepthStencilbuffer
{
public:
	explicit Stencilbuffer(egl::Image *depthStencil);
	Stencilbuffer(GLsizei width, GLsizei height, GLsizei samples);

	virtual ~Stencilbuffer();

	egl::Image *getRenderTarget() override { return nullptr; }
};
}

#endif   // LIBGLESV2_RENDERBUFFER_H_
