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

#include "Vulkan/VkBuffer.hpp"
#include "Vulkan/VkBufferView.hpp"
#include "Vulkan/VkCommandBuffer.hpp"
#include "Vulkan/VkCommandPool.hpp"
#include "Vulkan/VkDevice.hpp"
#include "Vulkan/VkDeviceMemory.hpp"
#include "Vulkan/VkEvent.hpp"
#include "Vulkan/VkFence.hpp"
#include "Vulkan/VkFramebuffer.hpp"
#include "Vulkan/VkImage.hpp"
#include "Vulkan/VkImageView.hpp"
#include "Vulkan/VkInstance.hpp"
#include "Vulkan/VkPipeline.hpp"
#include "Vulkan/VkPipelineCache.hpp"
#include "Vulkan/VkPipelineLayout.hpp"
#include "Vulkan/VkPhysicalDevice.hpp"
#include "Vulkan/VkQueryPool.hpp"
#include "Vulkan/VkQueue.hpp"
#include "Vulkan/VkSampler.hpp"
#include "Vulkan/VkSemaphore.hpp"
#include "Vulkan/VkShaderModule.hpp"
#include "Vulkan/VkRenderPass.hpp"
#include "WSI/VkSurfaceKHR.hpp"
#include "WSI/VkSwapchainKHR.hpp"

#include <type_traits>

namespace vk
{

// Because Vulkan uses optional allocation callbacks, we use them in a custom
// placement new operator in the VkObjectBase class for simplicity.
// Unfortunately, since we use a placement new to allocate VkObjectBase derived
// classes objects, the corresponding deletion operator is a placement delete,
// which does nothing. In order to properly dispose of these objects' memory,
// we use this function, which calls the T:destroy() function then the T
// destructor prior to releasing the object (by default,
// VkObjectBase::destroy does nothing).
template<typename VkT>
inline void destroy(VkT vkObject, const VkAllocationCallbacks* pAllocator)
{
	auto object = Cast(vkObject);
	if(object)
	{
		using T = typename std::remove_pointer<decltype(object)>::type;
		object->destroy(pAllocator);
		object->~T();
		// object may not point to the same pointer as vkObject, for dispatchable objects,
		// for example, so make sure to deallocate based on the vkObject pointer, which
		// should always point to the beginning of the allocated memory
		vk::deallocate(vkObject, pAllocator);
	}
}

}
