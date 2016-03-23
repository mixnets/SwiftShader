// SwiftShader Software Renderer
//
// Copyright(c) 2005-2012 TransGaming Inc.
//
// All rights reserved. No part of this software may be copied, distributed, transmitted,
// transcribed, stored in a retrieval system, translated into any human or computer
// language by any means, or disclosed to third parties without the explicit written
// agreement of TransGaming Inc. Without such an agreement, no rights or licenses, express
// or implied, including but not limited to any patent rights, are granted to you.
//

// NameSpace.h: Defines the NameSpace class, which is used to
// allocate GL object names.

#ifndef gl_NameSpace_hpp
#define gl_NameSpace_hpp

#include "Object.hpp"

#include <unordered_map>
#include <algorithm>

typedef unsigned int GLuint;

namespace gl
{

template<class ObjectType, GLuint baseName = 1>
class NameSpace : public std::unordered_map<GLuint, ObjectType*>
{
public:
    NameSpace() : freeName(baseName)
	{
	}

    GLuint allocate()
	{
		GLuint name = freeName;

		while(find(name))
		{
			name++;
		}

		Map::insert({name, nullptr});
		freeName = name + 1;

		return name;
	}

	void insert(GLuint name, ObjectType *object = nullptr)
	{
		Map::operator[](name) = object;

		if(name == freeName)
		{
			freeName++;
		}
	}

    ObjectType *erase(GLuint name)
	{
		freeName = std::min(name, freeName);

		auto iterator = Map::find(name);

		if(iterator != end())
		{
			ObjectType *object = iterator->second;
			Map::erase(iterator);

			return object;
		}

		return nullptr;
	}

	ObjectType *find(GLuint name)
	{
		auto object = Map::find(name);

		if(object == end())
		{
			return nullptr;
		}

		return object->second;
	}

protected:
	typedef std::unordered_map<GLuint, ObjectType*> Map;

	GLuint freeName;   // Lowest known potentially free name
};

}

#endif   // gl_NameSpace_hpp
