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

namespace gl
{

template<class ObjectType, GLuint baseName = 1>
class NameSpace : std::unordered_map<GLuint, ObjectType*>
{
public:
    NameSpace() : freeName(baseName)
	{
	}

	bool empty()
	{
		return Map::empty();
	}

	// Should only be called when the map is known not to be empty
	GLuint firstName()
	{
		return begin()->first;
	}

	// Reserve the lowest available name
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

	void insert(GLuint name, ObjectType *object)
	{
		Map::operator[](name) = object;

		if(name == freeName)
		{
			freeName++;
		}
	}

    ObjectType *remove(GLuint name)
	{
		if(name < freeName)
		{
			freeName = name;
		}

		auto element = Map::find(name);

		if(element != end())
		{
			ObjectType *object = element->second;
			Map::erase(element);

			return object;
		}

		return nullptr;
	}

	ObjectType *find(GLuint name)
	{
		if(name < baseName)
		{
			return nullptr;
		}

		auto element = Map::find(name);

		if(element == end())
		{
			return nullptr;
		}

		return element->second;
	}

protected:
	typedef std::unordered_map<GLuint, ObjectType*> Map;

	GLuint freeName;   // Lowest known potentially free name
};

}

#endif   // gl_NameSpace_hpp
