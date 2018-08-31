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

#include <vulkan/vulkan.h>

// Whenever any object deriving from VkObject is declared, also declare two
// utility function to make any reinterpret_cast between the two types
// deterministic (an object of type VkT can only be cast to an object of
// type T* and vice versa) in order to prevent bad casting of objects.
#define VkClass(type) class type; \
                      static type* Cast(Vk##type vkObject) { return reinterpret_cast<type*>(vkObject); } \
                      static Vk##type Cast(type* object) { return reinterpret_cast<Vk##type>(object); } \
                      class type : public VkObject<type, Vk##type>

namespace vk
{
	template<typename VkT>
	inline void destroy(VkT vkObject, const VkAllocationCallbacks* pAllocator)
	{
		auto object = Cast(vkObject);
		if(object)
		{
			object->destroy();
			pAllocator ? pAllocator->pfnFree(nullptr, object) : sw::deallocate(object);
		}
	}
}

template<typename T, typename VkT>
class VkObject {
public:
	operator VkT() { return reinterpret_cast<VkT>(this); }

	virtual void destroy() {} // Method overridden by objects to delete their content, if necessary

	void* operator new(size_t count, const VkAllocationCallbacks* pAllocator)
	{
		static const size_t alignment = 4;
		return pAllocator ?
			pAllocator->pfnAllocation(nullptr, count, alignment, T::getSystemAllocationScope()) :
			sw::allocate(count, alignment);
	}

	void operator delete(void* ptr, const VkAllocationCallbacks* pAllocator)
	{
		// Does nothing, objects are deleted through the destroy function
		ASSERT(false);
	}
};

#endif // VK_OBJECT_HPP_