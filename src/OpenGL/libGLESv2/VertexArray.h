// SwiftShader Software Renderer
//
// Copyright(c) 2015 Google Inc.
//
// All rights reserved. No part of this software may be copied, distributed, transmitted,
// transcribed, stored in a retrieval system, translated into any human or computer
// language by any means, or disclosed to third parties without the explicit written
// agreement of Google Inc. Without such an agreement, no rights or licenses, express
// or implied, including but not limited to any patent rights, are granted to you.
//

// VertexArray.h: Defines the es2::VertexArray class

#ifndef LIBGLESV2_VERTEX_ARRAY_H_
#define LIBGLESV2_VERTEX_ARRAY_H_

#include "Buffer.h"
#include "Renderer/Renderer.hpp"

#define GL_APICALL
#include <GLES2/gl2.h>

#include <vector>

namespace es2
{

enum
{
	MAX_VERTEX_ATTRIBS = 16,
};

// Helper structure describing a single vertex attribute
class VertexAttribute
{
public:
	VertexAttribute() : mType(GL_FLOAT), mSize(0), mNormalized(false), mStride(0), mDivisor(0), mPointer(NULL), mArrayEnabled(false)
	{
		mCurrentValue[0].f = 0.0f;
		mCurrentValue[1].f = 0.0f;
		mCurrentValue[2].f = 0.0f;
		mCurrentValue[3].f = 1.0f;
		mCurrentValueType = ValueUnion::FloatType;
	}

	int typeSize() const
	{
		switch(mType)
		{
		case GL_BYTE:           return mSize * sizeof(GLbyte);
		case GL_UNSIGNED_BYTE:  return mSize * sizeof(GLubyte);
		case GL_SHORT:          return mSize * sizeof(GLshort);
		case GL_UNSIGNED_SHORT: return mSize * sizeof(GLushort);
		case GL_FIXED:          return mSize * sizeof(GLfixed);
		case GL_FLOAT:          return mSize * sizeof(GLfloat);
		default: UNREACHABLE(); return mSize * sizeof(GLfloat);
		}
	}

	GLsizei stride() const
	{
		return mStride ? mStride : typeSize();
	}

	inline float getCurrentValue(int i) const
	{
		switch(mCurrentValueType)
		{
		case ValueUnion::FloatType:	return mCurrentValue[i].f;
		case ValueUnion::IntType:	return static_cast<float>(mCurrentValue[i].i);
		case ValueUnion::UIntType:	return static_cast<float>(mCurrentValue[i].ui);
		default: UNREACHABLE();		return mCurrentValue[i].f;
		}
	}

	inline GLint getCurrentValueI(int i) const
	{
		switch(mCurrentValueType)
		{
		case ValueUnion::FloatType:	return static_cast<GLint>(mCurrentValue[i].f);
		case ValueUnion::IntType:	return mCurrentValue[i].i;
		case ValueUnion::UIntType:	return static_cast<GLint>(mCurrentValue[i].ui);
		default: UNREACHABLE();		return mCurrentValue[i].i;
		}
	}

	inline GLuint getCurrentValueUI(int i) const
	{
		switch(mCurrentValueType)
		{
		case ValueUnion::FloatType:	return static_cast<GLuint>(mCurrentValue[i].f);
		case ValueUnion::IntType:	return static_cast<GLuint>(mCurrentValue[i].i);
		case ValueUnion::UIntType:	return mCurrentValue[i].ui;
		default: UNREACHABLE();		return mCurrentValue[i].ui;
		}
	}

	inline void setCurrentValue(const GLfloat *values)
	{
		mCurrentValue[0].f = values[0];
		mCurrentValue[1].f = values[1];
		mCurrentValue[2].f = values[2];
		mCurrentValue[3].f = values[3];
		mCurrentValueType = ValueUnion::FloatType;
	}

	inline void setCurrentValue(const GLint *values)
	{
		mCurrentValue[0].i = values[0];
		mCurrentValue[1].i = values[1];
		mCurrentValue[2].i = values[2];
		mCurrentValue[3].i = values[3];
		mCurrentValueType = ValueUnion::IntType;
	}

	inline void setCurrentValue(const GLuint *values)
	{
		mCurrentValue[0].ui = values[0];
		mCurrentValue[1].ui = values[1];
		mCurrentValue[2].ui = values[2];
		mCurrentValue[3].ui = values[3];
		mCurrentValueType = ValueUnion::UIntType;
	}

	// From glVertexAttribPointer
	GLenum mType;
	GLint mSize;
	bool mNormalized;
	GLsizei mStride;   // 0 means natural stride
	GLuint mDivisor;   // From glVertexAttribDivisor

	union
	{
		const void *mPointer;
		intptr_t mOffset;
	};

	gl::BindingPointer<Buffer> mBoundBuffer;   // Captured when glVertexAttribPointer is called.

	bool mArrayEnabled;   // From glEnable/DisableVertexAttribArray
private:
	union ValueUnion
	{
		enum Type { FloatType, IntType, UIntType };

		float f;
		GLint i;
		GLuint ui;
	};
	ValueUnion mCurrentValue[4];   // From glVertexAttrib
	ValueUnion::Type mCurrentValueType;
};

typedef VertexAttribute VertexAttributeArray[MAX_VERTEX_ATTRIBS];

class VertexArray : public gl::NamedObject
{
public:
	VertexArray(GLuint name);
	~VertexArray();

	const VertexAttribute& getVertexAttribute(size_t attributeIndex) const;
	VertexAttributeArray& getVertexAttributes() { return mVertexAttributes; }

	void detachBuffer(GLuint bufferName);
	void setVertexAttribDivisor(GLuint index, GLuint divisor);
	void enableAttribute(unsigned int attributeIndex, bool enabledState);
	void setAttributeState(unsigned int attributeIndex, Buffer *boundBuffer, GLint size, GLenum type,
	                       bool normalized, GLsizei stride, const void *pointer);

	Buffer *getElementArrayBuffer() const { return mElementArrayBuffer; }
	void setElementArrayBuffer(Buffer *buffer);

private:
	VertexAttributeArray mVertexAttributes;
	gl::BindingPointer<Buffer> mElementArrayBuffer;
};

}

#endif // LIBGLESV2_VERTEX_ARRAY_H_
