#include "Main/FrameBuffer.hpp"

#include "Main/FrameBufferOzone.hpp"

#if defined(USE_X11)
#include "Main/FrameBufferX11.hpp"

#include "libX11.hpp"
#endif

NO_SANITIZE_FUNCTION sw::FrameBuffer *createFrameBuffer(void* display, intptr_t window, int width, int height)
{
#if defined(USE_X11)
        if (reinterpret_cast<::Display*>(display)) {
		return new sw::FrameBufferX11((::Display*)display, window, width, height);
        }
#endif
	return new sw::FrameBufferOzone((intptr_t)display, window, width, height);
}
