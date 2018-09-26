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

#ifndef VK_BUFFER_HPP_
#define VK_BUFFER_HPP_

#include "VkObject.hpp"

namespace vk
{

class Buffer : public VkObject<Buffer, VkBuffer>
{
public:
	Buffer(const VkBufferCreateInfo* pCreateInfo, char* membersMemory);
	~Buffer() = delete;
	void destroy(const VkAllocationCallbacks* pAllocator) override;

	static size_t ComputeRequiredAllocationSize(const VkBufferCreateInfo* pCreateInfo);

	const VkMemoryRequirements getMemoryRequirements() const;
	void bind(VkDeviceMemory pDeviceMemory, VkDeviceSize pMemoryOffset);
	VkDeviceMemory getDeviceMemory() const;


private:
	VkDeviceMemory        deviceMemory = nullptr;
	VkDeviceSize          memoryOffset = 0;
	VkBufferCreateFlags   flags = 0;
	VkDeviceSize          size = 0;
	VkBufferUsageFlags    usage = 0;
	VkSharingMode         sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	uint32_t              queueFamilyIndexCount = 0;
	uint32_t*             queueFamilyIndices = nullptr;
};

static inline Buffer* Cast(VkBuffer object)
{
	return reinterpret_cast<Buffer*>(object);
}

} // namespace vk

#endif // VK_BUFFER_HPP_