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

#ifndef VK_BUFFER_COLLECTION_FUCHSIA_HPP_
#define VK_BUFFER_COLLECTION_FUCHSIA_HPP_

#include <fuchsia/sysmem/cpp/fidl.h>

#include "VkObject.hpp"

// Implementation of the VkBufferCollectionFUCHSIA handle type used by
// the VK_FUCHSIA_buffer_collection extension.

// Since there is no official documentation for this extension yet, here is
// a technical note explaining how everything is supposed to work. Keep in mind
// that the extension's implementation is bound to change in the future until
// it becomes final.
//
//  - A Fuchsia "buffer collection" is similar to a Vulkan swapchain image
//    array, with the following important points:
//
//     o It can be shared between different processes. For example, a video
//       decoder might want to write into its buffers, another process could
//       appply some sort of filter to them, and finally the compositor might
//       want to send it to the final display server (as part of a stack of
//       composited layers) without accessing its pixel storage.
//
//     o The exact format of pixels within these buffers is determined by a
//       small negociation protocol between all users of the buffer collection:
//
//         * A "buffer collection token" is created by an initiator process,
//           which then passes duplicated handles to it to other participant
//           processes.
//
//         * Each participating process associates with the token its own
//           set of constraints, matching its own usage for the collection.
//           For example, a video decoder might want to ensure that a specific
//           pixel format is used, or that the memory is "protected", i.e.
//           not directly mappable to host memory (only possible if the decoder
//           is implemented in hardware, with special wiring, of course).
//
//         * When all constraints have been associated with the token, the
//           latter is used to create a "buffer collection" object. This
//           operation allocates all the image/buffers in the collection, and
//           may fail if not all constraints can be achieved at the same time.
//
//         * Participants can then retrieve handles to a given buffer inside
//           the collection and work with it (for example, mapping it into
//           their address space if they need to write directly into it).
//
//  - While Fuchsia distinguishes the token and the buffer collection objects
//    through different handle types, the Vulkan extension uses a single type,
//    VkBufferCollectionFUCHSIA, to reference both. One can be created by
//    calling vkCreateBufferCollectionFUCHSIA(), which takes a zx_handle_t to
//    identify the corresponding token. Use vkDestroyBufferCollectionFUCHSIA()
//    to destroy the handle.
//
//  - vkSetBufferCollectionConstraintsFUCHSIA() and
//    vkSetBufferCollectionBufferConstraintsFUCHSIA() are also used to set
//    constraints on the token held by the VkBufferCollectionFUCHSIA handle.
//    This should happen before the collection images are allocated.
//
//  - Allocating the buffer collection from the token happens outside of
//    Vulkan. Once it has succeeded, the same VkBufferCollectionFUCHSIA handles
//    are used to refer to the collection.
//
//  - vkGetBufferCollectionPropertiesFUCHSIA() can be used to retrieve
//    properties of the buffer collection once it has been allocated.
//    The most important one being |count|, which defines the number of
//    buffer/images in the collection.
//
//  - To render into a buffer collection image, call vkCreateImage(), passing
//    a VkBufferCollectionImageCreateInfoFUCHSIA instance in its pNext chain.
//    This will take a VkBufferCollectionFUCHSIA handle, and an image index.
//
//    One must also bind it to a VkDeviceMemory instance which has been created
//    with a VkImportMemoryBufferCollectionFUCHSIA extension in its pNext chain.
//    Said extension must use the same VkBufferCollectionFUCHSIA handle and
//    index for the bind to succeed.
//
//    An alternative is to call vkCreateImage() but use a
//    VkFuchsiaImageFormatFUCHSIA pNext extension instead. |imageFormat| should
//    point of |imageFormatSize| bytes of a FIDL-encoded
//    fuchsia.sysmem.SingleBufferSetting struct (useful when one wants to
//    share settings between different processes).
//
//  - Similarly for buffers, call vkCreateBuffer() with a
//    VkBufferCollectionImageCreateInfoFUCHSIA instance in its pNext chain,
//    and bind it to a VkDeviceMemory instance which has been created with
//    a VkImportMemoryBufferCollectionFUCHSIA extension.
//
// Note that creating Vulkan images and buffers on top of buffer collection
// images is similar to Vulkan external memory, except that one cannot export
// such an image to another Vulkan instance (at least for now).
//
// Also note that allocating the collection images/buffers is not part of
// the Vulkan extension, nor how different participants can synchronize
// about which collection indices to use to address a specific image/buffer.
//
// Implementing this extension requires modifications to the internals of
// the following classes (at least):
//
//    VkImage
//    VkBuffer
//    VkDeviceMemory
//

namespace vk
{

class BufferCollectionFUCHSIA : public Object<BufferCollectionFUCHSIA, VkBufferCollectionFUCHSIA>
{
public:
	BufferCollectionFUCHSIA(const VkBufferCollectionCreateInfoFUCHSIA* pCreateInfo, void* mem);

	static size_t ComputeRequiredAllocationSize(const VkBufferCollectionCreateInfoFUCHSIA* pCreateInfo);

	void destroy(const VkAllocationCallbacks* pAllocator);

	VkResult init(const VkBufferCollectionCreateInfoFUCHSIA* pCreateInfo);

	VkResult setConstraints(const VkImageCreateInfo* pImageInfo);

	VkResult setBufferConstraints(const VkBufferConstraintsInfoFUCHSIA* pBufferConstraintsInfo);

	VkResult getProperties(VkDevice device, VkBufferCollectionPropertiesFUCHSIA* pProperties) const;

private:
	fuchsia::sysmem::AllocatorSyncPtr&       sysmemAllocator;
	fuchsia::sysmem::BufferCollectionSyncPtr sysmemCollection;
};

static inline BufferCollectionFUCHSIA* Cast(VkBufferCollectionFUCHSIA object)
{
	return BufferCollectionFUCHSIA::Cast(object);
}

}  // namespace vk

#endif  // VK_BUFFER_COLLECTION_FUCHSIA_HPP_
