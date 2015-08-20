#ifndef ANDROID_COMMON
#define ANDROID_COMMON

namespace egl
{
class Image;
}

// Used internally
GLenum getColorFormatFromAndroid(int format);

// Used internally
GLenum getPixelFormatFromAndroid(int format);

#endif  // ANDROID_COMMON
