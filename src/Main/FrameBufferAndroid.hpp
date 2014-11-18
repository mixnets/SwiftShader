#ifndef sw_FrameBufferAndroid_hpp
#define sw_FrameBufferAndroid_hpp

#include "Main/FrameBuffer.hpp"
#include "Common/Debug.hpp"

#include <cutils/log.h>
#include <ui/ANativeObjectBase.h>
#include <pixelflinger/format.h>

namespace sw
{
    class FrameBufferAndroid : public FrameBuffer
    {
    public:
        FrameBufferAndroid(ANativeWindow* window, int width, int height);

        ~FrameBufferAndroid();

        virtual void flip(void *source, Format format);
        virtual void blit(void *source, const Rect *sourceRect, const Rect *destRect, Format format);

        virtual void *lock();
        virtual void unlock();

        virtual bool connect();
        virtual void disconnect();
        bool swapBuffers();
        bool setSwapRectangle(int l, int t, int w, int h);

    private:
        int lock(ANativeWindowBuffer* buf, int usage, void** vaddr);
        int unlock(ANativeWindowBuffer* buf);

        ANativeWindow* nativeWindow;
        ANativeWindowBuffer* buffer;
        ANativeWindowBuffer* previousBuffer;
        gralloc_module_t const* module;
        void* bits;
        GGLFormat const* pixelFormatTable;

        struct RectAndroid {
            inline RectAndroid() { };
            inline RectAndroid(int32_t w, int32_t h)
                : left(0), top(0), right(w), bottom(h) { }
            inline RectAndroid(int32_t l, int32_t t, int32_t r, int32_t b)
                : left(l), top(t), right(r), bottom(b) { }
            RectAndroid& andSelf(const RectAndroid& r) {
                left   = max(left, r.left);
                top    = max(top, r.top);
                right  = min(right, r.right);
                bottom = min(bottom, r.bottom);
                return *this;
            }
            bool isEmpty() const {
                return (left>=right || top>=bottom);
            }
            void dump(char const* what) {
                ALOGD("%s { %5d, %5d, w=%5d, h=%5d }",
                        what, left, top, right-left, bottom-top);
            }

            int32_t left;
            int32_t top;
            int32_t right;
            int32_t bottom;
        };

        struct Region {
            inline Region() : count(0) { }
            typedef RectAndroid const* const_iterator;
            const_iterator begin() const { return storage; }
            const_iterator end() const { return storage+count; }
            static Region subtract(const RectAndroid& lhs, const RectAndroid& rhs) {
                Region reg;
                RectAndroid* storage = reg.storage;
                if (!lhs.isEmpty()) {
                    if (lhs.top < rhs.top) { // top rect
                        storage->left   = lhs.left;
                        storage->top    = lhs.top;
                        storage->right  = lhs.right;
                        storage->bottom = rhs.top;
                        storage++;
                    }
                    const int32_t top = max(lhs.top, rhs.top);
                    const int32_t bot = min(lhs.bottom, rhs.bottom);
                    if (top < bot) {
                        if (lhs.left < rhs.left) { // left-side rect
                            storage->left   = lhs.left;
                            storage->top    = top;
                            storage->right  = rhs.left;
                            storage->bottom = bot;
                            storage++;
                        }
                        if (lhs.right > rhs.right) { // right-side rect
                            storage->left   = rhs.right;
                            storage->top    = top;
                            storage->right  = lhs.right;
                            storage->bottom = bot;
                            storage++;
                        }
                    }
                    if (lhs.bottom > rhs.bottom) { // bottom rect
                        storage->left   = lhs.left;
                        storage->top    = rhs.bottom;
                        storage->right  = lhs.right;
                        storage->bottom = lhs.bottom;
                        storage++;
                    }
                    reg.count = storage - reg.storage;
                }
                return reg;
            }
            bool isEmpty() const {
                return count<=0;
            }
        private:
            RectAndroid storage[4];
            ssize_t count;
        };

        void copyBlt(
                ANativeWindowBuffer* dst, void* dst_vaddr,
                ANativeWindowBuffer* src, void const* src_vaddr,
                const Region& clip);

        RectAndroid dirtyRegion;
        RectAndroid oldDirtyRegion;
    };
}

#endif   // sw_FrameBufferAndroid
