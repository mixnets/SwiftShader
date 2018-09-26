// Copyright 2018 The SwiftShader Authors. All Rights Reserved.
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

#ifndef VK_OBJECT_HPP_
#define VK_OBJECT_HPP_

#include "VkDebug.hpp"
#include "VkMemory.h"
#include <vulkan/vulkan.h>
#include <vulkan/vk_icd.h>

namespace vk
{
// For use in the placement new to make it verbose that we're allocating an object using internal memory
static constexpr VkAllocationCallbacks* INTERNAL_MEMORY = nullptr;

// Wrapper around dispatchable objects (Instance, PhysicalDevice, Device, CommandBuffer and Queue)
template<typename T, typename VkT>
class VkDispatchableObject
{
	VK_LOADER_DATA loaderData = { ICD_LOADER_MAGIC };
public:
	VkDispatchableObject(T* pDispatchableObject) : dispatchableObject(pDispatchableObject)
	{
	}

	operator VkT()
	{
		return reinterpret_cast<VkT>(this);
	}

	void destroy(const VkAllocationCallbacks* pAllocator)
	{
		if(dispatchableObject)
		{
			vk::destroy(dispatchableObject, pAllocator);
			dispatchableObject = nullptr;
		}
	}

	void* operator new(size_t count, const VkAllocationCallbacks* pAllocator)
	{
		return vk::allocate(count, pAllocator, T::GetAllocationScope());
	}

	void operator delete(void* ptr, const VkAllocationCallbacks* pAllocator)
	{
		// Does nothing, objects are deleted through the destroy function
		ASSERT(false);
	}

	T* get()
	{
		return dispatchableObject;
	}
private:
	T* dispatchableObject = nullptr;
};

template<typename T, typename VkT>
class VkObject
{
public:
	typedef VkDispatchableObject<T, VkT> DispatchableType;

	operator VkT()
	{
		return reinterpret_cast<VkT>(this);
	}

	template<class ...Args>
	static VkT newDispatchable(const VkAllocationCallbacks* pAllocator, Args... args)
	{
		return *new (pAllocator) DispatchableType(new (pAllocator) T(args...));
	}

	virtual void destroy(const VkAllocationCallbacks* pAllocator) {} // Method overridden by objects to delete their content, if necessary

	void* operator new(size_t count, const VkAllocationCallbacks* pAllocator)
	{
		return vk::allocate(count, pAllocator, T::GetAllocationScope());
	}

	void operator delete(void* ptr, const VkAllocationCallbacks* pAllocator)
	{
		// Does nothing, objects are deleted through the destroy function
		ASSERT(false);
	}

	static constexpr VkSystemAllocationScope GetAllocationScope() { return VK_SYSTEM_ALLOCATION_SCOPE_OBJECT; }

protected:
	// All derived classes should have deleted destructors
	~VkObject() {}
};

} // namespace vk

#endif // VK_OBJECT_HPP_