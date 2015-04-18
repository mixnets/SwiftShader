#ifndef ANDROID_COMMON
#define ANDROID_COMMON

// Used internally
static inline GLenum getColorFormatFromAndroid(int format)
{
    switch(format)
    {
        case HAL_PIXEL_FORMAT_RGBA_8888:
            return GL_RGBA;
        case HAL_PIXEL_FORMAT_RGBX_8888:
            return GL_RGB;
        case HAL_PIXEL_FORMAT_RGB_888:
            return GL_RGB;
        case HAL_PIXEL_FORMAT_BGRA_8888:
            return GL_BGRA_EXT;
        case HAL_PIXEL_FORMAT_RGB_565:
            return GL_RGB565_OES;
        case HAL_PIXEL_FORMAT_YV12:
        case HAL_PIXEL_FORMAT_Y8:
        case HAL_PIXEL_FORMAT_Y16:
        case HAL_PIXEL_FORMAT_RAW_SENSOR:
        case HAL_PIXEL_FORMAT_BLOB:
        case HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED:
        case HAL_PIXEL_FORMAT_YCbCr_420_888:
        default:
            UNIMPLEMENTED();
    }
    return GL_RGBA;
}

// Used internally
static inline GLenum getPixelFormatFromAndroid(int format)
{
    switch(format)
    {
        case HAL_PIXEL_FORMAT_RGBA_8888:
        case HAL_PIXEL_FORMAT_RGBX_8888:
        case HAL_PIXEL_FORMAT_RGB_888:
        case HAL_PIXEL_FORMAT_BGRA_8888:
            return GL_UNSIGNED_BYTE;
        case HAL_PIXEL_FORMAT_RGB_565:
            return GL_UNSIGNED_SHORT_5_6_5;
        case HAL_PIXEL_FORMAT_YV12:
        case HAL_PIXEL_FORMAT_Y8:
        case HAL_PIXEL_FORMAT_Y16:
        case HAL_PIXEL_FORMAT_RAW_SENSOR:
        case HAL_PIXEL_FORMAT_BLOB:
        case HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED:
        case HAL_PIXEL_FORMAT_YCbCr_420_888:
        default:
            UNIMPLEMENTED();
    }
    return GL_UNSIGNED_BYTE;
}

// Used in V1 & V2 Context.cpp
static inline EGLenum isSupportedAndroidBuffer(GLuint name)
{
    ANativeWindowBuffer *nativeBuffer = reinterpret_cast<ANativeWindowBuffer*>(name);

    if(!name)
    {
        ALOGE("%s called with name==NULL %s:%d", __FUNCTION__, __FILE__, __LINE__);
        return EGL_BAD_PARAMETER;
    }
    if(nativeBuffer->common.magic != ANDROID_NATIVE_BUFFER_MAGIC)
    {
        ALOGE("%s: failed: bad magic", __FUNCTION__);
        return EGL_BAD_PARAMETER;
    }

    if(nativeBuffer->common.version != sizeof(ANativeWindowBuffer))
    {
        ALOGE("%s: failed: bad size", __FUNCTION__ );
        return EGL_BAD_PARAMETER;
    }

    switch(nativeBuffer->format)
    {
        case HAL_PIXEL_FORMAT_RGBA_8888:
        case HAL_PIXEL_FORMAT_RGBX_8888:
        case HAL_PIXEL_FORMAT_RGB_565:
            return EGL_SUCCESS;
        default:
            ALOGE("%s: failed: bad format", __FUNCTION__ );
            return EGL_BAD_PARAMETER;
    }
}

// Used in V1 & V2 Context.cpp
template <typename I> I* wrapAndroidNativeWindow(GLuint name)
{
    ANativeWindowBuffer *nativeBuffer = reinterpret_cast<ANativeWindowBuffer*>(name);
    ALOGV("%s: wrapping %p", __FUNCTION__, nativeBuffer);
    nativeBuffer->common.incRef(&nativeBuffer->common);

    GLenum format = getColorFormatFromAndroid(nativeBuffer->format);
    GLenum type = getPixelFormatFromAndroid(nativeBuffer->format);

    I *image = new I(0, nativeBuffer->width, nativeBuffer->height, format, type);
    image->setNativeBuffer(nativeBuffer);
    image->markShared();

    return image;
}

#endif  // ANDROID_COMMON
