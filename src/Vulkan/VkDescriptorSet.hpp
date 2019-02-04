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

#ifndef VK_DESCRIPTOR_SET_HPP_
#define VK_DESCRIPTOR_SET_HPP_

#include <vulkan/vulkan.h>

namespace vk
{

class DescriptorSetLayout;

class DescriptorSet
{
public:
    static void Write(const VkWriteDescriptorSet&);
    static void Copy(const VkCopyDescriptorSet&);

    struct Header
    {
        DescriptorSetLayout* layout;
    };

    struct Sampler
    {
        void* _; // // FIXME(b/123244275) : Compute actual required size for each desciptor type
    };

    struct CombinedImageSampler
    {
        void* _; // // FIXME(b/123244275) : Compute actual required size for each desciptor type
    };

    struct SampledImage
    {
        void* _; // // FIXME(b/123244275) : Compute actual required size for each desciptor type
    };

    struct StorageImage
    {
        void* _; // // FIXME(b/123244275) : Compute actual required size for each desciptor type
    };

    struct UniformTexelBuffer
    {
        void* _; // // FIXME(b/123244275) : Compute actual required size for each desciptor type
    };

    struct StorageTexelBuffer
    {
        void* _; // // FIXME(b/123244275) : Compute actual required size for each desciptor type
    };

    struct UniformBuffer
    {
        void* _; // // FIXME(b/123244275) : Compute actual required size for each desciptor type
    };

    struct StorageBuffer
    {
        VkDescriptorBufferInfo bufferInfo;
    };

    struct UniformBufferDynamic
    {
        void* _; // // FIXME(b/123244275) : Compute actual required size for each desciptor type
    };

    struct StorageBufferDynamic
    {
        void* _; // // FIXME(b/123244275) : Compute actual required size for each desciptor type
    };

    struct InputAttachment
    {
        void* _; // // FIXME(b/123244275) : Compute actual required size for each desciptor type
    };

    DescriptorSet(VkDescriptorSetLayout);

    Header header;
    // Descriptors arrays live below.
};

static inline DescriptorSet* Cast(VkDescriptorSet object)
{
	return reinterpret_cast<DescriptorSet*>(object);
}

} // namespace vk

#endif // VK_DESCRIPTOR_SET_HPP_
