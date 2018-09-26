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

namespace vk
{

class DeviceMemory : public VkObject<DeviceMemory, VkDeviceMemory>
{
public:
	DeviceMemory(const VkMemoryAllocateInfo* pCreateInfo, const Memory& mem);
	~DeviceMemory() = delete;

	static MemorySize ComputeRequiredAllocationSize(const VkMemoryAllocateInfo* pCreateInfo);

	void destroy(const VkAllocationCallbacks* pAllocator);
	void* map(VkDeviceSize offset, VkDeviceSize size);
	void unmap();
	void flush();
	void invalidate();
	VkDeviceSize getCommittedMemoryInBytes() const;

private:
	void*        buffer = nullptr;
	VkDeviceSize size = 0;
};

static inline DeviceMemory* Cast(VkDeviceMemory object)
{
	return reinterpret_cast<DeviceMemory*>(object);
}


} // namespace vk

#endif // VK_DEVICE_MEMORY_HPP_