#ifndef sw_FrameBufferAndroid_hpp
#define sw_FrameBufferAndroid_hpp

#include "Main/FrameBuffer.hpp"
#include "Common/Debug.hpp"

#include <hardware/gralloc.h>
#include <system/window.h>

namespace sw
{
    class FrameBufferAndroid : public FrameBuffer
    {
    public:
        FrameBufferAndroid(ANativeWindow* window, int width, int height);

        ~FrameBufferAndroid();

        void flip(sw::Surface *source) override {blit(source, nullptr, nullptr);};
		void blit(sw::Surface *source, const Rect *sourceRect, const Rect *destRect) override;

		void *lock() override;
		void unlock() override;

        bool setSwapRectangle(int l, int t, int w, int h);

    private:
        ANativeWindow *nativeWindow;
        ANativeWindowBuffer *buffer;
        gralloc_module_t const *gralloc;
    };
}

#endif   // sw_FrameBufferAndroid
