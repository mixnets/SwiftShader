Headless Rendering
==================

SwiftShader supports rendering on Linux systems without any display output or windowing system (X11), through the [EGL_MESA_platform_gbm](https://www.khronos.org/registry/egl/extensions/MESA/EGL_MESA_platform_gbm.txt) extension. Example code is provided in the extension text. Calling ```eglGetPlatformDisplayEXT(EGL_PLATFORM_GBM_MESA, EGL_DEFAULT_DISPLAY)``` will give you a headless display. On Linux when libX11 is not available, ```eglGetDisplay(EGL_DEFAULT_DISPLAY)``` behaves identically, but note that this is implementation-specific behavior.

Aside from creating a windows surface with ```eglCreatePlatformWindowSurfaceEXT()```, you can also create a pbuffer surface using ```eglCreatePbufferSurface()```. In either case the rendered image can be read back with ```glReadPixels()```.

Note that ```glReadPixels()``` causes OpenGL to wait until all rendering to the surface is done. To avoid the cost of this synchronization and keep the processor(s) busy, one can implement 'double buffering' by alternating the rendering between two pbuffer surfaces.

```glReadPixels()``` also adds some minor overhead from having to copy the pixel data to an application-provided buffer. This could be avoided by having SwiftShader rendering directly into an application-provided buffer through ```eglCreatePbufferFromClientBuffer()``` or ```eglCreatePixmapSurface[EXT]()```. As yet these functions are unimplemented, and they would require an extension spec.