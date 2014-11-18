#include "FrameBufferAndroid.hpp"

namespace sw
{
    FrameBufferAndroid::FrameBufferAndroid(ANativeWindow* window, int width, int height)
        : FrameBuffer(width, height, false, false),
        nativeWindow(window), buffer(0), previousBuffer(0), module(0), bits(NULL)
    {
        hw_module_t const* pModule;
        hw_get_module(GRALLOC_HARDWARE_MODULE_ID, &pModule);
        module = reinterpret_cast<gralloc_module_t const*>(pModule);

        pixelFormatTable = gglGetPixelFormatTable();

        // keep a reference on the window
        nativeWindow->common.incRef(&nativeWindow->common);
        nativeWindow->query(nativeWindow, NATIVE_WINDOW_WIDTH, &width);
        nativeWindow->query(nativeWindow, NATIVE_WINDOW_HEIGHT, &height);
    }

    FrameBufferAndroid::~FrameBufferAndroid()
    {
        if (buffer) {
            buffer->common.decRef(&buffer->common);
        }
        if (previousBuffer) {
            previousBuffer->common.decRef(&previousBuffer->common);
        }
        nativeWindow->common.decRef(&nativeWindow->common);
    }

    void FrameBufferAndroid::flip(void *source, Format format)
    {
        copy(source, format);

        swapBuffers();
    }

    void FrameBufferAndroid::blit(void *source, const Rect *sourceRect, const Rect *destRect, Format format)
    {
        UNIMPLEMENTED();
    }

    void* FrameBufferAndroid::lock()
    {
        locked = bits;
        return locked;
    }

    void FrameBufferAndroid::unlock()
    {
        locked = 0;
    }

    bool FrameBufferAndroid::connect()
    {
        // we're intending to do software rendering
        native_window_set_usage(nativeWindow,
                GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_OFTEN);

        // dequeue a buffer
        int fenceFd = -1;
        if (nativeWindow->dequeueBuffer(nativeWindow, &buffer,
                &fenceFd) != android::NO_ERROR) {
            return false;
        }

        // wait for the buffer
        android::sp<android::Fence> fence(new android::Fence(fenceFd));
        if (fence->wait(android::Fence::TIMEOUT_NEVER) != android::NO_ERROR) {
            nativeWindow->cancelBuffer(nativeWindow, buffer, fenceFd);
            return false;
        }

        // allocate a corresponding depth-buffer
        width = buffer->width;
        height = buffer->height;
        if (depth.format) {
            depth.width   = width;
            depth.height  = height;
            depth.stride  = depth.width; // use the width here
            depth.data    = (GGLubyte*)malloc(depth.stride*depth.height*2);
            if (depth.data == 0) {
                return false;
            }
        }

        // keep a reference on the buffer
        buffer->common.incRef(&buffer->common);

        // pin the buffer down
        if (lock(buffer, GRALLOC_USAGE_SW_READ_OFTEN |
                GRALLOC_USAGE_SW_WRITE_OFTEN, &bits) != android::NO_ERROR) {
            ALOGE("connect() failed to lock buffer %p (%ux%u)",
                    buffer, buffer->width, buffer->height);
            return false;
            // FIXME: we should make sure we're not accessing the buffer anymore
        }
        return true;
    }

    void FrameBufferAndroid::disconnect()
    {
        if (buffer && bits) {
            bits = NULL;
            unlock(buffer);
        }
        if (buffer) {
            nativeWindow->cancelBuffer(nativeWindow, buffer, -1);
            buffer->common.decRef(&buffer->common);
            buffer = 0;
        }
        if (previousBuffer) {
            previousBuffer->common.decRef(&previousBuffer->common);
            previousBuffer = 0;
        }
    }

    void FrameBufferAndroid::copyBlt(
            ANativeWindowBuffer* dst, void* dst_vaddr,
            ANativeWindowBuffer* src, void const* src_vaddr,
            const Region& clip)
    {
        // NOTE: dst and src must be the same format

        Region::const_iterator cur = clip.begin();
        Region::const_iterator end = clip.end();

        const size_t bpp = pixelFormatTable[src->format].size;
        const size_t dbpr = dst->stride * bpp;
        const size_t sbpr = src->stride * bpp;

        uint8_t const * const src_bits = (uint8_t const *)src_vaddr;
        uint8_t       * const dst_bits = (uint8_t       *)dst_vaddr;

        while (cur != end) {
            const RectAndroid& r(*cur++);
            ssize_t w = r.right - r.left;
            ssize_t h = r.bottom - r.top;
            if (w <= 0 || h<=0) continue;
            size_t size = w * bpp;
            uint8_t const * s = src_bits + (r.left + src->stride * r.top) * bpp;
            uint8_t       * d = dst_bits + (r.left + dst->stride * r.top) * bpp;
            if (dbpr==sbpr && size==sbpr) {
                size *= h;
                h = 1;
            }
            do {
                memcpy(d, s, size);
                d += dbpr;
                s += sbpr;
            } while (--h > 0);
        }
    }

    bool FrameBufferAndroid::swapBuffers()
    {
        if (!buffer) {
            return false;
        }

        /*
         * Handle eglSetSwapRectangleANDROID()
         * We copyback from the front buffer
         */
        if (!dirtyRegion.isEmpty()) {
            dirtyRegion.andSelf(RectAndroid(buffer->width, buffer->height));
            if (previousBuffer) {
                // This was const Region copyBack, but that causes an
                // internal compile error on simulator builds
                /*const*/ Region copyBack(Region::subtract(oldDirtyRegion, dirtyRegion));
                if (!copyBack.isEmpty()) {
                    void* prevBits;
                    if (lock(previousBuffer,
                            GRALLOC_USAGE_SW_READ_OFTEN, &prevBits) == android::NO_ERROR) {
                        // copy from previousBuffer to buffer
                        copyBlt(buffer, bits, previousBuffer, prevBits, copyBack);
                        unlock(previousBuffer);
                    }
                }
            }
            oldDirtyRegion = dirtyRegion;
        }

        if (previousBuffer) {
            previousBuffer->common.decRef(&previousBuffer->common);
            previousBuffer = 0;
        }

        unlock(buffer);
        previousBuffer = buffer;
        nativeWindow->queueBuffer(nativeWindow, buffer, -1);
        buffer = 0;

        // dequeue a new buffer
        int fenceFd = -1;
        if (nativeWindow->dequeueBuffer(nativeWindow, &buffer, &fenceFd) == android::NO_ERROR) {
            android::sp<android::Fence> fence(new android::Fence(fenceFd));
            if (fence->wait(android::Fence::TIMEOUT_NEVER)) {
                nativeWindow->cancelBuffer(nativeWindow, buffer, fenceFd);
                return false;
            }

            // reallocate the depth-buffer if needed
            if ((width != buffer->width) || (height != buffer->height)) {
                // TODO: we probably should reset the swap rect here
                // if the window size has changed
                width = buffer->width;
                height = buffer->height;
                if (depth.data) {
                    free(depth.data);
                    depth.width   = width;
                    depth.height  = height;
                    depth.stride  = buffer->stride;
                    depth.data    = (GGLubyte*)malloc(depth.stride*depth.height*2);
                    if (depth.data == 0) {
                        return false;
                    }
                }
            }

            // keep a reference on the buffer
            buffer->common.incRef(&buffer->common);

            // finally pin the buffer down
            if (lock(buffer, GRALLOC_USAGE_SW_READ_OFTEN |
                    GRALLOC_USAGE_SW_WRITE_OFTEN, &bits) != android::NO_ERROR) {
                ALOGE("eglSwapBuffers() failed to lock buffer %p (%ux%u)",
                        buffer, buffer->width, buffer->height);
                return false;
                // FIXME: we should make sure we're not accessing the buffer anymore
            }
        } else {
            return false;
        }

        return true;
    }


    bool FrameBufferAndroid::setSwapRectangle(int l, int t, int w, int h)
    {
        dirtyRegion = RectAndroid(l, t, l+w, t+h);
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
