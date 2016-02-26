#ifndef egl_Context_hpp
#define egl_Context_hpp

#include "common/Object.hpp"
#include "common/Thread.hpp"

#include <EGL/egl.h>
#include <GLES/gl.h>

namespace egl
{
class Surface;
class Image;

class Context : public gl::Object
{
public:
	Context();

	virtual void makeCurrent(Surface *surface) = 0;
	virtual void bindTexImage(Surface *surface) = 0;
	virtual EGLenum validateSharedImage(EGLenum target, GLuint name, GLuint textureLevel) = 0;
	virtual Image *createSharedImage(EGLenum target, GLuint name, GLuint textureLevel) = 0;
	virtual int getClientVersion() const = 0;
    virtual void finish() = 0;

	void bindToThread(sw::Thread::TID tid);
	void unbind();
	bool isBound(sw::Thread::TID *outTid);

protected:
	virtual ~Context() {};

private:
	bool bound;
	sw::Thread::TID boundThread;
};

inline Context::Context()
	: bound(false) { }

inline void Context::unbind()
{
	bound = false;
}

inline void Context::bindToThread(sw::Thread::TID tid) {
	bound = true;
	boundThread = tid;
}

inline bool Context::isBound(sw::Thread::TID *outTid) {
	if(outTid) {
		*outTid = boundThread;
	}
	return bound;
}
}

#endif   // egl_Context_hpp
