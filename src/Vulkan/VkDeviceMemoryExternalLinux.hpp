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

#include "VkDebug.hpp"
#include "System/MemFdLinux.hpp"

#include <errno.h>
#include <string.h>
#include <sys/mman.h>

namespace vk
{

class DeviceMemory::External {
public:
	struct CreateInfo {
		bool importFd = false;
		bool exportFd = false;
		int fd = -1;

		CreateInfo() = default;

		// Parse the VkMemoryAllocateInfo->pNext chain to initialize a CreateInfo.
		CreateInfo(const VkMemoryAllocateInfo* pCreateInfo)
		{
			const auto* createInfo = reinterpret_cast<const VkBaseInStructure*>(pCreateInfo->pNext);
			while (createInfo)
			{
				switch (createInfo->sType)
				{
				case VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHR:
					{
						const auto* importInfo = reinterpret_cast<const VkImportMemoryFdInfoKHR*>(createInfo);
						
						if (importInfo->handleType != VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT)
						{
							UNIMPLEMENTED("importInfo->handleType");
						}
						importFd = true;
						fd = importInfo->fd;
					}
					break;
				case VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO:
					{
						const auto* exportInfo = reinterpret_cast<const VkExportMemoryAllocateInfo*>(createInfo);
						
						if (exportInfo->handleTypes != VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT)
						{
							UNIMPLEMENTED("exportInfo->handleTypes");
						}
						exportFd = true;
					}
					break;
					
				default:
					UNIMPLEMENTED("createInfo->sType");
				}
				createInfo = createInfo->pNext;
			}
		}
	};

	static size_t ComputeRequiredAllocationSize(const VkMemoryAllocateInfo* pCreateInfo)
	{
		CreateInfo info(pCreateInfo);
		return (info.importFd || info.exportFd) ? sizeof(External) : 0u;
	}

	explicit External(const VkMemoryAllocateInfo* pCreateInfo)
	{
		CreateInfo info(pCreateInfo);

		static int counter = 0;
		if (info.importFd)
		{
			memfd.importFd(info.fd);
		}
		else
		{
			ASSERT(info.exportFd);
			char name[40];
			snprintf(name, sizeof(name), "SwiftShader.Memory.%d", ++counter);
			if (!memfd.allocate(name, pCreateInfo->allocationSize))
			{
				ABORT("memfd.allocate() returned %s", strerror(errno));
			}
		}
	}

	int exportFd() const {
		ASSERT(memfd.isValid());
		return memfd.exportFd();
	}

	~External()
	{
		memfd.close();
	}

	void* map(size_t size)
	{
		void* addr = ::mmap(nullptr, size, PROT_READ|PROT_WRITE, MAP_SHARED, memfd.fd, 0);
		if (addr == MAP_FAILED)
		{
			ABORT("mmap() failed with: %s", strerror(errno));
		}
		return addr;
	}

	void unmap(void* buffer, size_t size)
	{
		::munmap(buffer, size);
	}

private:
	MemFdLinux memfd;
};

int DeviceMemory::exportFd() const
{
	return external->exportFd();
}

}  // namespace vk
