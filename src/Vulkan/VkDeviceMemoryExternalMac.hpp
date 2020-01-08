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

#ifndef VK_DEVICE_MEMORY_EXTERNAL_MAC_HPP_
#define VK_DEVICE_MEMORY_EXTERNAL_MAC_HPP_

#include "VkDeviceMemory.hpp"

struct __IOSurface;
typedef __IOSurface *IOSurfaceRef;

namespace vk
{

class IOSurfaceExternalMemory : vk::DeviceMemory::ExternalBase
{
public:
	IOSurfaceExternalMemory(const VkMemoryAllocateInfo* pAllocateInfo);

	void deallocate(void* buffer, size_t size) override;
	VkResult allocate(size_t size, void** pBuffer) override;

    VkExternalMemoryHandleTypeFlagBits getFlagBit() const override;

    void lock() const override;
	void unlock() const override;

	static bool supportsAllocateInfo(const VkMemoryAllocateInfo* pAllocateInfo);

	static const VkExternalMemoryHandleTypeFlagBits typeFlagBit = static_cast<VkExternalMemoryHandleTypeFlagBits>(0);
private:
	IOSurfaceRef ioSurface;
	int size;
	int plane;
};

} // namespace vk

#endif // VK_DEVICE_MEMORY_EXTERNAL_MAC_HPP_
