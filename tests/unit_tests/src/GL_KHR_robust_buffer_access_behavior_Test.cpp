#include "BaseGLES2Test.h"

#include <algorithm>
#include <string>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

class GL_KHR_robust_buffer_access_behavior_Test : public BaseGLES2Test {

};

TEST_F(GL_KHR_robust_buffer_access_behavior_Test, ExtensionAvailable)
{
  const GLubyte* extensions = glGetString(GL_EXTENSIONS);
  std::string extensionsString((const char*)extensions);
  ASSERT_NE(
    std::string::npos,
    extensionsString.find("GL_KHR_robust_buffer_access_behavior"));
}

TEST_F(GL_KHR_robust_buffer_access_behavior_Test, glDrawArraysClampsBufferReads)
{
  GLfloat vertexData[] = { 1.f,  1.f, 0.f,
                           0.f,  1.f, 0.f,

                          -1.f,  1.f, 0.f,
                           0.f,  1.f, 0.f,

                          -1.f, -1.f, 0.f,
                           0.f,  1.f, 0.f,

                          -1.f, -1.f, 0.f,
                           0.f,  1.f, 0.f,

                           1.f, -1.f, 0.f,
                           0.f,  1.f, 0.f,

                           1.f,  1.f, 0.f,
                           0.f,  1.f, 0.f,};
  GLuint vertexBuffer;
  glGenBuffers(1, &vertexBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

  ASSERT_EQ(GL_NO_ERROR, glGetError());

  const char vertexShaderSource[] =
      "attribute vec4 vertex;"
      "attribute vec4 color;"
      "varying highp vec4 fragColor;"
      "void main() {"
        "gl_Position = vertex;"
        "fragColor = color;"
      "}";
  const char fragmentShaderSource[] =
      "varying highp vec4 fragColor;"
      "void main() {"
        "gl_FragColor = fragColor;"
      "}";

  GLuint vertexShader = MakeShader(GL_VERTEX_SHADER, vertexShaderSource);
  GLuint fragmentShader = MakeShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

  GLuint program = glCreateProgram();
  glAttachShader(program, vertexShader);
  glAttachShader(program, fragmentShader);

  glLinkProgram(program);
  GLint linked;
  glGetProgramiv(program, GL_LINK_STATUS, &linked);
  ASSERT_TRUE(linked);

  glUseProgram(program);
  ASSERT_EQ(GL_NO_ERROR, glGetError());

  GLuint vertexAttrib = glGetAttribLocation(program, "vertex");
  ASSERT_EQ(GL_NO_ERROR, glGetError());

  glEnableVertexAttribArray(vertexAttrib);
  glVertexAttribPointer(vertexAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 6, 0);

  GLuint colorAttrib = glGetAttribLocation(program, "color");
  ASSERT_EQ(GL_NO_ERROR, glGetError());

  glEnableVertexAttribArray(colorAttrib);
  glVertexAttribPointer(colorAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 6, (void*)(sizeof(GLfloat) * 3));

  // Correctly draw 2 triangles (6 vertices) from the vertex buffer
  glDrawArrays(GL_TRIANGLES, 0, 6);
  ASSERT_EQ(GL_NO_ERROR, glGetError());
  ASSERT_TRUE(eglSwapBuffers(mNativeDisplayImpl->mEGLDisplay, mNativeDisplayImpl->mEGLSurface));

  // Read back the pixels and validate that they're all green. We're drawing a green square the size of the
  // window so that's expected.
  GLubyte* pixels = new GLubyte[4 * 800 * 800];
  glReadPixels(0, 0, 800, 800, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
  ASSERT_EQ(GL_NO_ERROR, glGetError());

  for (size_t i = 0; i < 800 * 800; i += 4)
  {
    ASSERT_EQ(0, pixels[i]);
    ASSERT_EQ(255, pixels[i + 1]);
    ASSERT_EQ(0, pixels[i + 2]);
    ASSERT_EQ(255, pixels[i + 3]);
  }

  // Draw way too many triangles vs what's in the vertex buffer. To comply with the extension spec, this should
  // not crash and return values from inside the buffer (or default values).
  glDrawArrays(GL_TRIANGLES, 0, 65530);
  ASSERT_EQ(GL_NO_ERROR, glGetError());
  ASSERT_TRUE(eglSwapBuffers(mNativeDisplayImpl->mEGLDisplay, mNativeDisplayImpl->mEGLSurface));

  // Verify that we're correctly clamping the reads in the vertex buffer (since we know that's what our implementation
  // is doing). Basically, all pixels should still be green since the only valid value of the color attribute given
  // our vertex buffer is pure green.
  glReadPixels(0, 0, 800, 800, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
  ASSERT_EQ(GL_NO_ERROR, glGetError());

  for (size_t i = 0; i < 800 * 800; i += 4)
  {
    ASSERT_EQ(0, pixels[i]);
    ASSERT_EQ(255, pixels[i + 1]);
    ASSERT_EQ(0, pixels[i + 2]);
    ASSERT_EQ(255, pixels[i + 3]);
  }

  delete[] pixels;
}
