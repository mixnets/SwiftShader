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

#include "VkBufferCollectionFUCHSIA.hpp"

#include <zircon/process.h>
#include <zircon/syscalls.h>

namespace fuchsia
{

// A fake external memory type used to model a memory buffer that is allocated
// through a Fuchsia buffer collection token.
class BufferCollectionMemory : public vk::DeviceMemory::Impl
{
public:
	struct ImportInfo {
		bool                      found = false;
		VkBufferCollectionFUCHSIA collection;
		uint32_t                  index;

		ImportInfo(const VkMemoryAllocateInfo* pAllocateInfo)
		{
			const auto* extInfo = reinterpret_cast<const VkBaseInStructure*>(pAllocateInfo->pNext);
			for (;extInfo != nullptr; extInfo = extInfo->pNext)
			{
				if (extInfo->sType == VK_STRUCTURE_TYPE_IMPORT_MEMORY_BUFFER_COLLECTION_FUCHSIA)
				{
					const auto* importInfo = reinterpret_cast<const VkImportMemoryBufferCollectionFUCHSIA *>(extInfo);

					found      = true;
					collection = importInfo->collection;
					index      = importInfo->index;
					break;
				}
			}
		}
	};
	
	static const VkExternalMemoryHandleTypeFlagBits typeFlagBit = (VkExternalMemoryHandleTypeFlagBits)0;

	VkExternalMemoryHandleTypeFlagBits getFlagBit() const override {
		return typeFlagBit;
	}

	static bool supportsAllocateInfo(const VkMemoryAllocateInfo* pAllocateInfo)
	{
		ImportInfo info(pAllocateInfo);
		return info.found;
	}

	explicit BufferCollectionMemory(const VkMemoryAllocateInfo* pAllocateInfo)
		: info(pAllocateInfo)
	{
	}

	~BufferCollectionMemory()
	{
		// TODO(digit)
	}

	VkResult allocate(size_t size, void** pBuffer) override
	{
		// TODO(digit)
		return VK_ERROR_MEMORY_MAP_FAILED;
	}

	void deallocate(void* buffer, size_t size) override
	{
		// TODO(digit)
	}

	VkResult checkBufferCollection(VkBufferCollectionFUCHSIA collection,
								   uint32_t                  index) const override
	{
		if (info.found && collection == info.collection && index == info.index)
			return VK_SUCCESS;

		return VK_ERROR_INITIALIZATION_FAILED;
	}

private:
	ImportInfo info;
	// TODO(digit): Get VMO to map/unmap it in the current address space?
};

}  // namespace fuchsia
