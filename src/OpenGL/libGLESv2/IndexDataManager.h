// Copyright 2016 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// IndexDataManager.h: Defines the IndexDataManager, a class that
// runs the Buffer translation process for index buffers.

#ifndef LIBGLESV2_INDEXDATAMANAGER_H_
#define LIBGLESV2_INDEXDATAMANAGER_H_

#include "Context.h"

#include <GLES2/gl2.h>

namespace es2
{

struct TranslatedIndexData
{
	unsigned int minIndex;
	unsigned int maxIndex;
	unsigned int indexOffset;

	sw::Resource *indexBuffer;
};

class StreamingIndexBuffer
{
public:
	StreamingIndexBuffer(unsigned int initialSize);
	virtual ~StreamingIndexBuffer();

	void *map(unsigned int requiredSpace, unsigned int *offset);
	void unmap();
	void reserveSpace(unsigned int requiredSpace, GLenum type);

	sw::Resource *getResource() const;

private:
	sw::Resource *mIndexBuffer;
	unsigned int mBufferSize;
	unsigned int mWritePosition;
};

struct PrimitiveRestartData
{
	struct Datum
	{
		Datum(const void *i, GLsizei c) : indices(i), count(c) {}
		const void *indices;
		GLsizei count;
	};

	PrimitiveRestartData() : index(-1) {}

	std::vector<Datum> data;
	GLuint index;
};

class IndexDataManager
{
public:
	IndexDataManager();
	virtual ~IndexDataManager();

	GLenum prepareIndexData(GLenum type, GLuint start, GLuint end, GLsizei count, Buffer *arrayElementBuffer, const void *indices, TranslatedIndexData *translated, bool preparedIndices);
	GLenum computePrimitiveRestart(GLenum type, GLsizei count, Buffer *arrayElementBuffer, const void *indices, PrimitiveRestartData* restartInfo);

	static std::size_t typeSize(GLenum type);

private:
	GLenum prepareIndices(GLenum type, GLsizei count, Buffer *buffer, const void* &indices, intptr_t& offset);

	StreamingIndexBuffer *mStreamingBuffer;
};

}

#endif   // LIBGLESV2_INDEXDATAMANAGER_H_
