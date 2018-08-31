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

#ifndef VK_SHADER_MODULE_HPP_
#define VK_SHADER_MODULE_HPP_

#include "VkObject.hpp"

namespace vk {

VkClass(ShaderModule) {
public:
	ShaderModule(const VkAllocationCallbacks* pAllocator, size_t pCodeSize, const uint32_t* pCode)
	{
		code = reinterpret_cast<char*>(vk::allocate(pCodeSize, pAllocator, getSystemAllocationScope(), pCode));
	}

	void destroy(const VkAllocationCallbacks* pAllocator) override
	{
		vk::deallocate(code, pAllocator);
		code = nullptr;
	}

	static VkSystemAllocationScope getSystemAllocationScope() { return VK_SYSTEM_ALLOCATION_SCOPE_OBJECT; }

private:
	char* code = nullptr;
};

} // namespace vk

#endif // VK_SHADER_MODULE_HPP_