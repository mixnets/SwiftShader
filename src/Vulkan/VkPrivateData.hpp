// Copyright 2021 The SwiftShader Authors. All Rights Reserved.
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

#ifndef VK_PRIVATE_DATA_HPP_
#define VK_PRIVATE_DATA_HPP_

#include "VkObject.hpp"

namespace vk {

class PrivateData : public Object<PrivateData, VkPrivateDataSlotEXT>
{
public:
	PrivateData(const VkPrivateDataSlotCreateInfoEXT *pCreateInfo, void *mem) {}

	static size_t ComputeRequiredAllocationSize(const VkPrivateDataSlotCreateInfoEXT *pCreateInfo)
	{
		return 0;
	}

	VkResult set(VkObjectType objectType, uint64_t objectHandle, uint64_t data)
	{
		this->objectType = objectType;
		this->objectHandle = objectHandle;
		this->data = data;
		return VK_SUCCESS;
	}

	void get(VkObjectType objectType, uint64_t objectHandle, uint64_t *data) const
	{
		*data = ((this->objectType == objectType) && (this->objectHandle == objectHandle)) ? this->data : 0;
	}

private:
	VkObjectType objectType = VK_OBJECT_TYPE_UNKNOWN;
	uint64_t objectHandle = 0;
	uint64_t data = 0;
};

static inline PrivateData *Cast(VkPrivateDataSlotEXT object)
{
	return PrivateData::Cast(object);
}

}  // namespace vk

#endif  // VK_PRIVATE_DATA_HPP_
