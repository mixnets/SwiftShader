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

#include <Vulkan/VkDebug.hpp>

namespace vk
{
namespace ds
{

void write(const VkWriteDescriptorSet& data)
{
    switch (data.descriptorType)
    {
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
    {
        auto set = reinterpret_cast<StorageBuffer*>(data.dstSet);
        set->bufferInfo = *data.pBufferInfo;
        break;
    }
    default:
        UNIMPLEMENTED();
    }
}

void copy(const VkCopyDescriptorSet& data)
{
    UNIMPLEMENTED();
}

} // namespace ds
} // namespace vk