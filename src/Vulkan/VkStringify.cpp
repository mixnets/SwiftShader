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

#include "VkStringify.hpp"

#include "System/Debug.hpp"

#include <vulkan/vk_ext_provoking_vertex.h>
#include <vulkan/vk_google_filtering_precision.h>
#include <vulkan/vulkan.hpp>

#include <iostream>
#include <map>
#include <string>

namespace vk {

std::string Stringify(VkStructureType value)
{
	vkh::StructureType sType = static_cast<vkh::StructureType>(value);
	return vkh::to_string(sType);
}

}  // namespace vk
