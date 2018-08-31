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

#include "VkBuffer.hpp"
#include "VkBufferView.hpp"
#include "VkCommandPool.hpp"
#include "VkConfig.h"
#include "VkDebug.hpp"
#include "VkDescriptorPool.hpp"
#include "VkDescriptorSetLayout.hpp"
#include "VkDevice.hpp"
#include "VkDeviceMemory.hpp"
#include "VkEvent.hpp"
#include "VkFence.hpp"
#include "VkFramebuffer.hpp"
#include "VkGetProcAddress.h"
#include "VkImage.hpp"
#include "VkImageView.hpp"
#include "VkInstance.hpp"
#include "VkPhysicalDevice.hpp"
#include "VkPipeline.hpp"
#include "VkPipelineCache.hpp"
#include "VkPipelineLayout.hpp"
#include "VkQueryPool.hpp"
#include "VkQueue.hpp"
#include "VkSampler.hpp"
#include "VkSemaphore.hpp"
#include "VkShaderModule.hpp"
#include "VkRenderPass.hpp"
#include <cstring>
#include <cstring>

namespace vk
{

template<typename T, typename T2>
struct CountedObject
{
	typedef uint32_t (T::*getCountFunction)() const;
	typedef void (T::*getObjectFunction)(uint32_t, T2*) const;

	static void get(T* object, uint32_t* count, T2* output, getCountFunction getCount, getObjectFunction getObject)
	{
		if(!output)
		{
			*count = (object->*getCount)();
		}
		else
		{
			(object->*getObject)(*count, output);
		}
	}
};

}

extern "C"
{
VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vk_icdGetInstanceProcAddr(VkInstance instance, const char* pName)
{
	return vk::GetProcAddr(pName);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance)
{
	TRACE("(const VkInstanceCreateInfo* pCreateInfo = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X, VkInstance* pInstance = 0x%X)",
			pCreateInfo, pAllocator, pInstance);

	if(pCreateInfo->enabledLayerCount)
	{
		UNIMPLEMENTED();
	}

	if(pCreateInfo->enabledExtensionCount)
	{
		UNIMPLEMENTED();
	}

	if(pCreateInfo->pNext)
	{
		switch(*reinterpret_cast<const VkStructureType*>(pCreateInfo->pNext))
		{
		case VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO:
			// According to the Vulkan spec, section 2.7.2. Implicit Valid Usage:
			// "The values VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO and
			//  VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO are reserved for
			//  internal use by the loader, and do not have corresponding
			//  Vulkan structures in this Specification."
			break;
		default:
			UNIMPLEMENTED();
		}
	}

	return vk::CheckAndSet(new (pAllocator) vk::Instance(pAllocator), pInstance, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator)
{
	TRACE("(VkInstance instance = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X)", instance, pAllocator);

	vk::destroy(instance, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkEnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount, VkPhysicalDevice* pPhysicalDevices)
{
	TRACE("(VkInstance instance = 0x%X, uint32_t* pPhysicalDeviceCount = 0x%X, VkPhysicalDevice* pPhysicalDevices = 0x%X)",
		    instance, pPhysicalDeviceCount, pPhysicalDevices);

	vk::CountedObject<vk::Instance, VkPhysicalDevice>::get(
		vk::Cast(instance), pPhysicalDeviceCount, pPhysicalDevices,
		&(vk::Instance::getPhysicalDeviceCount), &(vk::Instance::getPhysicalDevices));

	return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures* pFeatures)
{
	TRACE("(VkPhysicalDevice physicalDevice = 0x%X, VkPhysicalDeviceFeatures* pFeatures = 0x%X)",
			physicalDevice, pFeatures);

	*pFeatures = vk::Cast(physicalDevice)->getFeatures();
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties* pFormatProperties)
{
	TRACE("GetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice = 0x%X, VkFormat format = %d, VkFormatProperties* pFormatProperties = 0x%X)",
			physicalDevice, (int)format, pFormatProperties);

	vk::Cast(physicalDevice)->getFormatProperties(format, pFormatProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags, VkImageFormatProperties* pImageFormatProperties)
{
	TRACE("(VkPhysicalDevice physicalDevice = 0x%X, VkFormat format = %d, VkImageType type = %d, VkImageTiling tiling = %d, VkImageUsageFlags usage = %d, VkImageCreateFlags flags = %d, VkImageFormatProperties* pImageFormatProperties = 0x%X)",
			physicalDevice, (int)format, (int)type, (int)tiling, usage, flags, pImageFormatProperties);

	vk::Cast(physicalDevice)->getImageFormatProperties(format, type, tiling, usage, flags, pImageFormatProperties);

	return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties* pProperties)
{
	TRACE("(VkPhysicalDevice physicalDevice = 0x%X, VkPhysicalDeviceProperties* pProperties = 0x%X)",
		    physicalDevice, pProperties);

	*pProperties = vk::Cast(physicalDevice)->getProperties();
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties* pQueueFamilyProperties)
{
	TRACE("(VkPhysicalDevice physicalDevice = 0x%X, uint32_t* pQueueFamilyPropertyCount = 0x%X, VkQueueFamilyProperties* pQueueFamilyProperties = 0x%X))", physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);

	vk::CountedObject<vk::PhysicalDevice, VkQueueFamilyProperties>::get(
		vk::Cast(physicalDevice), pQueueFamilyPropertyCount, pQueueFamilyProperties,
		&(vk::PhysicalDevice::getQueueFamilyPropertyCount), &(vk::PhysicalDevice::getQueueFamilyProperties));
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties* pMemoryProperties)
{
	TRACE("(VkPhysicalDevice physicalDevice = 0x%X, VkPhysicalDeviceMemoryProperties* pMemoryProperties = 0x%X)", physicalDevice, pMemoryProperties);

	*pMemoryProperties = vk::Cast(physicalDevice)->getMemoryProperties();
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetInstanceProcAddr(VkInstance instance, const char* pName)
{
	TRACE("(VkInstance instance = 0x%X, const char* pName = 0x%X)", instance, pName);
	return vk::GetProcAddr(pName);
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetDeviceProcAddr(VkDevice device, const char* pName)
{
	TRACE("(VkDevice device = 0x%X, const char* pName = 0x%X)", device, pName);
	return vk::GetProcAddr(pName);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice)
{
	TRACE("(VkPhysicalDevice physicalDevice = 0x%X, const VkDeviceCreateInfo* pCreateInfo = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X, VkDevice* pDevice = 0x%X)",
		physicalDevice, pCreateInfo, pAllocator, pDevice);

	if(pCreateInfo->enabledLayerCount || pCreateInfo->enabledExtensionCount)
	{
		UNIMPLEMENTED();
	}

	if(pCreateInfo->pNext)
	{
		switch(*reinterpret_cast<const VkStructureType*>(pCreateInfo->pNext))
		{
		case VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO:
			// According to the Vulkan spec, section 2.7.2. Implicit Valid Usage:
			// "The values VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO and
			//  VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO are reserved for
			//  internal use by the loader, and do not have corresponding
			//  Vulkan structures in this Specification."
			break;
		default:
			UNIMPLEMENTED();
		}
	}

	if(pCreateInfo->pEnabledFeatures &&
	   !vk::Cast(physicalDevice)->hasFeatures(*(pCreateInfo->pEnabledFeatures)))
	{
		return VK_ERROR_FEATURE_NOT_PRESENT;
	}

	uint32_t queueFamilyPropertyCount = vk::Cast(physicalDevice)->getQueueFamilyPropertyCount();

	for(uint32_t i = 0; i < pCreateInfo->queueCreateInfoCount; i++)
	{
		const VkDeviceQueueCreateInfo& queueCreateInfo = pCreateInfo->pQueueCreateInfos[i];
		if(queueCreateInfo.pNext || queueCreateInfo.flags)
		{
			UNIMPLEMENTED();
		}
	}

	return vk::CheckAndSet(new (pAllocator) vk::Device(pAllocator, physicalDevice, pCreateInfo), pDevice, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator)
{
	TRACE("(VkDevice device = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X)", device, pAllocator);

	vk::destroy(device, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateInstanceExtensionProperties(const char* pLayerName, uint32_t* pPropertyCount, VkExtensionProperties* pProperties)
{
	TRACE("(const char* pLayerName = 0x%X, uint32_t* pPropertyCount = 0x%X, VkExtensionProperties* pProperties = 0x%X)",
	      pLayerName, pPropertyCount, pProperties);

	static const char *extensions[] =
	{
		"VK_KHR_16bit_storage",
		"VK_KHR_bind_memory2",
		"VK_KHR_dedicated_allocation",
		"VK_KHR_descriptor_update_template",
		"VK_KHR_device_group",
		"VK_KHR_device_group_creation",
		"VK_KHR_external_fence",
		"VK_KHR_external_fence_capabilities",
		"VK_KHR_external_memory",
		"VK_KHR_external_memory_capabilities",
		"VK_KHR_external_semaphore",
		"VK_KHR_external_semaphore_capabilities",
		"VK_KHR_get_memory_requirements2",
		"VK_KHR_get_physical_device_properties2",
		"VK_KHR_maintenance1",
		"VK_KHR_maintenance2",
		"VK_KHR_maintenance3",
		"VK_KHR_multiview",
		"VK_KHR_relaxed_block_layout",
		"VK_KHR_sampler_ycbcr_conversion",
		"VK_KHR_shader_draw_parameters",
		"VK_KHR_storage_buffer_storage_class",
		"VK_KHR_variable_pointers",
	};

	if(!pProperties)
	{
		*pPropertyCount = sizeof(extensions) / sizeof(extensions[0]);
		return VK_SUCCESS;
	}

	uint32_t apiVersion = 0;
	VkResult result = vkEnumerateInstanceVersion(&apiVersion);
	if(result != VK_SUCCESS)
	{
		return result;
	}

	for(uint32_t i = 0; i < *pPropertyCount; i++)
	{
		size_t len = strlen(extensions[i]);
		memcpy(pProperties[i].extensionName, extensions[i], len);
		pProperties[i].extensionName[len] = '\0';
		pProperties[i].specVersion = apiVersion;
	}

	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char* pLayerName, uint32_t* pPropertyCount, VkExtensionProperties* pProperties)
{
	TRACE("(VkPhysicalDevice physicalDevice = 0x%X, const char* pLayerName, uint32_t* pPropertyCount = 0x%X, VkExtensionProperties* pProperties = 0x%X)", physicalDevice, pPropertyCount, pProperties);

	if(!pProperties)
	{
		*pPropertyCount = 0;
		return VK_SUCCESS;
	}

	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateInstanceLayerProperties(uint32_t* pPropertyCount, VkLayerProperties* pProperties)
{
	TRACE("(uint32_t* pPropertyCount = 0x%X, VkLayerProperties* pProperties = 0x%X)", pPropertyCount, pProperties);

	if(!pProperties)
	{
		*pPropertyCount = 0;
		return VK_SUCCESS;
	}

	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkLayerProperties* pProperties)
{
	TRACE("(VkPhysicalDevice physicalDevice = 0x%X, uint32_t* pPropertyCount = 0x%X, VkLayerProperties* pProperties = 0x%X)", physicalDevice, pPropertyCount, pProperties);

	if(!pProperties)
	{
		*pPropertyCount = 0;
		return VK_SUCCESS;
	}

	return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL vkGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue)
{
	TRACE("(VkDevice device = 0x%X, uint32_t queueFamilyIndex = %d, uint32_t queueIndex = %d, VkQueue* pQueue = 0x%X)",
		    device, queueFamilyIndex, queueIndex, pQueue);

	*pQueue = vk::Cast(device)->getQueue(queueFamilyIndex, queueIndex);
}

VKAPI_ATTR VkResult VKAPI_CALL vkQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence)
{
	TRACE("(VkQueue queue = 0x%X, uint32_t submitCount = %d, const VkSubmitInfo* pSubmits = 0x%X, VkFence fence = 0x%X)",
	      queue, submitCount, pSubmits, fence);

	vk::Cast(queue)->submit(submitCount, pSubmits, fence);

	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL vkQueueWaitIdle(VkQueue queue)
{
	TRACE("(VkQueue queue = 0x%X)", queue);

	vk::Cast(queue)->waitIdle();

	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL vkDeviceWaitIdle(VkDevice device)
{
	TRACE("(VkDevice device = 0x%X)", device);

	vk::Cast(device)->waitIdle();

	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL vkAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo, const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory)
{
	TRACE("(VkDevice device = 0x%X, const VkMemoryAllocateInfo* pAllocateInfo = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X, VkDeviceMemory* pMemory = 0x%X)",
		    device, pAllocateInfo, pAllocator, pMemory);

	if(pAllocateInfo->pNext)
	{
		UNIMPLEMENTED();
	}

	return vk::CheckAndSet(new (pAllocator) vk::DeviceMemory(pAllocateInfo), pMemory, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL vkFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator)
{
	TRACE("(VkDevice device = 0x%X, VkDeviceMemory memory = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X)",
		    device, memory, pAllocator);

	vk::destroy(memory, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** ppData)
{
	TRACE("(VkDevice device = 0x%X, VkDeviceMemory memory = 0x%X, VkDeviceSize offset = %d, VkDeviceSize size = %d, VkMemoryMapFlags flags = 0x%X, void** ppData = 0x%X)",
		    device, memory, offset, size, flags, ppData);

	*ppData = vk::Cast(memory)->map(offset, size);

	return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL vkUnmapMemory(VkDevice device, VkDeviceMemory memory)
{
	TRACE("(VkDevice device = 0x%X, VkDeviceMemory memory = 0x%X)", device, memory);

	vk::Cast(memory)->unmap();
}

VKAPI_ATTR VkResult VKAPI_CALL vkFlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount, const VkMappedMemoryRange* pMemoryRanges)
{
	TRACE("(VkDevice device = 0x%X, uint32_t memoryRangeCount = %d, const VkMappedMemoryRange* pMemoryRanges = 0x%X)",
		    device, memoryRangeCount, pMemoryRanges);

	for(uint32_t i = 0; i < memoryRangeCount; i++)
	{
		vk::Cast(pMemoryRanges[i].memory)->flush();
	}

	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL vkInvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount, const VkMappedMemoryRange* pMemoryRanges)
{
	TRACE("(VkDevice device = 0x%X, uint32_t memoryRangeCount = %d, const VkMappedMemoryRange* pMemoryRanges = 0x%X)",
		    device, memoryRangeCount, pMemoryRanges);

	for(uint32_t i = 0; i < memoryRangeCount; i++)
	{
		vk::Cast(pMemoryRanges[i].memory)->invalidate();
	}

	return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL vkGetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory, VkDeviceSize* pCommittedMemoryInBytes)
{
	TRACE("(VkDevice device = 0x%X, VkDeviceMemory memory = 0x%X, VkDeviceSize* pCommittedMemoryInBytes = 0x%X)",
	      device, memory, pCommittedMemoryInBytes);

	*pCommittedMemoryInBytes = vk::Cast(memory)->getCommittedMemoryInBytes();
}

VKAPI_ATTR VkResult VKAPI_CALL vkBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset)
{
	TRACE("(VkDevice device = 0x%X, VkBuffer buffer = 0x%X, VkDeviceMemory memory = 0x%X, VkDeviceSize memoryOffset = %d)",
		    device, buffer, memory, memoryOffset);

	vk::Cast(buffer)->bind(memory, memoryOffset);

	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL vkBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset)
{
	TRACE("(VkDevice device = 0x%X, VkImage image = 0x%X, VkDeviceMemory memory = 0x%X, VkDeviceSize memoryOffset = %d)",
		    device, image, memory, memoryOffset);

	vk::Cast(image)->bind(memory, memoryOffset);

	return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL vkGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements)
{
	TRACE("(VkDevice device = 0x%X, VkBuffer buffer = 0x%X, VkMemoryRequirements* pMemoryRequirements = 0x%X)",
		    device, buffer, pMemoryRequirements);

	*pMemoryRequirements = vk::Cast(buffer)->GetMemoryRequirements();
}

VKAPI_ATTR void VKAPI_CALL vkGetImageMemoryRequirements(VkDevice device, VkImage image, VkMemoryRequirements* pMemoryRequirements)
{
	TRACE("(VkDevice device = 0x%X, VkImage image = 0x%X, VkMemoryRequirements* pMemoryRequirements = 0x%X)",
		    device, image, pMemoryRequirements);

	*pMemoryRequirements = vk::Cast(image)->GetMemoryRequirements();
}

VKAPI_ATTR void VKAPI_CALL vkGetImageSparseMemoryRequirements(VkDevice device, VkImage image, uint32_t* pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements* pSparseMemoryRequirements)
{
	TRACE("(VkDevice device, VkImage image, uint32_t* pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements* pSparseMemoryRequirements)",
	      device, image, pSparseMemoryRequirementCount, pSparseMemoryRequirements);

	vk::Cast(device)->getImageSparseMemoryRequirements(image, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceSparseImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkSampleCountFlagBits samples, VkImageUsageFlags usage, VkImageTiling tiling, uint32_t* pPropertyCount, VkSparseImageFormatProperties* pProperties)
{
	TRACE("(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkSampleCountFlagBits samples, VkImageUsageFlags usage, VkImageTiling tiling, uint32_t* pPropertyCount, VkSparseImageFormatProperties* pProperties)",
	      physicalDevice, format, type, samples, usage, tiling, pPropertyCount, pProperties);

	vk::Cast(physicalDevice)->getSparseImageFormatProperties(format, type, samples, usage, tiling, pPropertyCount, pProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL vkQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence)
{
	TRACE("(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence)",
	      queue, bindInfoCount, pBindInfo, fence);

	vk::Cast(queue)->bindSparse(bindInfoCount, pBindInfo, fence);

	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFence* pFence)
{
	TRACE("(VkDevice device = 0x%X, const VkFenceCreateInfo* pCreateInfo = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X, VkFence* pFence = 0x%X)",
		    device, pCreateInfo, pAllocator, pFence);

	if(pCreateInfo->pNext)
	{
		UNIMPLEMENTED();
	}

	return vk::CheckAndSet(new (pAllocator) vk::Fence(pCreateInfo), pFence, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks* pAllocator)
{
	TRACE("(VkDevice device = 0x%X, VkFence fence = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X)",
		    device, fence, pAllocator);

	vk::destroy(fence, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkResetFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences)
{
	TRACE("(VkDevice device = 0x%X, uint32_t fenceCount = %d, const VkFence* pFences = 0x%X)",
	      device, fenceCount, pFences);

	for(uint32_t i = 0; i < fenceCount; i++)
	{
		vk::Cast(pFences[i])->reset();
	}

	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetFenceStatus(VkDevice device, VkFence fence)
{
	TRACE("(VkDevice device = 0x%X, VkFence fence = 0x%X)", device, fence);

	return vk::Cast(fence)->getStatus();
}

VKAPI_ATTR VkResult VKAPI_CALL vkWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll, uint64_t timeout)
{
	TRACE("(VkDevice device = 0x%X, uint32_t fenceCount = %d, const VkFence* pFences = 0x%X, VkBool32 waitAll = %d, uint64_t timeout = %d)",
	      device, fenceCount, pFences, waitAll, timeout);

	vk::Cast(device)->waitForFences(fenceCount, pFences, waitAll, timeout);

	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore)
{
	TRACE("(VkDevice device = 0x%X, const VkSemaphoreCreateInfo* pCreateInfo = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X, VkSemaphore* pSemaphore = 0x%X)",
	      device, pCreateInfo, pAllocator, pSemaphore);

	if(pCreateInfo->pNext)
	{
		UNIMPLEMENTED();
	}

	return vk::CheckAndSet(new (pAllocator) vk::Semaphore(pCreateInfo), pSemaphore, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL vkDestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator)
{
	TRACE("(VkDevice device = 0x%X, VkSemaphore semaphore = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X)",
	      device, semaphore, pAllocator);

	vk::destroy(semaphore, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateEvent(VkDevice device, const VkEventCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkEvent* pEvent)
{
	TRACE("(VkDevice device = 0x%X, const VkEventCreateInfo* pCreateInfo = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X, VkEvent* pEvent = 0x%X)",
	      device, pCreateInfo, pAllocator, pEvent);

	if(pCreateInfo->pNext)
	{
		UNIMPLEMENTED();
	}

	return vk::CheckAndSet(new (pAllocator) vk::Event(pCreateInfo), pEvent, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks* pAllocator)
{
	TRACE("(VkDevice device = 0x%X, VkEvent event = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X)",
	      device, event, pAllocator);

	vk::destroy(event, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetEventStatus(VkDevice device, VkEvent event)
{
	TRACE("(VkDevice device = 0x%X, VkEvent event = 0x%X)", device, event);

	return vk::Cast(event)->getStatus();
}

VKAPI_ATTR VkResult VKAPI_CALL vkSetEvent(VkDevice device, VkEvent event)
{
	TRACE("(VkDevice device = 0x%X, VkEvent event = 0x%X)", device, event);

	vk::Cast(event)->signal();

	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL vkResetEvent(VkDevice device, VkEvent event)
{
	TRACE("(VkDevice device = 0x%X, VkEvent event = 0x%X)", device, event);
	
	vk::Cast(event)->reset();

	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool)
{
	TRACE("(VkDevice device = 0x%X, const VkQueryPoolCreateInfo* pCreateInfo = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X, VkQueryPool* pQueryPool = 0x%X)",
	      device, pCreateInfo, pAllocator, pQueryPool);

	if(pCreateInfo->pNext || pCreateInfo->flags)
	{
		UNIMPLEMENTED();
	}

	return vk::CheckAndSet(new (pAllocator) vk::QueryPool(pCreateInfo), pQueryPool, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator)
{
	TRACE("(VkDevice device = 0x%X, VkQueryPool queryPool = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X)",
	      device, queryPool, pAllocator);

	vk::destroy(queryPool, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL 
vkGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, size_t dataSize, void* pData, VkDeviceSize stride, VkQueryResultFlags flags)
{
	TRACE("(VkDevice device = 0x%X, VkQueryPool queryPool = 0x%X, uint32_t firstQuery = %d, uint32_t queryCount = %d, size_t dataSize = %d, void* pData = 0x%X, VkDeviceSize stride = 0x%X, VkQueryResultFlags flags = %d)",
	      device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags);

	vk::Cast(queryPool)->getResults(firstQuery, queryCount, dataSize, pData, stride, flags);

	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer)
{
	TRACE("(VkDevice device = 0x%X, const VkBufferCreateInfo* pCreateInfo = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X, VkBuffer* pBuffer = 0x%X)",
		    device, pCreateInfo, pAllocator, pBuffer);

	if(pCreateInfo->pNext)
	{
		UNIMPLEMENTED();
	}

	return vk::CheckAndSet(new (pAllocator) vk::Buffer(pAllocator, pCreateInfo), pBuffer, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator)
{
	TRACE("(VkDevice device = 0x%X, VkBuffer buffer = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X)",
		    device, buffer, pAllocator);

	vk::destroy(buffer, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateBufferView(VkDevice device, const VkBufferViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBufferView* pView)
{
	TRACE("(VkDevice device, const VkBufferViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBufferView* pView)",
	      device, pCreateInfo, pAllocator, pView);

	if(pCreateInfo->pNext || pCreateInfo->flags)
	{
		UNIMPLEMENTED();
	}

	return vk::CheckAndSet(new (pAllocator) vk::BufferView(pCreateInfo), pView, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks* pAllocator)
{
	TRACE("(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks* pAllocator)",
	      device, bufferView, pAllocator);

	vk::destroy(bufferView, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImage* pImage)
{
	TRACE("(VkDevice device = 0x%X, const VkImageCreateInfo* pCreateInfo = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X, VkImage* pImage = 0x%X)",
		    device, pCreateInfo, pAllocator, pImage);

	if(pCreateInfo->pNext)
	{
		UNIMPLEMENTED();
	}

	return vk::CheckAndSet(new (pAllocator) vk::Image(pAllocator, pCreateInfo), pImage, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator)
{
	TRACE("(VkDevice device = 0x%X, VkImage image = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X)",
		    device, image, pAllocator);

	vk::destroy(image, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL vkGetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource* pSubresource, VkSubresourceLayout* pLayout)
{
	TRACE("(VkDevice device, VkImage image, const VkImageSubresource* pSubresource, VkSubresourceLayout* pLayout)",
	      device, image, pSubresource, pLayout);

	vk::Cast(image)->getSubresourceLayout(pSubresource, pLayout);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImageView* pView)
{
	TRACE("(VkDevice device = 0x%X, const VkImageViewCreateInfo* pCreateInfo = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X, VkImageView* pView = 0x%X)",
		    device, pCreateInfo, pAllocator, pView);

	if(pCreateInfo->pNext || pCreateInfo->flags)
	{
		UNIMPLEMENTED();
	}

	return vk::CheckAndSet(new (pAllocator) vk::ImageView(pCreateInfo), pView, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator)
{
	TRACE("(VkDevice device = 0x%X, VkImageView imageView = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X)",
	      device, imageView, pAllocator);

	vk::destroy(imageView, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule)
{
	TRACE("(VkDevice device = 0x%X, const VkShaderModuleCreateInfo* pCreateInfo = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X, VkShaderModule* pShaderModule = 0x%X)",
		    device, pCreateInfo, pAllocator, pShaderModule);

	if(pCreateInfo->pNext || pCreateInfo->flags)
	{
		UNIMPLEMENTED();
	}

	return vk::CheckAndSet(new (pAllocator) vk::ShaderModule(pAllocator, pCreateInfo), pShaderModule, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyShaderModule(VkDevice device, VkShaderModule shaderModule, const VkAllocationCallbacks* pAllocator)
{
	TRACE("(VkDevice device = 0x%X, VkShaderModule shaderModule = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X)",
		    device, shaderModule, pAllocator);

	vk::destroy(shaderModule, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache)
{
	TRACE("(VkDevice device, const VkPipelineCacheCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache)",
	      device, pCreateInfo, pAllocator, pPipelineCache);

	if(pCreateInfo->pNext || pCreateInfo->flags)
	{
		UNIMPLEMENTED();
	}

	return vk::CheckAndSet(new (pAllocator) vk::PipelineCache(pCreateInfo), pPipelineCache, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache, const VkAllocationCallbacks* pAllocator)
{
	TRACE("(VkDevice device, VkPipelineCache pipelineCache, const VkAllocationCallbacks* pAllocator)",
	      device, pipelineCache, pAllocator);

	vk::destroy(pipelineCache, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache, size_t* pDataSize, void* pData)
{
	TRACE("(VkDevice device, VkPipelineCache pipelineCache, size_t* pDataSize, void* pData)",
	      device, pipelineCache, pDataSize, pData);

	if(!pData)
	{
		*pDataSize = vk::Cast(pipelineCache)->getDataSize();
	}
	else
	{
		vk::Cast(pipelineCache)->copyData(*pDataSize, pData);
	}

	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL vkMergePipelineCaches(VkDevice device, VkPipelineCache dstCache, uint32_t srcCacheCount, const VkPipelineCache* pSrcCaches)
{
	TRACE("(VkDevice device, VkPipelineCache dstCache, uint32_t srcCacheCount, const VkPipelineCache* pSrcCaches)",
	      device, dstCache, srcCacheCount, pSrcCaches);

	vk::Cast(dstCache)->merge(srcCacheCount, pSrcCaches);

	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkGraphicsPipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines)
{
	TRACE("(VkDevice device = 0x%X, VkPipelineCache pipelineCache = 0x%X, uint32_t createInfoCount = %d, const VkGraphicsPipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator = 0x%X, VkPipeline* pPipelines = 0x%X)",
		    device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);

	if(pipelineCache != VK_NULL_HANDLE)
	{
		UNIMPLEMENTED();
	}

	for(uint32_t i = 0; i < createInfoCount; ++i)
	{
		VkResult result =
			vk::CheckAndSet(new (pAllocator) vk::Pipeline(pAllocator, pCreateInfos[i]), &(pPipelines[i]), pAllocator);
		if(result != VK_SUCCESS)
		{
			return result;
		}
	}

	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkComputePipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines)
{
	TRACE("(VkDevice device = 0x%X, VkPipelineCache pipelineCache = 0x%X, uint32_t createInfoCount = %d, const VkComputePipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator = 0x%X, VkPipeline* pPipelines = 0x%X)",
		device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);

	if(pipelineCache != VK_NULL_HANDLE)
	{
		UNIMPLEMENTED();
	}

	for(uint32_t i = 0; i < createInfoCount; ++i)
	{
		VkResult result =
			vk::CheckAndSet(new (pAllocator) vk::Pipeline(pAllocator, pCreateInfos[i]), &(pPipelines[i]), pAllocator);
		if(result != VK_SUCCESS)
		{
			return result;
		}
	}

	return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator)
{
	TRACE("(VkDevice device = 0x%X, VkPipeline pipeline = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X)",
		    device, pipeline, pAllocator);

	vk::destroy(pipeline, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout)
{
	TRACE("(VkDevice device = 0x%X, const VkPipelineLayoutCreateInfo* pCreateInfo = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X, VkPipelineLayout* pPipelineLayout = 0x%X)",
		    device, pCreateInfo, pAllocator, pPipelineLayout);

	if(pCreateInfo->pNext || pCreateInfo->flags)
	{
		UNIMPLEMENTED();
	}

	return vk::CheckAndSet(new (pAllocator) vk::PipelineLayout(pAllocator, pCreateInfo), pPipelineLayout, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout, const VkAllocationCallbacks* pAllocator)
{
	TRACE("(VkDevice device = 0x%X, VkPipelineLayout pipelineLayout = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X)",
		    device, pipelineLayout, pAllocator);

	vk::destroy(pipelineLayout, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSampler* pSampler)
{
	TRACE("(VkDevice device = 0x%X, const VkSamplerCreateInfo* pCreateInfo = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X, VkSampler* pSampler = 0x%X)",
		    device, pCreateInfo, pAllocator, pSampler);

	if(pCreateInfo->pNext || pCreateInfo->flags)
	{
		UNIMPLEMENTED();
	}

	return vk::CheckAndSet(new (pAllocator) vk::Sampler(device, pCreateInfo), pSampler, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL vkDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator)
{
	TRACE("(VkDevice device = 0x%X, VkSampler sampler = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X)",
		    device, sampler, pAllocator);

	vk::destroy(sampler, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorSetLayout* pSetLayout)
{
	TRACE("(VkDevice device = 0x%X, const VkDescriptorSetLayoutCreateInfo* pCreateInfo = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X, VkDescriptorSetLayout* pSetLayout = 0x%X)",
	      device, pCreateInfo, pAllocator, pSetLayout);

	if(pCreateInfo->pNext)
	{
		UNIMPLEMENTED();
	}

	return vk::CheckAndSet(new (pAllocator) vk::DescriptorSetLayout(pCreateInfo), pSetLayout, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, const VkAllocationCallbacks* pAllocator)
{
	TRACE("(VkDevice device = 0x%X, VkDescriptorSetLayout descriptorSetLayout = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X)",
	      device, descriptorSetLayout, pAllocator);

	vk::destroy(descriptorSetLayout, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool)
{
	TRACE("(VkDevice device = 0x%X, const VkDescriptorPoolCreateInfo* pCreateInfo = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X, VkDescriptorPool* pDescriptorPool = 0x%X)",
	      device, pCreateInfo, pAllocator, pDescriptorPool);

	if(pCreateInfo->pNext)
	{
		UNIMPLEMENTED();
	}

	return vk::CheckAndSet(new (pAllocator) vk::DescriptorPool(pAllocator, pCreateInfo), pDescriptorPool, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, const VkAllocationCallbacks* pAllocator)
{
	TRACE("(VkDevice device = 0x%X, VkDescriptorPool descriptorPool = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X)",
	      device, descriptorPool, pAllocator);

	vk::destroy(descriptorPool, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags flags)
{
	TRACE("(VkDevice device = 0x%X, VkDescriptorPool descriptorPool = 0x%X, VkDescriptorPoolResetFlags flags = 0x%X)",
	      device, descriptorPool, flags);

	if(flags)
	{
		UNIMPLEMENTED();
	}

	vk::Cast(descriptorPool)->reset();

	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL vkAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo, VkDescriptorSet* pDescriptorSets)
{
	TRACE("(VkDevice device = 0x%X, const VkDescriptorSetAllocateInfo* pAllocateInfo = 0x%X, VkDescriptorSet* pDescriptorSets = 0x%X)",
	      device, pAllocateInfo, pDescriptorSets);

	if(pAllocateInfo->pNext)
	{
		UNIMPLEMENTED();
	}

	vk::Cast(pAllocateInfo->descriptorPool)->allocate(pAllocateInfo->descriptorSetCount, pAllocateInfo->pSetLayouts, pDescriptorSets);

	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL vkFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets)
{
	TRACE("(VkDevice device = 0x%X, VkDescriptorPool descriptorPool = 0x%X, uint32_t descriptorSetCount = %d, const VkDescriptorSet* pDescriptorSets = 0x%X)",
	      device, descriptorPool, descriptorSetCount, pDescriptorSets);

	vk::Cast(descriptorPool)->free(descriptorSetCount, pDescriptorSets);

	return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL vkUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount, const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount, const VkCopyDescriptorSet* pDescriptorCopies)
{
	TRACE("()");
	UNIMPLEMENTED();
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer)
{
	TRACE("(VkDevice device = 0x%X, const VkFramebufferCreateInfo* pCreateInfo = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X, VkFramebuffer* pFramebuffer = 0x%X)",
		    device, pCreateInfo, pAllocator, pFramebuffer);

	if(pCreateInfo->pNext || pCreateInfo->flags)
	{
		UNIMPLEMENTED();
	}

	return vk::CheckAndSet(new (pAllocator) vk::Framebuffer(pAllocator, pCreateInfo), pFramebuffer, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks* pAllocator)
{
	TRACE("(VkDevice device = 0x%X, VkFramebuffer framebuffer = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X)");

	vk::destroy(framebuffer, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass)
{
	TRACE("(VkDevice device = 0x%X, const VkRenderPassCreateInfo* pCreateInfo = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X, VkRenderPass* pRenderPass = 0x%X)",
		    device, pCreateInfo, pAllocator, pRenderPass);

	if(pCreateInfo->pNext || pCreateInfo->flags)
	{
		UNIMPLEMENTED();
	}

	return vk::CheckAndSet(new (pAllocator) vk::RenderPass(pAllocator, pCreateInfo), pRenderPass, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator)
{
	TRACE("(VkDevice device = 0x%X, VkRenderPass renderPass = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X)",
		    device, renderPass, pAllocator);

	vk::destroy(renderPass, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL vkGetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass, VkExtent2D* pGranularity)
{
	TRACE("()");
	UNIMPLEMENTED();
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool)
{
	TRACE("(VkDevice device = 0x%X, const VkCommandPoolCreateInfo* pCreateInfo = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X, VkCommandPool* pCommandPool = 0x%X)",
		    device, pCreateInfo, pAllocator, pCommandPool);

	if(pCreateInfo->pNext)
	{
		UNIMPLEMENTED();
	}

	return vk::CheckAndSet(new (pAllocator) vk::CommandPool(pCreateInfo), pCommandPool, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator)
{
	TRACE("(VkDevice device = 0x%X, VkCommandPool commandPool = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X)",
		    device, commandPool, pAllocator);

	vk::destroy(commandPool, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags)
{
	TRACE("()");
	UNIMPLEMENTED();
	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL vkAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo, VkCommandBuffer* pCommandBuffers)
{
	TRACE("(VkDevice device = 0x%X, const VkCommandBufferAllocateInfo* pAllocateInfo = 0x%X, VkCommandBuffer* pCommandBuffers = 0x%X)",
		    device, pAllocateInfo, pCommandBuffers);

	if(pAllocateInfo->pNext)
	{
		UNIMPLEMENTED();
	}

	return vk::Cast(pAllocateInfo->commandPool)->allocateCommandBuffers(
		pAllocateInfo->level, pAllocateInfo->commandBufferCount, pCommandBuffers);
}

VKAPI_ATTR void VKAPI_CALL vkFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers)
{
	TRACE("(VkDevice device = 0x%X, VkCommandPool commandPool = 0x%X, uint32_t commandBufferCount = %d, const VkCommandBuffer* pCommandBuffers = 0x%X)",
		    device, commandPool, commandBufferCount, pCommandBuffers);

	vk::Cast(commandPool)->freeCommandBuffers(commandBufferCount, pCommandBuffers);
}

VKAPI_ATTR VkResult VKAPI_CALL vkBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo)
{
	TRACE("(VkCommandBuffer commandBuffer = 0x%X, const VkCommandBufferBeginInfo* pBeginInfo = 0x%X)",
		    commandBuffer, pBeginInfo);

	if(pBeginInfo->pNext)
	{
		UNIMPLEMENTED();
	}

	vk::Cast(commandBuffer)->begin(pBeginInfo->flags, pBeginInfo->pInheritanceInfo);

	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL vkEndCommandBuffer(VkCommandBuffer commandBuffer)
{
	TRACE("(VkCommandBuffer commandBuffer = 0x%X)", commandBuffer);

	vk::Cast(commandBuffer)->end();

	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL vkResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags)
{
	TRACE("()");
	UNIMPLEMENTED();
	return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL vkCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline)
{
	TRACE("(VkCommandBuffer commandBuffer = 0x%X, VkPipelineBindPoint pipelineBindPoint = %d, VkPipeline pipeline = 0x%X)",
		    commandBuffer, pipelineBindPoint, pipeline);

	vk::Cast(commandBuffer)->bindPipeline(pipelineBindPoint, pipeline);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkViewport* pViewports)
{
	TRACE("()");
	UNIMPLEMENTED();
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount, const VkRect2D* pScissors)
{
	TRACE("()");
	UNIMPLEMENTED();
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth)
{
	TRACE("()");
	UNIMPLEMENTED();
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor)
{
	TRACE("()");
	UNIMPLEMENTED();
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4])
{
	TRACE("()");
	UNIMPLEMENTED();
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds)
{
	TRACE("()");
	UNIMPLEMENTED();
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t compareMask)
{
	TRACE("()");
	UNIMPLEMENTED();
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t writeMask)
{
	TRACE("()");
	UNIMPLEMENTED();
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t reference)
{
	TRACE("()");
	UNIMPLEMENTED();
}

VKAPI_ATTR void VKAPI_CALL vkCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets)
{
	TRACE("()");
	UNIMPLEMENTED();
}

VKAPI_ATTR void VKAPI_CALL vkCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType)
{
	TRACE("()");
	UNIMPLEMENTED();
}

VKAPI_ATTR void VKAPI_CALL vkCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets)
{
	TRACE("(VkCommandBuffer commandBuffer = 0x%X, uint32_t firstBinding = %d, uint32_t bindingCount = %d, const VkBuffer* pBuffers = 0x%X, const VkDeviceSize* pOffsets = 0x%X)",
		    commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);

	vk::Cast(commandBuffer)->bindVertexBuffers(firstBinding, bindingCount, pBuffers, pOffsets);
}

VKAPI_ATTR void VKAPI_CALL vkCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
	TRACE("(VkCommandBuffer commandBuffer = 0x%X, uint32_t vertexCount = %d, uint32_t instanceCount = %d, uint32_t firstVertex = %d, uint32_t firstInstance = %d)",
		    commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);

	vk::Cast(commandBuffer)->draw(vertexCount, instanceCount, firstVertex, firstInstance);
}

VKAPI_ATTR void VKAPI_CALL vkCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
{
	TRACE("(VkCommandBuffer commandBuffer = 0x%X, uint32_t indexCount = %d, uint32_t instanceCount = %d, uint32_t firstIndex = %d, int32_t vertexOffset = %d, uint32_t firstInstance = %d)",
		    commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);

	vk::Cast(commandBuffer)->drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

VKAPI_ATTR void VKAPI_CALL vkCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride)
{
	TRACE("(VkCommandBuffer commandBuffer = 0x%X, VkBuffer buffer = 0x%X, VkDeviceSize offset = %d, uint32_t drawCount = %d, uint32_t stride = %d)",
		    commandBuffer, buffer, offset, drawCount, stride);

	vk::Cast(commandBuffer)->drawIndirect(buffer, offset, drawCount, stride);
}

VKAPI_ATTR void VKAPI_CALL vkCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride)
{
	TRACE("(VkCommandBuffer commandBuffer = 0x%X, VkBuffer buffer = 0x%X, VkDeviceSize offset = %d, uint32_t drawCount = %d, uint32_t stride = %d)",
		    commandBuffer, buffer, offset, drawCount, stride);

	vk::Cast(commandBuffer)->drawIndexedIndirect(buffer, offset, drawCount, stride);
}

VKAPI_ATTR void VKAPI_CALL vkCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
	TRACE("()");
	UNIMPLEMENTED();
}

VKAPI_ATTR void VKAPI_CALL vkCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset)
{
	TRACE("()");
	UNIMPLEMENTED();
}

VKAPI_ATTR void VKAPI_CALL vkCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy* pRegions)
{
	TRACE("()");
	UNIMPLEMENTED();
}

VKAPI_ATTR void VKAPI_CALL vkCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy* pRegions)
{
	TRACE("()");
	UNIMPLEMENTED();
}

VKAPI_ATTR void VKAPI_CALL vkCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit* pRegions, VkFilter filter)
{
	TRACE("()");
	UNIMPLEMENTED();
}

VKAPI_ATTR void VKAPI_CALL vkCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy* pRegions)
{
	TRACE("()");
	UNIMPLEMENTED();
}

VKAPI_ATTR void VKAPI_CALL vkCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions)
{
	TRACE("(VkCommandBuffer commandBuffer = 0x%X, VkImage srcImage = 0x%X, VkImageLayout srcImageLayout = %d, VkBuffer dstBuffer = 0x%X, uint32_t regionCount = %d, const VkBufferImageCopy* pRegions = 0x%X)",
		    commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);

	vk::Cast(commandBuffer)->copyImageToBuffer(srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
}

VKAPI_ATTR void VKAPI_CALL vkCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize dataSize, const void* pData)
{
	TRACE("()");
	UNIMPLEMENTED();
}

VKAPI_ATTR void VKAPI_CALL vkCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size, uint32_t data)
{
	TRACE("()");
	UNIMPLEMENTED();
}

VKAPI_ATTR void VKAPI_CALL vkCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearColorValue* pColor, uint32_t rangeCount, const VkImageSubresourceRange* pRanges)
{
	TRACE("()");
	UNIMPLEMENTED();
}

VKAPI_ATTR void VKAPI_CALL vkCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount, const VkImageSubresourceRange* pRanges)
{
	TRACE("()");
	UNIMPLEMENTED();
}

VKAPI_ATTR void VKAPI_CALL vkCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount, const VkClearAttachment* pAttachments, uint32_t rectCount, const VkClearRect* pRects)
{
	TRACE("()");
	UNIMPLEMENTED();
}

VKAPI_ATTR void VKAPI_CALL vkCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageResolve* pRegions)
{
	TRACE("()");
	UNIMPLEMENTED();
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask)
{
	TRACE("()");
	UNIMPLEMENTED();
}

VKAPI_ATTR void VKAPI_CALL vkCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask)
{
	TRACE("()");
	UNIMPLEMENTED();
}

VKAPI_ATTR void VKAPI_CALL vkCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers)
{
	TRACE("()");
	UNIMPLEMENTED();
}

VKAPI_ATTR void VKAPI_CALL vkCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers)
{
	TRACE("(VkCommandBuffer commandBuffer = 0x%X, VkPipelineStageFlags srcStageMask = 0x%X, VkPipelineStageFlags dstStageMask = 0x%X, VkDependencyFlags dependencyFlags = %d, uint32_t memoryBarrierCount = %d, onst VkMemoryBarrier* pMemoryBarriers = 0x%X,"
		    " uint32_t bufferMemoryBarrierCount = %d, const VkBufferMemoryBarrier* pBufferMemoryBarriers = 0x%X, uint32_t imageMemoryBarrierCount = %d, const VkImageMemoryBarrier* pImageMemoryBarriers = 0x%X)",
		    commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);

	vk::Cast(commandBuffer)->pipelineBarrier(srcStageMask, dstStageMask, dependencyFlags,
		                                    memoryBarrierCount, pMemoryBarriers,
		                                    bufferMemoryBarrierCount, pBufferMemoryBarriers,
		                                    imageMemoryBarrierCount, pImageMemoryBarriers);
}

VKAPI_ATTR void VKAPI_CALL vkCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags)
{
	TRACE("()");
	UNIMPLEMENTED();
}

VKAPI_ATTR void VKAPI_CALL vkCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query)
{
	TRACE("()");
	UNIMPLEMENTED();
}

VKAPI_ATTR void VKAPI_CALL vkCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount)
{
	TRACE("()");
	UNIMPLEMENTED();
}

VKAPI_ATTR void VKAPI_CALL vkCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkQueryPool queryPool, uint32_t query)
{
	TRACE("()");
	UNIMPLEMENTED();
}

VKAPI_ATTR void VKAPI_CALL vkCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride, VkQueryResultFlags flags)
{
	TRACE("()");
	UNIMPLEMENTED();
}

VKAPI_ATTR void VKAPI_CALL vkCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* pValues)
{
	TRACE("()");
	UNIMPLEMENTED();
}

VKAPI_ATTR void VKAPI_CALL vkCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin, VkSubpassContents contents)
{
	TRACE("(VkCommandBuffer commandBuffer = 0x%X, const VkRenderPassBeginInfo* pRenderPassBegin = 0x%X, VkSubpassContents contents = %d)",
		    commandBuffer, pRenderPassBegin, contents);

	if(pRenderPassBegin->pNext)
	{
		UNIMPLEMENTED();
	}

	vk::Cast(commandBuffer)->beginRenderPass(pRenderPassBegin->renderPass, pRenderPassBegin->framebuffer,
		                                    pRenderPassBegin->renderArea, pRenderPassBegin->clearValueCount,
		                                    pRenderPassBegin->pClearValues, contents);
}

VKAPI_ATTR void VKAPI_CALL vkCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents)
{
	TRACE("()");
	UNIMPLEMENTED();
}

VKAPI_ATTR void VKAPI_CALL vkCmdEndRenderPass(VkCommandBuffer commandBuffer)
{
	TRACE("(VkCommandBuffer commandBuffer = 0x%X)", commandBuffer);

	vk::Cast(commandBuffer)->endRenderPass();
}

VKAPI_ATTR void VKAPI_CALL vkCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers)
{
	TRACE("()");
	UNIMPLEMENTED();
}

VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateInstanceVersion(uint32_t* pApiVersion)
{
	TRACE("(uint32_t* pApiVersion = 0x%X)", pApiVersion);
	*pApiVersion = VK_API_VERSION_1_1;
	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL vkBindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos)
{
	TRACE("(VkDevice device = 0x%X, uint32_t bindInfoCount = %d, const VkBindBufferMemoryInfo* pBindInfos = 0x%X)",
	      device, bindInfoCount, pBindInfos);

	for(uint32_t i = 0; i < bindInfoCount; i++)
	{
		if(pBindInfos[i].pNext)
		{
			UNIMPLEMENTED();
		}

		vkBindBufferMemory(device, pBindInfos[i].buffer, pBindInfos[i].memory, pBindInfos[i].memoryOffset);
	}

	return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL vkBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos)
{
	TRACE("(VkDevice device = 0x%X, uint32_t bindInfoCount = %d, const VkBindImageMemoryInfo* pBindInfos = 0x%X)",
	      device, bindInfoCount, pBindInfos);

	for(uint32_t i = 0; i < bindInfoCount; i++)
	{
		if(pBindInfos[i].pNext)
		{
			UNIMPLEMENTED();
		}

		vkBindImageMemory(device, pBindInfos[i].image, pBindInfos[i].memory, pBindInfos[i].memoryOffset);
	}

	return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL vkGetDeviceGroupPeerMemoryFeatures(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex, uint32_t remoteDeviceIndex, VkPeerMemoryFeatureFlags* pPeerMemoryFeatures)
{
	TRACE("()");
	UNIMPLEMENTED();
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask)
{
	TRACE("()");
	UNIMPLEMENTED();
}

VKAPI_ATTR void VKAPI_CALL vkCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
	TRACE("()");
	UNIMPLEMENTED();
}

VKAPI_ATTR VkResult VKAPI_CALL vkEnumeratePhysicalDeviceGroups(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount, VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties)
{
	TRACE("VkInstance instance = 0x%X, uint32_t* pPhysicalDeviceGroupCount = 0x%X, VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties = 0x%X",
	      instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);

	vk::CountedObject<vk::Instance, VkPhysicalDeviceGroupProperties>::get(
		vk::Cast(instance), pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties,
		&(vk::Instance::getPhysicalDeviceGroupCount), &(vk::Instance::getPhysicalDeviceGroups));

	return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL vkGetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements)
{
	TRACE("(VkDevice device = 0x%X, const VkImageMemoryRequirementsInfo2* pInfo = 0x%X, VkMemoryRequirements2* pMemoryRequirements = 0x%X)",
	      device, pInfo, pMemoryRequirements);

	if(pInfo->pNext || pMemoryRequirements->pNext)
	{
		UNIMPLEMENTED();
	}

	vkGetImageMemoryRequirements(device, pInfo->image, &(pMemoryRequirements->memoryRequirements));
}

VKAPI_ATTR void VKAPI_CALL vkGetBufferMemoryRequirements2(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements)
{
	TRACE("(VkDevice device = 0x%X, const VkBufferMemoryRequirementsInfo2* pInfo = 0x%X, VkMemoryRequirements2* pMemoryRequirements = 0x%X)",
	      device, pInfo, pMemoryRequirements);

	if(pInfo->pNext || pMemoryRequirements->pNext)
	{
		UNIMPLEMENTED();
	}

	vkGetBufferMemoryRequirements(device, pInfo->buffer, &(pMemoryRequirements->memoryRequirements));
}

VKAPI_ATTR void VKAPI_CALL vkGetImageSparseMemoryRequirements2(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo, uint32_t* pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements2* pSparseMemoryRequirements)
{
	TRACE("(VkDevice device = 0x%X, const VkImageSparseMemoryRequirementsInfo2* pInfo = 0x%X, uint32_t* pSparseMemoryRequirementCount = 0x%X, VkSparseImageMemoryRequirements2* pSparseMemoryRequirements = 0x%X)",
	      device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);

	if(pInfo->pNext || pSparseMemoryRequirements->pNext)
	{
		UNIMPLEMENTED();
	}

	vkGetImageSparseMemoryRequirements(device, pInfo->image, pSparseMemoryRequirementCount, &(pSparseMemoryRequirements->memoryRequirements));
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures)
{
	TRACE("(VkPhysicalDevice physicalDevice = 0x%X, VkPhysicalDeviceFeatures2* pFeatures = 0x%X)", physicalDevice, pFeatures);

	void* pNext = pFeatures->pNext;
	while(pNext)
	{
		switch(*reinterpret_cast<const VkStructureType*>(pNext))
		{
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES:
		{
			auto& features = *reinterpret_cast<VkPhysicalDeviceSamplerYcbcrConversionFeatures*>(pNext);
			features.samplerYcbcrConversion = VK_FALSE;
			pNext = features.pNext;
		}
		break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES:
		{
			auto& features = *reinterpret_cast<VkPhysicalDevice16BitStorageFeatures*>(pNext);
			features.storageBuffer16BitAccess = VK_FALSE;
			features.storageInputOutput16 = VK_FALSE;
			features.storagePushConstant16 = VK_FALSE;
			features.uniformAndStorageBuffer16BitAccess = VK_FALSE;
			pNext = features.pNext;
		}
		break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VARIABLE_POINTER_FEATURES:
		{
			auto& features = *reinterpret_cast<VkPhysicalDeviceVariablePointerFeatures*>(pNext);
			features.variablePointersStorageBuffer = VK_FALSE;
			features.variablePointers = VK_FALSE;
			pNext = features.pNext;
		}
		break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES_KHR:
		{
			auto& features = *reinterpret_cast<VkPhysicalDevice8BitStorageFeaturesKHR*>(pNext);
			features.storageBuffer8BitAccess = VK_FALSE;
			features.uniformAndStorageBuffer8BitAccess = VK_FALSE;
			features.storagePushConstant8 = VK_FALSE;
			pNext = features.pNext;
		}
		break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES:
		{
			auto& features = *reinterpret_cast<VkPhysicalDeviceMultiviewFeatures*>(pNext);
			features.multiview = VK_FALSE;
			features.multiviewGeometryShader = VK_FALSE;
			features.multiviewTessellationShader = VK_FALSE;
			pNext = features.pNext;
		}
		break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_FEATURES:
		{
			auto& features = *reinterpret_cast<VkPhysicalDeviceProtectedMemoryFeatures*>(pNext);
			features.protectedMemory = VK_FALSE;
			pNext = features.pNext;
		}
		break;
		default:
			UNIMPLEMENTED();
		}
	}

	vkGetPhysicalDeviceFeatures(physicalDevice, &(pFeatures->features));
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2* pProperties)
{
	TRACE("(VkPhysicalDevice physicalDevice = 0x%X, VkPhysicalDeviceProperties2* pProperties = 0x%X)", physicalDevice, pProperties);

	void* pNext = pProperties->pNext;
	while(pNext)
	{
		switch(*reinterpret_cast<const VkStructureType*>(pNext))
		{
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES:
		{
			auto& features = *reinterpret_cast<VkPhysicalDeviceIDProperties*>(pNext);
			static const uint8_t device[VK_UUID_SIZE] = "SwiftShaderUUID";
			static const uint8_t driver[VK_UUID_SIZE] = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\1";
			memcpy(features.deviceUUID, device, VK_UUID_SIZE);
			memcpy(features.driverUUID, driver, VK_UUID_SIZE);
			memset(features.deviceLUID, 0, VK_LUID_SIZE);
			features.deviceNodeMask = 0;
			features.deviceLUIDValid = VK_FALSE;
			pNext = features.pNext;
		}
		break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_3_PROPERTIES:
		{
			auto& features = *reinterpret_cast<VkPhysicalDeviceMaintenance3Properties*>(pNext);
			features.maxMemoryAllocationSize = 1 << 31;
			features.maxPerSetDescriptors = 1024;
			pNext = features.pNext;
		}
		break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PROPERTIES:
		{
			auto& features = *reinterpret_cast<VkPhysicalDeviceMultiviewProperties*>(pNext);
			features.maxMultiviewInstanceIndex = (1 << 27) - 1;
			features.maxMultiviewViewCount = 6;
			pNext = features.pNext;
		}
		break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_POINT_CLIPPING_PROPERTIES:
		{
			auto& features = *reinterpret_cast<VkPhysicalDevicePointClippingProperties*>(pNext);
			features.pointClippingBehavior = VK_POINT_CLIPPING_BEHAVIOR_ALL_CLIP_PLANES;
			pNext = features.pNext;
		}
		break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_PROPERTIES:
		{
			auto& features = *reinterpret_cast<VkPhysicalDeviceProtectedMemoryProperties*>(pNext);
			features.protectedNoFault = VK_FALSE;
			pNext = features.pNext;
		}
		break;
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES:
		{
			auto& features = *reinterpret_cast<VkPhysicalDeviceSubgroupProperties*>(pNext);
			features.subgroupSize = 1;
			features.supportedStages = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
			features.supportedOperations = VK_SUBGROUP_FEATURE_BASIC_BIT;
			features.quadOperationsInAllStages = VK_FALSE;
			pNext = features.pNext;
		}
		break;
		default:
			UNIMPLEMENTED();
		}
	}

	vkGetPhysicalDeviceProperties(physicalDevice, &(pProperties->properties));
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceFormatProperties2(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties2* pFormatProperties)
{
	TRACE("(VkPhysicalDevice physicalDevice = 0x%X, VkFormat format = %d, VkFormatProperties2* pFormatProperties = 0x%X)",
		    physicalDevice, format, pFormatProperties);

	if(pFormatProperties->pNext)
	{
		UNIMPLEMENTED();
	}

	vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &(pFormatProperties->formatProperties));
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo, VkImageFormatProperties2* pImageFormatProperties)
{
	TRACE("(VkPhysicalDevice physicalDevice = 0x%X, const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo = 0x%X, VkImageFormatProperties2* pImageFormatProperties = 0x%X)",
		    physicalDevice, pImageFormatInfo, pImageFormatProperties);

	if(pImageFormatInfo->pNext)
	{
		UNIMPLEMENTED();
	}

	return vkGetPhysicalDeviceImageFormatProperties(physicalDevice,
		                                            pImageFormatInfo->format,
		                                            pImageFormatInfo->type,
		                                            pImageFormatInfo->tiling,
		                                            pImageFormatInfo->usage,
		                                            pImageFormatInfo->flags,
		                                            &(pImageFormatProperties->imageFormatProperties));
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties2* pQueueFamilyProperties)
{
	TRACE("(VkPhysicalDevice physicalDevice = 0x%X, uint32_t* pQueueFamilyPropertyCount = 0x%X, VkQueueFamilyProperties2* pQueueFamilyProperties = 0x%X)",
		physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);

	if(pQueueFamilyProperties && pQueueFamilyProperties->pNext)
	{
		UNIMPLEMENTED();
	}

	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount,
		pQueueFamilyProperties ? &(pQueueFamilyProperties->queueFamilyProperties) : nullptr);
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceMemoryProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties2* pMemoryProperties)
{
	TRACE("(VkPhysicalDevice physicalDevice = 0x%X, VkPhysicalDeviceMemoryProperties2* pMemoryProperties = 0x%X)", physicalDevice, pMemoryProperties);

	if(pMemoryProperties->pNext)
	{
		UNIMPLEMENTED();
	}

	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &(pMemoryProperties->memoryProperties));
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceSparseImageFormatProperties2(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo, uint32_t* pPropertyCount, VkSparseImageFormatProperties2* pProperties)
{
	TRACE("(VkPhysicalDevice physicalDevice = 0x%X, const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo = 0x%X, uint32_t* pPropertyCount = 0x%X, VkSparseImageFormatProperties2* pProperties = 0x%X)",
	     physicalDevice, pFormatInfo, pPropertyCount, pProperties);

	if(pProperties && pProperties->pNext)
	{
		UNIMPLEMENTED();
	}

	vkGetPhysicalDeviceSparseImageFormatProperties(physicalDevice, pFormatInfo->format, pFormatInfo->type,
	                                               pFormatInfo->samples, pFormatInfo->usage, pFormatInfo->tiling,
	                                               pPropertyCount, pProperties ? &(pProperties->properties) : nullptr);
}

VKAPI_ATTR void VKAPI_CALL vkTrimCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags)
{
	TRACE("(VkDevice device = 0x%X, VkCommandPool commandPool = 0x%X, VkCommandPoolTrimFlags flags = %d)",
	      device, commandPool, flags);

	vk::Cast(commandPool)->trim();
}

VKAPI_ATTR void VKAPI_CALL vkGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2* pQueueInfo, VkQueue* pQueue)
{
	TRACE("(VkDevice device = 0x%X, const VkDeviceQueueInfo2* pQueueInfo = 0x%X, VkQueue* pQueue = 0x%X)",
	      device, pQueueInfo, pQueue);

	if(pQueueInfo->pNext)
	{
		UNIMPLEMENTED();
	}

	// The only flag that can be set here is VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT 
	// According to the Vulkan spec, 4.3.1. Queue Family Properties:
	// "VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT specifies that the device queue is a
	//  protected-capable queue. If the protected memory feature is not enabled,
	//  the VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT bit of flags must not be set."
	if(pQueueInfo->flags)
	{
		*pQueue = VK_NULL_HANDLE;
	}

	vkGetDeviceQueue(device, pQueueInfo->queueFamilyIndex, pQueueInfo->queueIndex, pQueue);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSamplerYcbcrConversion* pYcbcrConversion)
{
	TRACE("()");
	UNIMPLEMENTED();
	return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL vkDestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion, const VkAllocationCallbacks* pAllocator)
{
	TRACE("()");
	UNIMPLEMENTED();
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDescriptorUpdateTemplate(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate)
{
	TRACE("()");
	UNIMPLEMENTED();
	return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDescriptorUpdateTemplate(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const VkAllocationCallbacks* pAllocator)
{
	TRACE("()");
	UNIMPLEMENTED();
}

VKAPI_ATTR void VKAPI_CALL vkUpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void* pData)
{
	TRACE("()");
	UNIMPLEMENTED();
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceExternalBufferProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo, VkExternalBufferProperties* pExternalBufferProperties)
{
	TRACE("()");
	UNIMPLEMENTED();
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceExternalFenceProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo, VkExternalFenceProperties* pExternalFenceProperties)
{
	TRACE("()");
	UNIMPLEMENTED();
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceExternalSemaphoreProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo, VkExternalSemaphoreProperties* pExternalSemaphoreProperties)
{
	TRACE("()");
	UNIMPLEMENTED();
}

VKAPI_ATTR void VKAPI_CALL vkGetDescriptorSetLayoutSupport(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo, VkDescriptorSetLayoutSupport* pSupport)
{
	TRACE("()");
	UNIMPLEMENTED();
}

}
