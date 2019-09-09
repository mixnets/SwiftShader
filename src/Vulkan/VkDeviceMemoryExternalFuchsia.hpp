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

#include <zircon/process.h>
#include <zircon/syscalls.h>

namespace vk
{

class DeviceMemory::External {
public:
	struct CreateInfo {
		bool importHandle = false;
		bool exportHandle = false;
		zx_handle_t handle = ZX_HANDLE_INVALID;

		CreateInfo() = default;

		// Parse the VkMemoryAllocateInfo->pNext chain to initialize a CreateInfo.
		CreateInfo(const VkMemoryAllocateInfo* pCreateInfo)
		{
			const auto* createInfo = reinterpret_cast<const VkBaseInStructure*>(pCreateInfo->pNext);
			while (createInfo)
			{
				switch (createInfo->sType)
				{
				case VK_STRUCTURE_TYPE_TEMP_IMPORT_MEMORY_ZIRCON_HANDLE_INFO_FUCHSIA:
					{
						const auto* importInfo = reinterpret_cast<const VkImportMemoryZirconHandleInfoFUCHSIA*>(createInfo);

						if (importInfo->handleType != VK_EXTERNAL_MEMORY_HANDLE_TYPE_TEMP_ZIRCON_VMO_BIT_FUCHSIA)
						{
							UNIMPLEMENTED("importInfo->handleType");
						}
						importHandle = true;
						handle = importInfo->handle;
					}
					break;
				case VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO:
					{
						const auto* exportInfo = reinterpret_cast<const VkExportMemoryAllocateInfo*>(createInfo);

						if (exportInfo->handleTypes != VK_EXTERNAL_MEMORY_HANDLE_TYPE_TEMP_ZIRCON_VMO_BIT_FUCHSIA)
						{
							UNIMPLEMENTED("exportInfo->handleTypes");
						}
						exportHandle = true;
					}
					break;

				default:
					;
				}
				createInfo = createInfo->pNext;
			}
		}
	};

	static size_t ComputeRequiredAllocationSize(const VkMemoryAllocateInfo* pCreateInfo)
	{
		CreateInfo info(pCreateInfo);
		return (info.importHandle || info.exportHandle) ? sizeof(External) : 0u;
	}

	explicit External(const VkMemoryAllocateInfo* pCreateInfo)
	{
		CreateInfo info(pCreateInfo);

		if (info.importHandle)
		{
			zx_status_t status = zx_handle_duplicate(info.handle, ZX_RIGHT_SAME_RIGHTS, &vmo_handle);
			if (status != ZX_OK)
			{
				ABORT("zx_handle_duplicate() returned %d", status);
			}
		}
		else
		{
			ASSERT(info.exportHandle);
			zx_status_t status = zx_vmo_create(pCreateInfo->allocationSize, 0, &vmo_handle);
			if (status != ZX_OK)
			{
				ABORT("zx_vmo_create() returned %d", status);
			}
		}
	}

	int exportHandle() const {
		ASSERT(vmo_handle != ZX_HANDLE_INVALID);
		zx_handle_t result = ZX_HANDLE_INVALID;
		zx_status_t status = zx_handle_duplicate(vmo_handle, ZX_RIGHT_SAME_RIGHTS, &result);
		if (status != ZX_OK)
		{
			ABORT("zx_handle_duplicate() returned %d", status);
		}
		return result;
	}

	~External()
	{
		if (vmo_handle != ZX_HANDLE_INVALID)
		{
			zx_handle_close(vmo_handle);
		}
	}

	void* map(size_t size)
	{
		zx_vaddr_t addr = 0;
		zx_status_t status = zx_vmar_map(zx_vmar_root_self(),
										 ZX_VM_PERM_READ|ZX_VM_PERM_WRITE,
								         0,  // vmar_offset
								         vmo_handle,
										 0,  // vmo_offset
										 size,
										 &addr);
		if (status != ZX_OK)
		{
			ABORT("zx_vmar_map() failed with %d", status);
		}
		return reinterpret_cast<void*>(addr);
	}

	void unmap(void* buffer, size_t size)
	{
		zx_status_t status = zx_vmar_unmap(zx_vmar_root_self(),
										   reinterpret_cast<zx_vaddr_t>(buffer),
										   size);
		if (status != ZX_OK)
		{
			ABORT("zx_vmar_unmap() failed with %d", status);
		}
	}

private:
	zx_handle_t vmo_handle = ZX_HANDLE_INVALID;
};

zx_handle_t DeviceMemory::exportHandle() const
{
	return external->exportHandle();
}

bool DeviceMemory::checkExternalMemoryHandleType(
		VkExternalMemoryHandleTypeFlags supportedHandleTypes) const {
	if (external != nullptr) {
		// Since this is an external memory handle, VkCreate{Image,Buffer}()
		// should have specified a compatible handle type in a
		// VkExternalMemory{Image,Buffer}CreateInfo struct.
		return (supportedHandleTypes & VK_EXTERNAL_MEMORY_HANDLE_TYPE_TEMP_ZIRCON_VMO_BIT_FUCHSIA) != 0;
	}
	// Otherwise non-external device memory is compatible with all buffers and images.
	return true;
}

}  // namespace vk
