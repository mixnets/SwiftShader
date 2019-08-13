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

#include "VkDeviceMemory.hpp"

#include "VkConfig.h"

#if SWIFTSHADER_EXTERNAL_MEMORY_TYPE != SWIFTSHADER_EXTERNAL_MEMORY_MEMFD
#error "This source file should only be compiled for systems supporting memfd_create()"
#endif

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

namespace vk
{

VkResult DeviceMemory::importMemoryFd(int fd) {
	if (fd < 0)
	{
		// Name is only used for debugging.
		static int counter = 0;
		char name[64];
		snprintf(name, sizeof(name), "SwiftShaderVulkanMemory%d", ++counter);
		if (!memfd.allocate(name, size))
		{
			return VK_ERROR_OUT_OF_DEVICE_MEMORY;
		}
	}
	else
	{
		memfd.importFd(fd);
	}
	return VK_SUCCESS;
}

int DeviceMemory::exportMemoryFd() const {
	return memfd.exportFd();
}

void DeviceMemory::deallocateExternal()
{
	if (memfd.isValid())
	{
		::munmap(buffer, size);
		memfd.close();
		buffer = nullptr;
	}
}

VkResult DeviceMemory::allocateExternal()
{
	if (memfd.isValid())
	{
		void* addr = ::mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, memfd.fd, 0);
		if (addr == MAP_FAILED) {
			WARN("mmap() for %lld bytes returned MAP_FAILED: %s", (long long)size, strerror(errno));
			return VK_ERROR_OUT_OF_DEVICE_MEMORY;
		}
		buffer = addr;
	}
	return VK_SUCCESS;
}

} // namespace vk
