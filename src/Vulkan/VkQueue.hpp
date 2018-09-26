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

#ifndef VK_QUEUE_HPP_
#define VK_QUEUE_HPP_

#include "VkObject.hpp"

namespace vk
{

class Queue : public VkObject<Queue, VkQueue>
{
public:
	~Queue() = delete;

private:
	// Dispatchable objects have private constructors
	Queue(uint32_t pFamilyIndex, float pPriority);
	// Only the base class may instantiate this object
	friend class VkObject<Queue, VkQueue>;

	uint32_t familyIndex = 0;
	float    priority = 0.0f;
};

static Queue* Cast(VkQueue queue)
{
	return reinterpret_cast<Queue::DispatchableType*>(queue)->get();
}

} // namespace vk

#endif // VK_QUEUE_HPP_
