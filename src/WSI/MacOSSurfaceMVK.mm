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

#include "MacOSSurfaceMVK.h"
#include "Vulkan/VkDeviceMemory.hpp"
#include "Vulkan/VkImage.hpp"

#include <Metal/Metal.h>
#include <QuartzCore/CAMetalLayer.h>
#include <AppKit/NSView.h>

namespace vk {

MacOSSurfaceMVK::MacOSSurfaceMVK(const VkMacOSSurfaceCreateInfoMVK *pCreateInfo, void *mem)
{
    id<NSObject> obj = (id<NSObject>)pCreateInfo->pView;

    if([obj isKindOfClass: [NSView class]])
    {
        if(!NSThread.isMainThread)
        {
            UNREACHABLE("vkCreateMacOSSurfaceMVK(): not called from main thread");
        }
        view = (NSView*)[obj retain];

        obj = view.layer;
        if ([obj isKindOfClass: [CAMetalLayer class]])
        {
            metalLayer = (CAMetalLayer*)[obj retain];
        }
        else
        {
            UNREACHABLE("vkCreateMacOSSurfaceMVK(): view doesn't have metal backed layer");
        }
    }
}

void MacOSSurfaceMVK::destroySurface(const VkAllocationCallbacks *pAllocator)
{
    if(metalLayer)
    {
        [metalLayer release];
    }

    if(view)
    {
        [view release];
    }
}

size_t MacOSSurfaceMVK::ComputeRequiredAllocationSize(const VkMacOSSurfaceCreateInfoMVK *pCreateInfo)
{
    return 0;
}

void MacOSSurfaceMVK::getSurfaceCapabilities(VkSurfaceCapabilitiesKHR *pSurfaceCapabilities) const
{
    SurfaceKHR::getSurfaceCapabilities(pSurfaceCapabilities);

    CGSize drawSize = metalLayer.bounds.size;
    CGFloat scaleFactor = metalLayer.contentsScale;
    drawSize.width = trunc(drawSize.width * scaleFactor);
    drawSize.height = trunc(drawSize.height * scaleFactor);
    VkExtent2D extent =
    {
        static_cast<uint32_t>(drawSize.width),
        static_cast<uint32_t>(drawSize.height)
    };

    pSurfaceCapabilities->currentExtent = extent;
    pSurfaceCapabilities->minImageExtent = extent;
    pSurfaceCapabilities->maxImageExtent = extent;
}

void MacOSSurfaceMVK::present(PresentImage* image)
{
    if(metalLayer)
    {
        auto drawable = [metalLayer nextDrawable];
        if(drawable)
        {
            VkExtent3D extent = image->getImage()->getMipLevelExtent(VK_IMAGE_ASPECT_COLOR_BIT, 0);
            [drawable.texture replaceRegion:MTLRegionMake2D(0, 0, extent.width, extent.height)
                              mipmapLevel:0
                              withBytes:image->getImageMemory()->getOffsetPointer(0)
                              bytesPerRow:image->getImage()->rowPitchBytes(VK_IMAGE_ASPECT_COLOR_BIT, 0)];
            [drawable present];
        }
    }
}

}
