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

// ResourceManager.h : Defines the ResourceManager class, which tracks objects
// shared by multiple GL contexts.

#ifndef LIBGL_RESOURCEMANAGER_H_
#define LIBGL_RESOURCEMANAGER_H_

#include "common/NameSpace.hpp"

#define _GDI32_
#include <windows.h>
#include <GL/GL.h>
#define GL_GLEXT_PROTOTYPES
#include <GL/glext.h>

namespace gl
{
class Buffer;
class Shader;
class Program;
class Texture;
class Renderbuffer;

enum TextureType
{
    TEXTURE_2D,
    PROXY_TEXTURE_2D,
    TEXTURE_CUBE,

    TEXTURE_TYPE_COUNT,
    TEXTURE_UNKNOWN
};

class ResourceManager
{
  public:
    ResourceManager();
    ~ResourceManager();

    void addRef();
    void release();

    GLuint createBuffer();
    GLuint createShader(GLenum type);
    GLuint createProgram();
    GLuint createTexture();
    GLuint createRenderbuffer();

    void deleteBuffer(GLuint buffer);
    void deleteShader(GLuint shader);
    void deleteProgram(GLuint program);
    void deleteTexture(GLuint texture);
    void deleteRenderbuffer(GLuint renderbuffer);

    Buffer *getBuffer(GLuint handle);
    Shader *getShader(GLuint handle);
    Program *getProgram(GLuint handle);
    Texture *getTexture(GLuint handle);
    Renderbuffer *getRenderbuffer(GLuint handle);
    
    void setRenderbuffer(GLuint handle, Renderbuffer *renderbuffer);

    void checkBufferAllocation(unsigned int buffer);
    void checkTextureAllocation(GLuint texture, TextureType type);
    void checkRenderbufferAllocation(GLuint renderbuffer);

  private:
    std::size_t mRefCount;

    typedef NameSpace<Buffer> BufferMap;
    BufferMap mBufferMap;
    
    typedef NameSpace<Shader> ShaderMap;
    ShaderMap mShaderMap;

    typedef NameSpace<Program> ProgramMap;
    ProgramMap mProgramMap;
    
    typedef NameSpace<Texture> TextureMap;
    TextureMap mTextureMap;
    
    typedef NameSpace<Renderbuffer> RenderbufferMap;
    RenderbufferMap mRenderbufferMap;
};

}

#endif // LIBGL_RESOURCEMANAGER_H_
