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

#include "VkSemaphore.hpp"

namespace vk
{

Semaphore::Semaphore(const VkSemaphoreCreateInfo* pCreateInfo, void* mem) {}

Semaphore::~Semaphore() {}

size_t Semaphore::ComputeRequiredAllocationSize(const VkSemaphoreCreateInfo* pCreateInfo)
{
    return 0;
}

void Semaphore::wait()
{
    // Semaphores are noop for now
}

void Semaphore::wait(const VkPipelineStageFlags& flags)
{
    // VkPipelineStageFlags is the pipeline stage at which the semaphore wait will occur
    wait()
}

void Semaphore::signal()
{
    // Semaphores are noop for now
}

}  // namespace vk
