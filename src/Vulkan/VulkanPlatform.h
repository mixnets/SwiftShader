// Copyright 2019 The SwiftShader Authors. All Rights Reserved.
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

#ifndef VULKAN_PLATFORM
#define VULKAN_PLATFORM

#include <cstdint>

template<typename T> class alignas(sizeof(uint64_t)) VkWrapper
{
public:
	using HandleType = T;

	VkWrapper()
	{
		u.dummy = 0;
	}

	VkWrapper(HandleType handle)
	{
		u.dummy = 0;
		u.handle = handle;
	}

	void operator=(HandleType handle)
	{
		u.handle = handle;
	}

	HandleType get() const
	{
		return u.handle;
	}

	operator HandleType() const
	{
		return u.handle;
	}

private:
	union PointerHandleUnion
	{
		HandleType handle;
		uint64_t dummy; // VkWrapper's size must always be 64 bits even when void* is 32 bits
	};
	PointerHandleUnion u;
};

// VkDescriptorSet objects are really just memory in the VkDescriptorPool
// object, so define different/more convenient operators for this object.
struct VkDescriptorSet_T;
template<> class alignas(sizeof(uint64_t)) VkWrapper<VkDescriptorSet_T*>
{
public:
	using HandleType = uint8_t*;

	VkWrapper(uint8_t* handle)
	{
		u.dummy = 0;
		u.handle = handle;
	}

	HandleType get() const
	{
		return u.handle;
	}

	operator HandleType() const
	{
		return u.handle;
	}

	HandleType operator+(ptrdiff_t rhs) const
	{
		return u.handle + rhs;
	}

	HandleType operator+=(ptrdiff_t rhs)
	{
		return (u.handle += rhs);
	}

	ptrdiff_t operator-(const HandleType rhs) const
	{
		return u.handle - rhs;
	}

private:
	union PointerHandleUnion
	{
		HandleType handle;
		uint64_t dummy; // VkWrapper's size must always be 64 bits even when void* is 32 bits
	};
	PointerHandleUnion u;
};

#define VK_DEFINE_NON_DISPATCHABLE_HANDLE(object) \
	typedef struct object##_T *object##Ptr; \
	typedef VkWrapper<object##Ptr> object;

#include <vulkan/vulkan.h>

#endif // VULKAN_PLATFORM
