#include "FrameBufferAndroid.hpp"

#include <cutils/log.h>

namespace sw
{
    FrameBufferAndroid::FrameBufferAndroid(ANativeWindow* window, int width, int height)
        : FrameBuffer(width, height, false, false),
        nativeWindow(window)
    {
        ANativeWindow_acquire(nativeWindow);
    }

    FrameBufferAndroid::~FrameBufferAndroid()
    {
        ANativeWindow_release(nativeWindow);
    }

    void FrameBufferAndroid::blit(void *source, const Rect *sourceRect, const Rect *destRect, Format format)
    {
        copy(source, format);
        ANativeWindow_unlockAndPost(nativeWindow);
    }

    void* FrameBufferAndroid::lock()
    {
        ANativeWindow_Buffer buffer;
        ANativeWindow_lock(nativeWindow, &buffer, &dirtyRegion);
        locked = buffer.bits;
        return locked;
    }

    void FrameBufferAndroid::unlock()
    {
        locked = 0;
    }

    bool FrameBufferAndroid::setSwapRectangle(int l, int t, int w, int h)
    {
        dirtyRegion.left = l;
        dirtyRegion.top = t;
        dirtyRegion.right = l+w;
        dirtyRegion.bottom = t+h;
        return true;
    }
}

extern "C"
{
    sw::FrameBuffer *createFrameBuffer(void *display, void* window, int width, int height)
    {
        return new sw::FrameBufferAndroid((ANativeWindow*)window, width, height);
    }
}
