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

#ifndef VK_FENCE_HPP_
#define VK_FENCE_HPP_

#include "VkObject.hpp"

namespace vk
{

class Fence : public VkObject<Fence, VkFence>
{
public:
	Fence(const VkFenceCreateInfo* pCreateInfo) : flags(pCreateInfo->flags) {}

	~Fence() = delete;

	void signal()
	{
		isSignaled = true;
	}

	void reset()
	{
		isSignaled = false;
	}

	VkResult getStatus() const
	{
		return isSignaled ? VK_SUCCESS : VK_NOT_READY;
	}

private:
	VkFenceCreateFlags flags = 0;
	bool isSignaled = false;
};

static inline Fence* Cast(VkFence object)
{
	return reinterpret_cast<Fence*>(object);
}

} // namespace vk

#endif // VK_FENCE_HPP_
