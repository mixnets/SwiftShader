#include "BaseGLES2Test.h"

#ifdef __linux__

#include "X11/Xlib.h"
#include "X11/Xutil.h"

struct NativeDisplayImpl : public NativeDisplayBase {
    Display* mDisplay;
};

void Init(NativeDisplayImpl* nativeDisplayImpl)
{
  nativeDisplayImpl->mDisplay = XOpenDisplay(0);
  ASSERT_TRUE(nativeDisplayImpl->mDisplay);

  int defaultScreen = XDefaultScreen(nativeDisplayImpl->mDisplay);
  int defaultDepth = DefaultDepth(nativeDisplayImpl->mDisplay, defaultScreen);
  XVisualInfo visualInfo;
  ASSERT_NE(0, XMatchVisualInfo(nativeDisplayImpl->mDisplay, defaultScreen, defaultDepth, TrueColor, &visualInfo));

  Window rootWindow = RootWindow(nativeDisplayImpl->mDisplay, defaultScreen);
	XSetWindowAttributes windowAttributes;
	windowAttributes.colormap = XCreateColormap(nativeDisplayImpl->mDisplay, rootWindow, visualInfo.visual, AllocNone);
	windowAttributes.event_mask = StructureNotifyMask | ExposureMask | ButtonPressMask;
	Window nativeWindow = XCreateWindow(nativeDisplayImpl->mDisplay, rootWindow,
						  		  0, 0, 800, 800, 0,
	                              visualInfo.depth,
								  InputOutput,
								  visualInfo.visual,
								  CWEventMask | CWColormap,
								  &windowAttributes);

	XMapWindow(nativeDisplayImpl->mDisplay, nativeWindow);
	XStoreName(nativeDisplayImpl->mDisplay, nativeWindow, "SwiftShader unit tests");

	Atom windowManagerDelete = XInternAtom(nativeDisplayImpl->mDisplay, "WM_DELETE_WINDOW", True);
	XSetWMProtocols(nativeDisplayImpl->mDisplay, nativeWindow, &windowManagerDelete , 1);

  nativeDisplayImpl->mEGLDisplay = eglGetDisplay((EGLNativeDisplayType)nativeDisplayImpl->mDisplay);
  ASSERT_NE(EGL_NO_DISPLAY, nativeDisplayImpl->mEGLDisplay);

  EGLint major, minor;
  ASSERT_TRUE(eglInitialize(nativeDisplayImpl->mEGLDisplay, &major, &minor));

  const EGLint configurationAttributes[] =
	{
		EGL_SURFACE_TYPE,		EGL_WINDOW_BIT,
		EGL_RENDERABLE_TYPE,	EGL_OPENGL_ES2_BIT,
		EGL_NONE
	};

	EGLint configsReturned;
	EGLConfig eglConfig;
	ASSERT_TRUE(eglChooseConfig(nativeDisplayImpl->mEGLDisplay, configurationAttributes, &eglConfig, 1, &configsReturned));
	ASSERT_EQ(1, configsReturned);

	nativeDisplayImpl->mEGLSurface = eglCreateWindowSurface(nativeDisplayImpl->mEGLDisplay, eglConfig, (EGLNativeWindowType)nativeWindow, NULL);
	ASSERT_EQ(EGL_SUCCESS, eglGetError());

  EGLint contextAttributes[] =
	{
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};

	EGLContext eglContext = eglCreateContext(nativeDisplayImpl->mEGLDisplay, eglConfig, NULL, contextAttributes);
	ASSERT_EQ(EGL_SUCCESS, eglGetError());

	eglBindAPI(EGL_OPENGL_ES_API);
	ASSERT_EQ(EGL_SUCCESS, eglGetError());

	eglMakeCurrent(nativeDisplayImpl->mEGLDisplay, nativeDisplayImpl->mEGLSurface, nativeDisplayImpl->mEGLSurface, eglContext);
	ASSERT_EQ(EGL_SUCCESS, eglGetError());
}

#endif // __linux__

BaseGLES2Test::BaseGLES2Test()
{
  mNativeDisplayImpl = new NativeDisplayImpl;

  Init(static_cast<NativeDisplayImpl*>(mNativeDisplayImpl));

  glClearColor(0.f, 0.f, 0.f, 1.f);
  glClear(GL_COLOR_BUFFER_BIT);
}

BaseGLES2Test::~BaseGLES2Test()
{
  delete mNativeDisplayImpl;
}

unsigned int BaseGLES2Test::MakeShader(GLenum type, const char* source)
{
  GLuint handle = glCreateShader(type);

  glShaderSource(handle, 1, &source, NULL);
  glCompileShader(handle);

  GLint compiled;
  glGetShaderiv(handle, GL_COMPILE_STATUS, &compiled);

  EXPECT_TRUE(compiled);

  return handle;
}
