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

#ifndef VK_CONFIG_HPP_
#define VK_CONFIG_HPP_

namespace vk
{

enum {
	REQUIRED_MEMORY_ALIGNMENT = 8, // For 64 bit formats on ARM64
	REQUIRED_MEMORY_TYPE_BITS = 1,
};

enum {
	MaxImageLevels1D = 14,
	MaxImageLevels2D = 14,
	MaxImageLevels3D = 11,
	MaxImageLevelsCube = 14,
	MaxImageArrayLayers = 11,
};

enum {
	MaxVertexInputBindings = 16,
};

}

#endif // VK_CONFIG_HPP_
