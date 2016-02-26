#ifndef egl_Context_hpp
#define egl_Context_hpp

#include "common/Object.hpp"
#include "Common/Thread.hpp"

#include <EGL/egl.h>
#include <GLES/gl.h>

namespace egl
{
class Surface;
class Image;

class Context : public gl::Object
{
public:

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
	sw::Thread::TID boundThread;
};


inline void Context::unbind()
{
	boundThread = sw::Thread::TID();
}

inline void Context::bindToThread(sw::Thread::TID tid)
{
	boundThread = tid;
}

inline bool Context::isBound(sw::Thread::TID *outTid)
{
	if(outTid) {
		*outTid = boundThread;
	}
	return boundThread.isValid();
}
}

#endif   // egl_Context_hpp
