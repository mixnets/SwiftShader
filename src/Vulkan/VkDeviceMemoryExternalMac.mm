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

#include "VkDeviceMemoryExternalMac.hpp"

#include <IOSurface/IOSurface.h>

namespace vk
{

IOSurfaceExternalMemory::IOSurfaceExternalMemory(const VkMemoryAllocateInfo *pAllocateInfo)
{
    if (pAllocateInfo->pNext != 0)
    {
        const VkIOSurfaceMemoryInfo *ioSurfaceMemoryInfoPtr =
            reinterpret_cast<const VkIOSurfaceMemoryInfo *>(pAllocateInfo->pNext);
        if (ioSurfaceMemoryInfoPtr->sType == VK_STRUCTURE_TYPE_IOSURFACE_MEMORY_INFO)
        {
            ioSurface = reinterpret_cast<IOSurfaceRef>(ioSurfaceMemoryInfoPtr->pIOSurfaceRef);
            size      = ioSurfaceMemoryInfoPtr->size;
            plane     = ioSurfaceMemoryInfoPtr->plane;
        }
    }
}

void IOSurfaceExternalMemory::deallocate(void *buffer, size_t size) {}

VkResult IOSurfaceExternalMemory::allocate(size_t size, void **pBuffer)
{
    if (!ioSurface)
    {
        return VK_ERROR_OUT_OF_DEVICE_MEMORY;
    }
    *pBuffer = IOSurfaceGetBaseAddressOfPlane(ioSurface, plane);
    return VK_SUCCESS;
}

VkExternalMemoryHandleTypeFlagBits IOSurfaceExternalMemory::getFlagBit() const
{
    return typeFlagBit;
}

void IOSurfaceExternalMemory::lock() const
{
    IOSurfaceLock(ioSurface, 0, nullptr);
}

void IOSurfaceExternalMemory::unlock() const
{
    IOSurfaceUnlock(ioSurface, 0, nullptr);
}

bool IOSurfaceExternalMemory::supportsAllocateInfo(const VkMemoryAllocateInfo *pAllocateInfo)
{
    if (pAllocateInfo->pNext != 0)
    {
        const VkIOSurfaceMemoryInfo *ioSurfaceMemoryInfoPtr =
            reinterpret_cast<const VkIOSurfaceMemoryInfo *>(pAllocateInfo->pNext);
        if (ioSurfaceMemoryInfoPtr->sType == VK_STRUCTURE_TYPE_IOSURFACE_MEMORY_INFO)
        {
            return true;
        }
    }
    return false;
}

}  // namespace vk
