// Copyright 2019 The SwiftShader Authors. All Rights Reserved.
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

#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include <stdint.h>
#include <stddef.h>
#include <cstring> // TODO REMOVE

namespace
{

EGLDisplay display;
EGLConfig config;
EGLSurface surface;
EGLContext context;

bool initialize(int version) {
    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    EGLint major;
    EGLint minor;
    EGLBoolean initialized = eglInitialize(display, &major, &minor);
    if (!initialized) { return false; }

    const EGLint configAttributes[] =
    {
        EGL_SURFACE_TYPE,		EGL_PBUFFER_BIT,
        EGL_RENDERABLE_TYPE,	EGL_OPENGL_ES2_BIT,
        EGL_ALPHA_SIZE,			8,
        EGL_NONE
    };

    EGLint num_config = -1;
    EGLBoolean success = eglChooseConfig(display, configAttributes, &config, 1, &num_config);
    if (success != EGL_TRUE) { return false; }

    EGLint surfaceAttributes[] =
    {
        EGL_WIDTH, 8,
        EGL_HEIGHT, 8,
        EGL_NONE
    };

    auto surface = eglCreatePbufferSurface(display, config, surfaceAttributes);

    EGLint contextAttributes[] =
    {
        EGL_CONTEXT_CLIENT_VERSION, version,
        EGL_NONE
    };

    context = eglCreateContext(display, config, NULL, contextAttributes);
    success = eglMakeCurrent(display, surface, surface, context);
    if (success != EGL_TRUE) { return false; }
    return true;
}

void shutdown()
{
    eglDestroyContext(display, context);
    eglDestroySurface(display, surface);
    eglTerminate(display);
}

}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    initialize(2);

    auto program = glCreateProgram();

    auto vs = glCreateShader(GL_VERTEX_SHADER);
    const char* vs_source =
        "#version 300 es\n"
        "in vec4 position;\n"
        "void main()\n"
        "{\n"
        "	gl_Position = vec4(position.xy, 0.0, 1.0);\n"
        "}\n";
    glShaderSource(vs, 1, &vs_source, nullptr);
    glCompileShader(vs);
    glAttachShader(program, vs);

    auto fs = glCreateShader(GL_FRAGMENT_SHADER);
    auto fs_source = reinterpret_cast<const GLchar*>(data);
    auto length = static_cast<GLint>(size);
    glShaderSource(fs, 1, &fs_source, &length);
    glCompileShader(fs);
    glAttachShader(program, fs);

    glLinkProgram(program);

    GLint status = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == GL_TRUE) {
        glUseProgram(program);

        GLint posLoc = glGetAttribLocation(program, "position");

        float vertices[18] = { -1.0f,  1.0f, 0.5f,
                                -1.0f, -1.0f, 0.5f,
                                1.0f, -1.0f, 0.5f,
                                -1.0f,  1.0f, 0.5f,
                                1.0f, -1.0f, 0.5f,
                                1.0f,  1.0f, 0.5f };

        glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, vertices);
        glEnableVertexAttribArray(posLoc);
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }

    shutdown();

	return 0;
}
