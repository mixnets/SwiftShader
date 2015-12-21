#ifndef BASEGLES2TEST_H
#define BASEGLES2TEST_H

#include <gtest/gtest.h>

#include <EGL/egl.h>
#include <GLES2/gl2.h>

struct NativeDisplayBase {
    EGLDisplay mEGLDisplay;
    EGLSurface mEGLSurface;
};

class BaseGLES2Test : public ::testing::Test
{
public:
    BaseGLES2Test();
    virtual ~BaseGLES2Test();

    unsigned int MakeShader(GLenum type, const char* source);

protected:
    NativeDisplayBase* mNativeDisplayImpl;
};

#endif // BASEGLES2TEST_H
