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

#ifndef VK_FORMAT_UTILS_HPP_
#define VK_FORMAT_UTILS_HPP_

#include <vulkan/vulkan_core.h>

namespace vk
{
namespace utils
{
	bool IsSignedNonNormalizedInteger(VkFormat format);

	bool IsUnsignedNonNormalizedInteger(VkFormat format);

	bool IsStencil(VkFormat format);

	bool IsDepth(VkFormat format);

	int Bytes(VkFormat format);

	int PitchB(int width, int border, VkFormat format, bool target);

	int SliceB(int width, int height, int border, VkFormat format, bool target);
} // namespace utils
} // namespace vk

#endif // VK_FORMAT_UTILS_HPP_