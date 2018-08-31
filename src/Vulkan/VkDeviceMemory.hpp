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

#ifndef VK_DEVICE_MEMORY_HPP_
#define VK_DEVICE_MEMORY_HPP_

#include "VkObject.hpp"

namespace vk {

VkClass(DeviceMemory) {
public:
	DeviceMemory(const VkAllocationCallbacks* pAllocator, VkDeviceSize pSize) : size(pSize)
	{
		buffer = vk::allocate(size, pAllocator, getSystemAllocationScope());
	}

	void* map(VkDeviceSize offset, VkDeviceSize size)
	{
		return reinterpret_cast<char*>(buffer) + offset;
	}

	void unmap()
	{
	}

	static VkSystemAllocationScope getSystemAllocationScope() { return VK_SYSTEM_ALLOCATION_SCOPE_OBJECT; }

private:
	void*        buffer;
	VkDeviceSize size;
};

} // namespace vk

#endif // VK_DEVICE_MEMORY_HPP_