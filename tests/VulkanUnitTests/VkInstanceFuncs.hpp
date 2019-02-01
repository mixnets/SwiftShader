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

// VkInstanceFuncs.h lists the vulkan driver per-instance functions.
//
// TODO: Generate this list.

// VK_INSTANCE(<function name>, <return type>, <arguments>...)
VK_INSTANCE(vkEnumeratePhysicalDevices, VkResult, VkInstance, uint32_t*, VkPhysicalDevice*)
VK_INSTANCE(vkGetPhysicalDeviceProperties, void, VkPhysicalDevice, VkPhysicalDeviceProperties*)
