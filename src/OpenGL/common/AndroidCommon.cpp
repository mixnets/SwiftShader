#include <system/window.h>
#include "GL/glcorearb.h"
#include "GL/glext.h"
#include "EGL/egl.h"

#define GL_RGB565_OES                     0x8D62

#include "AndroidCommon.hpp"
#include "Image.hpp"

#include "../../Common/DebugAndroid.hpp"
#include "../../Common/GrallocAndroid.hpp"

GLenum getColorFormatFromAndroid(int format)
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
#if LATER
            if (GrallocModule::getInstance()->supportsConversion()) {
                return GL_RGB565_OES;
            } else {
				ALOGE("%s badness converting gralloc not supported for RGB_565",
					  __FUNCTION__);
                return GL_RGB565_OES;
            }
#else
            return GL_RGB565_OES;
#endif
        case HAL_PIXEL_FORMAT_YV12:
			return GL_YV12;
        case HAL_PIXEL_FORMAT_BLOB:
        case HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED:
        default:
			ALOGE("%s badness unsupported format=%x", __FUNCTION__, format);
    }
    return GL_RGBA;
}

// Used internally
GLenum getPixelFormatFromAndroid(int format)
{
    switch(format)
    {
        case HAL_PIXEL_FORMAT_RGBA_8888:
        case HAL_PIXEL_FORMAT_RGBX_8888:
        case HAL_PIXEL_FORMAT_RGB_888:
        case HAL_PIXEL_FORMAT_BGRA_8888:
            return GL_UNSIGNED_BYTE;
        case HAL_PIXEL_FORMAT_RGB_565:
#if LATER
            if (GrallocModule::getInstance()->supportsConversion()) {
                return GL_UNSIGNED_SHORT_5_6_5;
            } else {
				ALOGE("%s badness converting gralloc not supported for RGB_565",
					  __FUNCTION__);
                return GL_UNSIGNED_SHORT_5_6_5;
            }
#else
            return GL_UNSIGNED_SHORT_5_6_5;
#endif
        case HAL_PIXEL_FORMAT_YV12:
			return GL_UNSIGNED_BYTE;
        case HAL_PIXEL_FORMAT_BLOB:
        case HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED:
        default:
			ALOGE("%s badness unsupported format=%x", __FUNCTION__, format);
    }
    return GL_UNSIGNED_BYTE;
}
