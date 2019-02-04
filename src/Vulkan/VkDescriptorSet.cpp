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

#include "VkDescriptorSet.hpp"
#include "VkDescriptorSetLayout.hpp"

#include <Vulkan/VkDebug.hpp>

namespace vk
{

DescriptorSet::DescriptorSet(VkDescriptorSetLayout layout)
{
    header.layout = Cast(layout);
}

void DescriptorSet::Write(const VkWriteDescriptorSet& data)
{
    auto layout = Cast(data.dstSet)->header.layout;
    auto base = reinterpret_cast<uint8_t*>(data.dstSet) +
        layout->bindingOffset(data.descriptorType, data.dstBinding);

    switch (data.descriptorType)
    {
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
    {
        for (uint32_t i = 0; i < data.descriptorCount; i++) {
            auto& binding = reinterpret_cast<StorageBuffer*>(base)[i];
            binding.bufferInfo = *data.pBufferInfo;
        }
        break;
    }
    default:
        UNIMPLEMENTED();
    }
}

void DescriptorSet::Copy(const VkCopyDescriptorSet& data)
{
    UNIMPLEMENTED();
}

} // namespace vk