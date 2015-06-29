#include "DebugAndroid.hpp"

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <cutils/properties.h>

void AndroidEnterDebugger()
{
    ALOGE(__FUNCTION__);
#ifndef NDEBUG
    static volatile int * const makefault = nullptr;
    char value[PROPERTY_VALUE_MAX];
    property_get("debug.db.uid", value, "-1");
    int debug_uid = atoi(value);
    if ((debug_uid >= 0) && (geteuid() < static_cast<uid_t>(debug_uid)))
    {
        ALOGE("Waiting for debugger: gdbserver :${PORT} --attach %u. Look for thread %u", getpid(), gettid());
        volatile int waiting = 1;
        while (waiting) {
            sleep(1);
        }
    } else {
        ALOGE("No debugger");
    }
#endif
}

const char* android_pixel_format_to_string(int format) {
    switch (format) {
        // Formats that are universal across versions
        case 1: // HAL_PIXEL_FORMAT_RGBA_8888
            return "RGBA_8888";
        case 2: // HAL_PIXEL_FORMAT_RGBX_8888:
            return "RGBX_8888";
        case 3: // HAL_PIXEL_FORMAT_RGB_888:
            return "RGB_888";
        case 4: // HAL_PIXEL_FORMAT_RGB_565:
            return "RGB_565";
        case 5: // HAL_PIXEL_FORMAT_BGRA_8888:
            return "BGRA_8888";
        case 6: // HAL_PIXEL_FORMAT_RGBA_5551. Support was dropped on K (API 19)
            return "RGBA_5551";
        case 7: // HAL_PIXEL_FORMAT_RGBA_4444. Support was dropped on K (API 19)
            return "RGBA_4444";
        case 0xC: // HAL_PIXEL_FORMAT_sRGB_A_8888
            return "sRGB_A_8888";
        case 0xD: // HAL_PIXEL_FORMAT_sRGB_X_8888
            return "sRGB_X_8888";

        case 0x32315659: // HAL_PIXEL_FORMAT_YV12
            return "YV12";

        case 0x20203859: // HAL_PIXEL_FORMAT_Y8
            return "Y8";
        case 0x20363159: // HAL_PIXEL_FORMAT_Y16
            return "Y16";

        // Aks RAW_SENSOR.
        case 0x20: // HAL_PIXEL_FORMAT_RAW_SENSOR
            return "RAW_16";

        case 0x25: // HAL_PIXEL_FORMAT_RAW10
            return "RAW_10";

        case 0x26: // HAL_PIXEL_FORMAT_RAW12
            return "RAW_12";

        case 0x24: // HAL_PIXEL_FORMAT_RAW_OPAQUE
            return "RAW_OPAQUE";

        case 0x21: // HAL_PIXEL_FORMAT_BLOB
            return "BLOB";

        case 0x22: // HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED
            return "IMPLEMENTATION_DEFINED";

        case 0x23: // HAL_PIXEL_FORMAT_YCbCr_420_888
            return "YCbCr_420_888";

        case 0x27: // HAL_PIXEL_FORMAT_YCbCr_422_888
            return "YCbCr_422_888";

        case 0x28: // HAL_PIXEL_FORMAT_YCbCr_444_888
            return "YCbCr_444_888";

        case 0x29: // HAL_PIXEL_FORMAT_FLEX_RGB_888
            return "FLEX_RGB_888";

        case 0x2A: // HAL_PIXEL_FORMAT_FLEX_RGBA_8888
            return "FLEX_RGBA_8888";

        case 0x10: // HAL_PIXEL_FORMAT_YCbCr_422_SP
            return "YCbCr_422_SP";

        case 0x11: // HAL_PIXEL_FORMAT_YCrCb_420_SP
            return "YCrCb_420_SP";

        case 0x14: // HAL_PIXEL_FORMAT_YCbCr_422_I
            return "YCbCr_422_I";
    }
    return "UNKNOWN";
}

void trace(const char *format, ...)
{
#ifndef NDEBUG
    va_list vararg;
    va_start(vararg, format);
    android_vprintLog(ANDROID_LOG_VERBOSE, NULL, LOG_TAG, format, vararg);
    va_end(vararg);
#endif
}
