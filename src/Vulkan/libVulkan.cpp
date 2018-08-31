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
#include "VkCommandPool.hpp"
#include "VkConfig.h"
#include "VkDebug.hpp"
#include "VkDevice.hpp"
#include "VkDeviceMemory.hpp"
#include "VkFence.hpp"
#include "VkFramebuffer.hpp"
#include "VkImage.hpp"
#include "VkImageView.hpp"
#include "VkInstance.hpp"
#include "VkPipeline.hpp"
#include "VkPipelineLayout.hpp"
#include "VkSampler.hpp"
#include "VkShaderModule.hpp"
#include "VkRenderPass.hpp"
#include "VkUtils.h"

namespace vk
{
	VkResult CreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance)
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
				// The values VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO and
				// VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO are reserved for
				// internal use by the loader, and do not have corresponding
				// Vulkan structures in this Specification.
				break;
			default:
				UNIMPLEMENTED();
			}
		}

		if(pCreateInfo->pApplicationInfo)
		{
			uint32_t apiVersion;
			VkResult result = vkEnumerateInstanceVersion(&apiVersion);
			if(result != VK_SUCCESS)
			{
				return result;
			}

			if(pCreateInfo->pApplicationInfo->apiVersion != apiVersion)
			{
				return VK_ERROR_INCOMPATIBLE_DRIVER;
			}
		}

		*pInstance = *(new (pAllocator) Instance(pAllocator));

		return VK_SUCCESS;
	}

	void DestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator)
	{
		TRACE("(VkInstance instance = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X)", instance, pAllocator);

		destroy(instance, pAllocator);
	}

	VkResult EnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount, VkPhysicalDevice* pPhysicalDevices)
	{
		TRACE("(VkInstance instance = 0x%X, uint32_t* pPhysicalDeviceCount = 0x%X, VkPhysicalDevice* pPhysicalDevices = 0x%X)",
		      instance, pPhysicalDeviceCount, pPhysicalDevices);

		auto myInstance = Cast(instance);

		if(!pPhysicalDevices)
		{
			*pPhysicalDeviceCount = 1;
			myInstance->physicalDeviceCount = *pPhysicalDeviceCount;
			return VK_SUCCESS;
		}

		ASSERT(myInstance->physicalDeviceCount == 1);

		*pPhysicalDevices = myInstance->physicalDevice;

		return VK_SUCCESS;
	}

	void GetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures* pFeatures)
	{
		TRACE("(VkPhysicalDevice physicalDevice = 0x%X, VkPhysicalDeviceFeatures* pFeatures = 0x%X)",
			  physicalDevice, pFeatures);

		*pFeatures = Cast(physicalDevice)->getFeatures();
	}

	void GetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties* pFormatProperties)
	{
		TRACE("GetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice = 0x%X, VkFormat format = %d, VkFormatProperties* pFormatProperties = 0x%X)",
			  physicalDevice, (int)format, pFormatProperties);

		pFormatProperties->linearTilingFeatures = 0; // Unsupported format
		pFormatProperties->optimalTilingFeatures = 0; // Unsupported format
		pFormatProperties->bufferFeatures = 0; // Unsupported format
	}

	VkResult GetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags, VkImageFormatProperties* pImageFormatProperties)
	{
		TRACE("(VkPhysicalDevice physicalDevice = 0x%X, VkFormat format = %d, VkImageType type = %d, VkImageTiling tiling = %d, VkImageUsageFlags usage = %d, VkImageCreateFlags flags = %d, VkImageFormatProperties* pImageFormatProperties = 0x%X)",
			  physicalDevice, (int)format, (int)type, (int)tiling, usage, flags, pImageFormatProperties);

		pImageFormatProperties->maxArrayLayers = 1 << MaxImageArrayLayers;

		switch(type)
		{
		case VK_IMAGE_TYPE_1D:
			pImageFormatProperties->maxMipLevels = MaxImageLevels1D;
			pImageFormatProperties->maxExtent.width = 1 << MaxImageLevels1D;
			pImageFormatProperties->maxExtent.height = 1;
			pImageFormatProperties->maxExtent.depth = 1;
			break;
		case VK_IMAGE_TYPE_2D:
			if(flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT)
			{
				pImageFormatProperties->maxMipLevels = MaxImageLevelsCube;
				pImageFormatProperties->maxExtent.width = 1 << MaxImageLevelsCube;
				pImageFormatProperties->maxExtent.height = 1 << MaxImageLevelsCube;
				pImageFormatProperties->maxExtent.depth = 1;
			}
			else
			{
				pImageFormatProperties->maxMipLevels = MaxImageLevels2D;
				pImageFormatProperties->maxExtent.width = 1 << MaxImageLevels2D;
				pImageFormatProperties->maxExtent.height = 1 << MaxImageLevels2D;
				pImageFormatProperties->maxExtent.depth = 1;
			}
			break;
		case VK_IMAGE_TYPE_3D:
			pImageFormatProperties->maxMipLevels = MaxImageLevels3D;
			pImageFormatProperties->maxExtent.width = 1 << MaxImageLevels3D;
			pImageFormatProperties->maxExtent.height = 1 << MaxImageLevels3D;
			pImageFormatProperties->maxExtent.depth = 1 << MaxImageLevels3D;
			break;
		default:
			UNREACHABLE(type);
			break;
		}

		pImageFormatProperties->maxResourceSize = 1 << 31; // Minimum value for maxResourceSize
		pImageFormatProperties->sampleCounts = vkutils::GetSampleCounts();

		return VK_SUCCESS;
	}

	void GetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties* pProperties)
	{
		TRACE("(VkPhysicalDevice physicalDevice = 0x%X, VkPhysicalDeviceProperties* pProperties = 0x%X)",
		      physicalDevice, pProperties);

		*pProperties = vkutils::GetPhysicalDeviceProperties();
	}

	void GetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties* pQueueFamilyProperties)
	{
		TRACE("(VkPhysicalDevice physicalDevice = 0x%X, uint32_t* pQueueFamilyPropertyCount = 0x%X, VkQueueFamilyProperties* pQueueFamilyProperties = 0x%X))", physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);

		if(!pQueueFamilyProperties)
		{
			*pQueueFamilyPropertyCount = 1;
		}
		else
		{
			if(*pQueueFamilyPropertyCount > 0)
			{
				pQueueFamilyProperties[0].minImageTransferGranularity.width = 1;
				pQueueFamilyProperties[0].minImageTransferGranularity.height = 1;
				pQueueFamilyProperties[0].minImageTransferGranularity.depth = 1;
				pQueueFamilyProperties[0].queueCount = 1;
				pQueueFamilyProperties[0].queueFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
				pQueueFamilyProperties[0].timestampValidBits = 0; // No support for time stamps
			}
		}
	}

	void GetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties* pMemoryProperties)
	{
		TRACE("(VkPhysicalDevice physicalDevice = 0x%X, VkPhysicalDeviceMemoryProperties* pMemoryProperties = 0x%X)", physicalDevice, pMemoryProperties);

		pMemoryProperties->memoryTypeCount = 1;
		pMemoryProperties->memoryTypes[0].heapIndex = 0;
		pMemoryProperties->memoryTypes[0].propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
		                                                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		                                                  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		pMemoryProperties->memoryHeapCount = 1;
		pMemoryProperties->memoryHeaps[0].flags = VK_MEMORY_HEAP_DEVICE_LOCAL_BIT;
		pMemoryProperties->memoryHeaps[0].size = 1 << 31; // FIXME(sugoi): This could be configurable based on available RAM
	}

	PFN_vkVoidFunction GetInstanceProcAddr(VkInstance instance, const char* pName)
	{
		TRACE("(VkInstance instance = 0x%X, const char* pName = 0x%X)", instance, pName);
		return vkutils::GetProcAddr(pName);
	}

	PFN_vkVoidFunction GetDeviceProcAddr(VkDevice device, const char* pName)
	{
		TRACE("(VkDevice device = 0x%X, const char* pName = 0x%X)", device, pName);
		return vkutils::GetProcAddr(pName);
	}

	VkResult CreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice)
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
				// The values VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO and
				// VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO are reserved for
				// internal use by the loader, and do not have corresponding
				// Vulkan structures in this Specification.
				break;
			default:
				UNIMPLEMENTED();
			}
		}

		if(pCreateInfo->queueCreateInfoCount == 0)
		{
			return VK_ERROR_INITIALIZATION_FAILED;
		}

		if(pCreateInfo->pEnabledFeatures && !vkutils::AreRequestedFeaturesPresent(*(pCreateInfo->pEnabledFeatures)))
		{
			return VK_ERROR_FEATURE_NOT_PRESENT;
		}

		uint32_t queueFamilyPropertyCount = 0;
		GetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertyCount, nullptr);

		for(uint32_t i = 0; i < pCreateInfo->queueCreateInfoCount; i++)
		{
			const VkDeviceQueueCreateInfo& queueCreateInfo = pCreateInfo->pQueueCreateInfos[i];
			if(queueCreateInfo.pNext || queueCreateInfo.flags)
			{
				UNIMPLEMENTED();
			}

			if(queueCreateInfo.queueFamilyIndex > queueFamilyPropertyCount)
			{
				return VK_ERROR_INITIALIZATION_FAILED;
			}
		}

		*pDevice = *(new (pAllocator) Device(pAllocator, physicalDevice, pCreateInfo));

		return VK_SUCCESS;
	}

	void DestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator)
	{
		TRACE("(VkDevice device = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X)", device, pAllocator);

		destroy(device, pAllocator);
	}

	VkResult EnumerateInstanceExtensionProperties(const char* pLayerName, uint32_t* pPropertyCount, VkExtensionProperties* pProperties)
	{
		TRACE("(const char* pLayerName = 0x%X, uint32_t* pPropertyCount = 0x%X, VkExtensionProperties* pProperties = 0x%X)", pLayerName, pPropertyCount, pProperties);

		if(pProperties == NULL)
		{
			*pPropertyCount = 0;
			return VK_SUCCESS;
		}

		return VK_SUCCESS;
	}

	VkResult EnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char* pLayerName, uint32_t* pPropertyCount, VkExtensionProperties* pProperties)
	{
		TRACE("(VkPhysicalDevice physicalDevice = 0x%X, const char* pLayerName, uint32_t* pPropertyCount = 0x%X, VkExtensionProperties* pProperties = 0x%X)", physicalDevice, pPropertyCount, pProperties);

		if(pProperties == NULL)
		{
			*pPropertyCount = 0;
			return VK_SUCCESS;
		}

		return VK_SUCCESS;
	}

	VkResult EnumerateInstanceLayerProperties(uint32_t* pPropertyCount, VkLayerProperties* pProperties)
	{
		TRACE("(uint32_t* pPropertyCount = 0x%X, VkLayerProperties* pProperties = 0x%X)", pPropertyCount, pProperties);

		if(pProperties == NULL)
		{
			*pPropertyCount = 0;
			return VK_SUCCESS;
		}

		return VK_SUCCESS;
	}

	VkResult EnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkLayerProperties* pProperties)
	{
		TRACE("(VkPhysicalDevice physicalDevice = 0x%X, uint32_t* pPropertyCount = 0x%X, VkLayerProperties* pProperties = 0x%X)", physicalDevice, pPropertyCount, pProperties);

		if(pProperties == NULL)
		{
			*pPropertyCount = 0;
			return VK_SUCCESS;
		}

		return VK_SUCCESS;
	}

	void GetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue)
	{
		TRACE("(VkDevice device = 0x%X, uint32_t queueFamilyIndex = %d, uint32_t queueIndex = %d, VkQueue* pQueue = 0x%X)",
		      device, queueFamilyIndex, queueIndex, pQueue);

		*pQueue = Cast(device)->getQueue(queueFamilyIndex, queueIndex);
	}

	VkResult QueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	VkResult QueueWaitIdle(VkQueue queue)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	VkResult DeviceWaitIdle(VkDevice device)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	VkResult AllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo, const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory)
	{
		TRACE("(VkDevice device = 0x%X, const VkMemoryAllocateInfo* pAllocateInfo = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X, VkDeviceMemory* pMemory = 0x%X)",
		      device, pAllocateInfo, pAllocator, pMemory);

		if(pAllocateInfo->pNext)
		{
			UNIMPLEMENTED();
		}

		*pMemory = *(new (pAllocator) DeviceMemory(pAllocateInfo));

		return VK_SUCCESS;
	}

	void FreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator)
	{
		TRACE("(VkDevice device = 0x%X, VkDeviceMemory memory = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X)",
		      device, memory, pAllocator);

		destroy(memory, pAllocator);
	}

	VkResult MapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** ppData)
	{
		TRACE("(VkDevice device = 0x%X, VkDeviceMemory memory = 0x%X, VkDeviceSize offset = %d, VkDeviceSize size = %d, VkMemoryMapFlags flags = 0x%X, void** ppData = 0x%X)",
		      device, memory, offset, size, flags, ppData);

		*ppData = Cast(memory)->map(offset, size);

		return VK_SUCCESS;
	}

	void UnmapMemory(VkDevice device, VkDeviceMemory memory)
	{
		TRACE("(VkDevice device = 0x%X, VkDeviceMemory memory = 0x%X)", device, memory);

		Cast(memory)->unmap();
	}

	VkResult FlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount, const VkMappedMemoryRange* pMemoryRanges)
	{
		TRACE("(VkDevice device = 0x%X, uint32_t memoryRangeCount = %d, const VkMappedMemoryRange* pMemoryRanges = 0x%X)",
		      device, memoryRangeCount, pMemoryRanges);

		for(uint32_t i = 0; i < memoryRangeCount; i++)
		{
			Cast(pMemoryRanges[i].memory)->flush();
		}

		return VK_SUCCESS;
	}

	VkResult InvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount, const VkMappedMemoryRange* pMemoryRanges)
	{
		TRACE("(VkDevice device = 0x%X, uint32_t memoryRangeCount = %d, const VkMappedMemoryRange* pMemoryRanges = 0x%X)",
		      device, memoryRangeCount, pMemoryRanges);

		for(uint32_t i = 0; i < memoryRangeCount; i++)
		{
			Cast(pMemoryRanges[i].memory)->invalidate();
		}

		return VK_SUCCESS;
	}

	void GetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory, VkDeviceSize* pCommittedMemoryInBytes)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	VkResult BindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset)
	{
		TRACE("(VkDevice device = 0x%X, VkBuffer buffer = 0x%X, VkDeviceMemory memory = 0x%X, VkDeviceSize memoryOffset = %d)",
		      device, buffer, memory, memoryOffset);

		Cast(buffer)->bind(memory, memoryOffset);

		return VK_SUCCESS;
	}

	VkResult BindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset)
	{
		TRACE("(VkDevice device = 0x%X, VkImage image = 0x%X, VkDeviceMemory memory = 0x%X, VkDeviceSize memoryOffset = %d)",
		      device, image, memory, memoryOffset);

		Cast(image)->bind(memory, memoryOffset);

		return VK_SUCCESS;
	}

	void GetBufferMemoryRequirements(VkDevice device, VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements)
	{
		TRACE("(VkDevice device = 0x%X, VkBuffer buffer = 0x%X, VkMemoryRequirements* pMemoryRequirements = 0x%X)",
		      device, buffer, pMemoryRequirements);

		*pMemoryRequirements = Cast(buffer)->GetMemoryRequirements();
	}

	void GetImageMemoryRequirements(VkDevice device, VkImage image, VkMemoryRequirements* pMemoryRequirements)
	{
		TRACE("(VkDevice device = 0x%X, VkImage image = 0x%X, VkMemoryRequirements* pMemoryRequirements = 0x%X)",
		      device, image, pMemoryRequirements);

		*pMemoryRequirements = Cast(image)->GetMemoryRequirements();
	}

	void GetImageSparseMemoryRequirements(VkDevice device, VkImage image, uint32_t* pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements* pSparseMemoryRequirements)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void GetPhysicalDeviceSparseImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkSampleCountFlagBits samples, VkImageUsageFlags usage, VkImageTiling tiling, uint32_t* pPropertyCount, VkSparseImageFormatProperties* pProperties)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	VkResult QueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	VkResult CreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFence* pFence)
	{
		TRACE("(VkDevice device = 0x%X, const VkFenceCreateInfo* pCreateInfo = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X, VkFence* pFence = 0x%X)",
		      device, pCreateInfo, pAllocator, pFence);

		if(pCreateInfo->pNext)
		{
			UNIMPLEMENTED();
		}

		*pFence = *(new (pAllocator) Fence(pCreateInfo));

		return VK_SUCCESS;
	}

	void DestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks* pAllocator)
	{
		TRACE("(VkDevice device = 0x%X, VkFence fence = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X)",
		      device, fence, pAllocator);
	
		destroy(fence, pAllocator);
	}

	VkResult ResetFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	VkResult GetFenceStatus(VkDevice device, VkFence fence)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	VkResult WaitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll, uint64_t timeout)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	VkResult CreateASemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	void DestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	VkResult CreateAnEvent(VkDevice device, const VkEventCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkEvent* pEvent)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	void DestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks* pAllocator)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	VkResult GetEventStatus(VkDevice device, VkEvent event)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	VkResult SetEvent(VkDevice device, VkEvent event)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	VkResult ResetEvent(VkDevice device, VkEvent event)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	VkResult CreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	void DestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	VkResult GetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, size_t dataSize, void* pData, VkDeviceSize stride, VkQueryResultFlags flags)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	VkResult CreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer)
	{
		TRACE("(VkDevice device = 0x%X, const VkBufferCreateInfo* pCreateInfo = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X, VkBuffer* pBuffer = 0x%X)",
		      device, pCreateInfo, pAllocator, pBuffer);

		if(pCreateInfo->pNext)
		{
			UNIMPLEMENTED();
		}

		*pBuffer = *(new (pAllocator) Buffer(pAllocator, pCreateInfo));

		return VK_SUCCESS;
	}

	void DestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator)
	{
		TRACE("(VkDevice device = 0x%X, VkBuffer buffer = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X)",
		      device, buffer, pAllocator);

		destroy(buffer, pAllocator);
	}

	VkResult CreateBufferView(VkDevice device, const VkBufferViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBufferView* pView)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	void DestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks* pAllocator)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	VkResult CreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImage* pImage)
	{
		TRACE("(VkDevice device = 0x%X, const VkImageCreateInfo* pCreateInfo = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X, VkImage* pImage = 0x%X)",
		      device, pCreateInfo, pAllocator, pImage);

		if(pCreateInfo->pNext)
		{
			UNIMPLEMENTED();
		}

		*pImage = *(new (pAllocator) Image(pAllocator, pCreateInfo));

		return VK_SUCCESS;
	}

	void DestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator)
	{
		TRACE("(VkDevice device = 0x%X, VkImage image = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X)",
		      device, image, pAllocator);

		destroy(image, pAllocator);
	}

	void GetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource* pSubresource, VkSubresourceLayout* pLayout)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	VkResult CreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImageView* pView)
	{
		TRACE("(VkDevice device = 0x%X, const VkImageViewCreateInfo* pCreateInfo = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X, VkImageView* pView = 0x%X)",
		      device, pCreateInfo, pAllocator, pView);

		if(pCreateInfo->pNext || pCreateInfo->flags)
		{
			UNIMPLEMENTED();
		}

		*pView = *(new (pAllocator) ImageView(pCreateInfo));

		return VK_SUCCESS;
	}

	void DestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	VkResult CreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule)
	{
		TRACE("(VkDevice device = 0x%X, const VkShaderModuleCreateInfo* pCreateInfo = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X, VkShaderModule* pShaderModule = 0x%X)",
		      device, pCreateInfo, pAllocator, pShaderModule);

		if(pCreateInfo->pNext || pCreateInfo->flags)
		{
			UNIMPLEMENTED();
		}

		*pShaderModule = *(new (pAllocator) ShaderModule(pAllocator, pCreateInfo));

		return VK_SUCCESS;
	}

	void DestroyShaderModule(VkDevice device, VkShaderModule shaderModule, const VkAllocationCallbacks* pAllocator)
	{
		TRACE("(VkDevice device = 0x%X, VkShaderModule shaderModule = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X)",
		      device, shaderModule, pAllocator);

		destroy(shaderModule, pAllocator);
	}

	VkResult CreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	void DestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache, const VkAllocationCallbacks* pAllocator)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	VkResult GetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache, size_t* pDataSize, void* pData)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	VkResult MergePipelineCaches(VkDevice device, VkPipelineCache dstCache, uint32_t srcCacheCount, const VkPipelineCache* pSrcCaches)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	VkResult CreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkGraphicsPipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines)
	{
		TRACE("(VkDevice device = 0x%X, VkPipelineCache pipelineCache = 0x%X, uint32_t createInfoCount = %d, const VkGraphicsPipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator = 0x%X, VkPipeline* pPipelines = 0x%X)",
		      device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);

		if(pipelineCache != VK_NULL_HANDLE)
		{
			UNIMPLEMENTED();
		}

		for(uint32_t i = 0; i < createInfoCount; ++i)
		{
			pPipelines[i] = *(new (pAllocator) Pipeline(pAllocator, pCreateInfos[i]));
		}

		return VK_SUCCESS;
	}

	VkResult CreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkComputePipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines)
	{
		TRACE("(VkDevice device = 0x%X, VkPipelineCache pipelineCache = 0x%X, uint32_t createInfoCount = %d, const VkComputePipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator = 0x%X, VkPipeline* pPipelines = 0x%X)",
			device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);

		if(pipelineCache != VK_NULL_HANDLE)
		{
			UNIMPLEMENTED();
		}

		for(uint32_t i = 0; i < createInfoCount; ++i)
		{
			pPipelines[i] = *(new (pAllocator) Pipeline(pAllocator, pCreateInfos[i]));
		}

		return VK_SUCCESS;
	}

	void DestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator)
	{
		TRACE("(VkDevice device = 0x%X, VkPipeline pipeline = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X)",
		      device, pipeline, pAllocator);

		destroy(pipeline, pAllocator);
	}

	VkResult CreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout)
	{
		TRACE("(VkDevice device = 0x%X, const VkPipelineLayoutCreateInfo* pCreateInfo = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X, VkPipelineLayout* pPipelineLayout = 0x%X)",
		      device, pCreateInfo, pAllocator, pPipelineLayout);

		if(pCreateInfo->pNext || pCreateInfo->flags)
		{
			UNIMPLEMENTED();
		}

		*pPipelineLayout = *(new (pAllocator) PipelineLayout(pAllocator, pCreateInfo));

		return VK_SUCCESS;
	}

	void DestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout, const VkAllocationCallbacks* pAllocator)
	{
		TRACE("(VkDevice device = 0x%X, VkPipelineLayout pipelineLayout = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X)",
		      device, pipelineLayout, pAllocator);

		destroy(pipelineLayout, pAllocator);
	}

	VkResult CreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSampler* pSampler)
	{
		TRACE("(VkDevice device = 0x%X, const VkSamplerCreateInfo* pCreateInfo = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X, VkSampler* pSampler = 0x%X)",
		      device, pCreateInfo, pAllocator, pSampler);

		if(pCreateInfo->pNext || pCreateInfo->flags)
		{
			UNIMPLEMENTED();
		}

		*pSampler = *(new (pAllocator) Sampler(device, pCreateInfo));

		return VK_SUCCESS;
	}

	void DestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator)
	{
		TRACE("(VkDevice device = 0x%X, VkSampler sampler = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X)",
		      device, sampler, pAllocator);

		destroy(sampler, pAllocator);
	}

	VkResult CreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorSetLayout* pSetLayout)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	void DestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, const VkAllocationCallbacks* pAllocator)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	VkResult CreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	void DestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, const VkAllocationCallbacks* pAllocator)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	VkResult ResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags flags)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	VkResult AllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo, VkDescriptorSet* pDescriptorSets)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	VkResult FreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	void UpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount, const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount, const VkCopyDescriptorSet* pDescriptorCopies)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	VkResult CreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer)
	{
		TRACE("(VkDevice device = 0x%X, const VkFramebufferCreateInfo* pCreateInfo = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X, VkFramebuffer* pFramebuffer = 0x%X)",
		      device, pCreateInfo, pAllocator, pFramebuffer);

		if(pCreateInfo->pNext || pCreateInfo->flags)
		{
			UNIMPLEMENTED();
		}

		*pFramebuffer = *(new (pAllocator) Framebuffer(pAllocator, pCreateInfo));

		return VK_SUCCESS;
	}

	void DestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks* pAllocator)
	{
		TRACE("(VkDevice device = 0x%X, VkFramebuffer framebuffer = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X)");

		destroy(framebuffer, pAllocator);
	}

	VkResult CreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass)
	{
		TRACE("(VkDevice device = 0x%X, const VkRenderPassCreateInfo* pCreateInfo = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X, VkRenderPass* pRenderPass = 0x%X)",
		      device, pCreateInfo, pAllocator, pRenderPass);

		if(pCreateInfo->pNext || pCreateInfo->flags)
		{
			UNIMPLEMENTED();
		}

		*pRenderPass = *(new (pAllocator) RenderPass(pAllocator, pCreateInfo));

		return VK_SUCCESS;
	}

	void DestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator)
	{
		TRACE("(VkDevice device = 0x%X, VkRenderPass renderPass = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X)",
		      device, renderPass, pAllocator);

		destroy(renderPass, pAllocator);
	}

	void GetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass, VkExtent2D* pGranularity)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	VkResult CreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool)
	{
		TRACE("(VkDevice device = 0x%X, const VkCommandPoolCreateInfo* pCreateInfo = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X, VkCommandPool* pCommandPool = 0x%X)",
		      device, pCreateInfo, pAllocator, pCommandPool);

		if(pCreateInfo->pNext)
		{
			UNIMPLEMENTED();
		}

		*pCommandPool = *(new (pAllocator) CommandPool(pCreateInfo));

		return VK_SUCCESS;
	}

	void DestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator)
	{
		TRACE("(VkDevice device = 0x%X, VkCommandPool commandPool = 0x%X, const VkAllocationCallbacks* pAllocator = 0x%X)",
		      device, commandPool, pAllocator);

		destroy(commandPool, pAllocator);
	}

	VkResult ResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	VkResult AllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo, VkCommandBuffer* pCommandBuffers)
	{
		TRACE("(VkDevice device = 0x%X, const VkCommandBufferAllocateInfo* pAllocateInfo = 0x%X, VkCommandBuffer* pCommandBuffers = 0x%X)",
		      device, pAllocateInfo, pCommandBuffers);

		if(pAllocateInfo->pNext)
		{
			UNIMPLEMENTED();
		}

		Cast(pAllocateInfo->commandPool)->allocateCommandBuffers(
			 pAllocateInfo->level, pAllocateInfo->commandBufferCount, pCommandBuffers);

		return VK_SUCCESS;
	}

	void FreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers)
	{
		TRACE("(VkDevice device = 0x%X, VkCommandPool commandPool = 0x%X, uint32_t commandBufferCount = %d, const VkCommandBuffer* pCommandBuffers = 0x%X)",
		      device, commandPool, commandBufferCount, pCommandBuffers);

		Cast(commandPool)->freeCommandBuffers(commandBufferCount, pCommandBuffers);
	}

	VkResult BeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo)
	{
		TRACE("(VkCommandBuffer commandBuffer = 0x%X, const VkCommandBufferBeginInfo* pBeginInfo = 0x%X)",
		      commandBuffer, pBeginInfo);

		if(pBeginInfo->pNext)
		{
			UNIMPLEMENTED();
		}

		Cast(commandBuffer)->begin(pBeginInfo->flags, pBeginInfo->pInheritanceInfo);

		return VK_SUCCESS;
	}

	VkResult EndCommandBuffer(VkCommandBuffer commandBuffer)
	{
		TRACE("(VkCommandBuffer commandBuffer = 0x%X)", commandBuffer);

		Cast(commandBuffer)->end();

		return VK_SUCCESS;
	}

	VkResult ResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	void CmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline)
	{
		TRACE("(VkCommandBuffer commandBuffer = 0x%X, VkPipelineBindPoint pipelineBindPoint = %d, VkPipeline pipeline = 0x%X)",
		      commandBuffer, pipelineBindPoint, pipeline);

		Cast(commandBuffer)->bindPipeline(pipelineBindPoint, pipeline);
	}

	void CmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkViewport* pViewports)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void CmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount, const VkRect2D* pScissors)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void CmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void CmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void CmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4])
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void CmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void CmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t compareMask)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void CmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t writeMask)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void CmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t reference)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void CmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void CmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void CmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets)
	{
		TRACE("(VkCommandBuffer commandBuffer = 0x%X, uint32_t firstBinding = %d, uint32_t bindingCount = %d, const VkBuffer* pBuffers = 0x%X, const VkDeviceSize* pOffsets = 0x%X)",
		      commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);

		Cast(commandBuffer)->bindVertexBuffers(firstBinding, bindingCount, pBuffers, pOffsets);
	}

	void CmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
	{
		TRACE("(VkCommandBuffer commandBuffer = 0x%X, uint32_t vertexCount = %d, uint32_t instanceCount = %d, uint32_t firstVertex = %d, uint32_t firstInstance = %d)",
		      commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);

		Cast(commandBuffer)->draw(vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void CmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
	{
		TRACE("(VkCommandBuffer commandBuffer = 0x%X, uint32_t indexCount = %d, uint32_t instanceCount = %d, uint32_t firstIndex = %d, int32_t vertexOffset = %d, uint32_t firstInstance = %d)",
		      commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);

		Cast(commandBuffer)->drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}

	void CmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride)
	{
		TRACE("(VkCommandBuffer commandBuffer = 0x%X, VkBuffer buffer = 0x%X, VkDeviceSize offset = %d, uint32_t drawCount = %d, uint32_t stride = %d)",
		      commandBuffer, buffer, offset, drawCount, stride);

		Cast(commandBuffer)->drawIndirect(buffer, offset, drawCount, stride);
	}

	void CmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride)
	{
		TRACE("(VkCommandBuffer commandBuffer = 0x%X, VkBuffer buffer = 0x%X, VkDeviceSize offset = %d, uint32_t drawCount = %d, uint32_t stride = %d)",
		      commandBuffer, buffer, offset, drawCount, stride);

		Cast(commandBuffer)->drawIndexedIndirect(buffer, offset, drawCount, stride);
	}

	void CmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void CmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void CmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy* pRegions)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void CmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy* pRegions)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void CmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit* pRegions, VkFilter filter)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void CmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy* pRegions)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void CmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions)
	{
		TRACE("(VkCommandBuffer commandBuffer = 0x%X, VkImage srcImage = 0x%X, VkImageLayout srcImageLayout = %d, VkBuffer dstBuffer = 0x%X, uint32_t regionCount = %d, const VkBufferImageCopy* pRegions = 0x%X)",
		      commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);

		Cast(commandBuffer)->copyImageToBuffer(srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
	}

	void CmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize dataSize, const void* pData)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void CmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size, uint32_t data)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void CmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearColorValue* pColor, uint32_t rangeCount, const VkImageSubresourceRange* pRanges)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void CmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount, const VkImageSubresourceRange* pRanges)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void CmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount, const VkClearAttachment* pAttachments, uint32_t rectCount, const VkClearRect* pRects)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void CmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageResolve* pRegions)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void CmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void CmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void CmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void CmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers)
	{
		TRACE("(VkCommandBuffer commandBuffer = 0x%X, VkPipelineStageFlags srcStageMask = 0x%X, VkPipelineStageFlags dstStageMask = 0x%X, VkDependencyFlags dependencyFlags = %d, uint32_t memoryBarrierCount = %d, onst VkMemoryBarrier* pMemoryBarriers = 0x%X,"
		      " uint32_t bufferMemoryBarrierCount = %d, const VkBufferMemoryBarrier* pBufferMemoryBarriers = 0x%X, uint32_t imageMemoryBarrierCount = %d, const VkImageMemoryBarrier* pImageMemoryBarriers = 0x%X)",
		      commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);

		Cast(commandBuffer)->pipelineBarrier(srcStageMask, dstStageMask, dependencyFlags,
		                                     memoryBarrierCount, pMemoryBarriers,
		                                     bufferMemoryBarrierCount, pBufferMemoryBarriers,
		                                     imageMemoryBarrierCount, pImageMemoryBarriers);
	}

	void CmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void CmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void CmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void CmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkQueryPool queryPool, uint32_t query)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void CmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride, VkQueryResultFlags flags)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void CmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* pValues)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void CmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin, VkSubpassContents contents)
	{
		TRACE("(VkCommandBuffer commandBuffer = 0x%X, const VkRenderPassBeginInfo* pRenderPassBegin = 0x%X, VkSubpassContents contents = %d)",
		      commandBuffer, pRenderPassBegin, contents);

		if(pRenderPassBegin->pNext)
		{
			UNIMPLEMENTED();
		}

		Cast(commandBuffer)->beginRenderPass(pRenderPassBegin->renderPass, pRenderPassBegin->framebuffer,
		                                     pRenderPassBegin->renderArea, pRenderPassBegin->clearValueCount,
		                                     pRenderPassBegin->pClearValues, contents);
	}

	void CmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void CmdEndRenderPass(VkCommandBuffer commandBuffer)
	{
		TRACE("(VkCommandBuffer commandBuffer = 0x%X)", commandBuffer);

		Cast(commandBuffer)->endRenderPass();
	}

	void CmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	VkResult EnumerateInstanceVersion(uint32_t* pApiVersion)
	{
		TRACE("()");
		*pApiVersion = VK_API_VERSION_1_1;
		return VK_SUCCESS;
	}

	VkResult BindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	VkResult BindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	void GetDeviceGroupPeerMemoryFeatures(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex, uint32_t remoteDeviceIndex, VkPeerMemoryFeatureFlags* pPeerMemoryFeatures)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void CmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void CmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	VkResult EnumeratePhysicalDeviceGroups(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount, VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	void GetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void GetBufferMemoryRequirements2(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void GetImageSparseMemoryRequirements2(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo, uint32_t* pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements2* pSparseMemoryRequirements)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void GetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures)
	{
		TRACE("(VkPhysicalDevice physicalDevice = 0x%X, VkPhysicalDeviceFeatures2* pFeatures = 0x%X)", physicalDevice, pFeatures);

		GetPhysicalDeviceFeatures(physicalDevice, &(pFeatures->features));
	}

	void GetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2* pProperties)
	{
		TRACE("(VkPhysicalDevice physicalDevice = 0x%X, VkPhysicalDeviceProperties2* pProperties = 0x%X)", physicalDevice, pProperties);

		GetPhysicalDeviceProperties(physicalDevice, &(pProperties->properties));
	}

	void GetPhysicalDeviceFormatProperties2(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties2* pFormatProperties)
	{
		TRACE("(VkPhysicalDevice physicalDevice = 0x%X, VkFormat format = %d, VkFormatProperties2* pFormatProperties = 0x%X)",
		      physicalDevice, format, pFormatProperties);

		GetPhysicalDeviceFormatProperties(physicalDevice, format, &(pFormatProperties->formatProperties));
	}

	VkResult GetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo, VkImageFormatProperties2* pImageFormatProperties)
	{
		TRACE("(VkPhysicalDevice physicalDevice = 0x%X, const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo = 0x%X, VkImageFormatProperties2* pImageFormatProperties = 0x%X)",
		      physicalDevice, pImageFormatInfo, pImageFormatProperties);

		return GetPhysicalDeviceImageFormatProperties(physicalDevice,
		                                              pImageFormatInfo->format,
		                                              pImageFormatInfo->type,
		                                              pImageFormatInfo->tiling,
		                                              pImageFormatInfo->usage,
		                                              pImageFormatInfo->flags,
		                                              &(pImageFormatProperties->imageFormatProperties));
	}

	void GetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties2* pQueueFamilyProperties)
	{
		TRACE("(VkPhysicalDevice physicalDevice = 0x%X, uint32_t* pQueueFamilyPropertyCount = 0x%X, VkQueueFamilyProperties2* pQueueFamilyProperties = 0x%X)",
			physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);

		GetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount, &(pQueueFamilyProperties->queueFamilyProperties));
	}

	void GetPhysicalDeviceMemoryProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties2* pMemoryProperties)
	{
		TRACE("(VkPhysicalDevice physicalDevice = 0x%X, VkPhysicalDeviceMemoryProperties2* pMemoryProperties = 0x%X)", physicalDevice, pMemoryProperties);

		GetPhysicalDeviceMemoryProperties(physicalDevice, &(pMemoryProperties->memoryProperties));
	}

	void GetPhysicalDeviceSparseImageFormatProperties2(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo, uint32_t* pPropertyCount, VkSparseImageFormatProperties2* pProperties)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void TrimCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void GetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2* pQueueInfo, VkQueue* pQueue)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	VkResult CreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSamplerYcbcrConversion* pYcbcrConversion)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	void DestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion, const VkAllocationCallbacks* pAllocator)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	VkResult CreateDescriptorUpdateTemplate(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	void DestroyDescriptorUpdateTemplate(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const VkAllocationCallbacks* pAllocator)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void UpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void* pData)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void GetPhysicalDeviceExternalBufferProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo, VkExternalBufferProperties* pExternalBufferProperties)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void GetPhysicalDeviceExternalFenceProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo, VkExternalFenceProperties* pExternalFenceProperties)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void GetPhysicalDeviceExternalSemaphoreProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo, VkExternalSemaphoreProperties* pExternalSemaphoreProperties)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void GetDescriptorSetLayoutSupport(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo, VkDescriptorSetLayoutSupport* pSupport)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void DestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks* pAllocator)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	VkResult GetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, VkSurfaceKHR surface, VkBool32* pSupported)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	VkResult GetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR* pSurfaceCapabilities)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	VkResult GetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	VkResult GetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	VkResult CreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	void DestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	VkResult GetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount, VkImage* pSwapchainImages)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	VkResult AcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	VkResult QueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	VkResult GetDeviceGroupPresentCapabilitiesKHR(VkDevice device, VkDeviceGroupPresentCapabilitiesKHR* pDeviceGroupPresentCapabilities)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	VkResult GetDeviceGroupSurfacePresentModesKHR(VkDevice device, VkSurfaceKHR surface, VkDeviceGroupPresentModeFlagsKHR* pModes)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	VkResult GetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pRectCount, VkRect2D* pRects)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	VkResult AcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo, uint32_t* pImageIndex)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	VkResult GetPhysicalDeviceDisplayPropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkDisplayPropertiesKHR* pProperties)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	VkResult GetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkDisplayPlanePropertiesKHR* pProperties)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	VkResult GetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice, uint32_t planeIndex, uint32_t* pDisplayCount, VkDisplayKHR* pDisplays)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	VkResult GetDisplayModePropertiesKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, uint32_t* pPropertyCount, VkDisplayModePropertiesKHR* pProperties)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	VkResult CreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, const VkDisplayModeCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDisplayModeKHR* pMode)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	VkResult GetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode, uint32_t planeIndex, VkDisplayPlaneCapabilitiesKHR* pCapabilities)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	VkResult CreateDisplayPlaneSurfaceKHR(VkInstance instance, const VkDisplaySurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	VkResult CreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount, const VkSwapchainCreateInfoKHR* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchains)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	void GetPhysicalDeviceFeatures2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void GetPhysicalDeviceProperties2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2* pProperties)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void GetPhysicalDeviceFormatProperties2KHR(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties2* pFormatProperties)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	VkResult GetPhysicalDeviceImageFormatProperties2KHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo, VkImageFormatProperties2* pImageFormatProperties)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	void GetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties2* pQueueFamilyProperties)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void GetPhysicalDeviceMemoryProperties2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties2* pMemoryProperties)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void GetPhysicalDeviceSparseImageFormatProperties2KHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo, uint32_t* pPropertyCount, VkSparseImageFormatProperties2* pProperties)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void GetDeviceGroupPeerMemoryFeaturesKHR(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex, uint32_t remoteDeviceIndex, VkPeerMemoryFeatureFlags* pPeerMemoryFeatures)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void CmdSetDeviceMaskKHR(VkCommandBuffer commandBuffer, uint32_t deviceMask)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void CmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void TrimCommandPoolKHR(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	VkResult EnumeratePhysicalDeviceGroupsKHR(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount, VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	void GetPhysicalDeviceExternalBufferPropertiesKHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo, VkExternalBufferProperties* pExternalBufferProperties)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	VkResult GetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR* pGetFdInfo, int* pFd)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	VkResult GetMemoryFdPropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, int fd, VkMemoryFdPropertiesKHR* pMemoryFdProperties)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	void GetPhysicalDeviceExternalSemaphorePropertiesKHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo, VkExternalSemaphoreProperties* pExternalSemaphoreProperties)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	VkResult ImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR* pImportSemaphoreFdInfo)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	VkResult GetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR* pGetFdInfo, int* pFd)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	void CmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount, const VkWriteDescriptorSet* pDescriptorWrites)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void CmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer, VkDescriptorUpdateTemplate descriptorUpdateTemplate, VkPipelineLayout layout, uint32_t set, const void* pData)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	VkResult CreateDescriptorUpdateTemplateKHR(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	void DestroyDescriptorUpdateTemplateKHR(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const VkAllocationCallbacks* pAllocator)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void UpdateDescriptorSetWithTemplateKHR(VkDevice device, VkDescriptorSet descriptorSet, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void* pData)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	VkResult CreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2KHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	void CmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin, const VkSubpassBeginInfoKHR* pSubpassBeginInfo)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void CmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfoKHR* pSubpassBeginInfo, const VkSubpassEndInfoKHR* pSubpassEndInfo)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void CmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfoKHR* pSubpassEndInfo)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	VkResult GetSwapchainStatusKHR(VkDevice device, VkSwapchainKHR swapchain)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	void GetPhysicalDeviceExternalFencePropertiesKHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo, VkExternalFenceProperties* pExternalFenceProperties)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	VkResult ImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR* pImportFenceFdInfo)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	VkResult GetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR* pGetFdInfo, int* pFd)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	VkResult GetPhysicalDeviceSurfaceCapabilities2KHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo, VkSurfaceCapabilities2KHR* pSurfaceCapabilities)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	VkResult GetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo, uint32_t* pSurfaceFormatCount, VkSurfaceFormat2KHR* pSurfaceFormats)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	VkResult GetPhysicalDeviceDisplayProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkDisplayProperties2KHR* pProperties)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	VkResult GetPhysicalDeviceDisplayPlaneProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkDisplayPlaneProperties2KHR* pProperties)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	VkResult GetDisplayModeProperties2KHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, uint32_t* pPropertyCount, VkDisplayModeProperties2KHR* pProperties)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	VkResult GetDisplayPlaneCapabilities2KHR(VkPhysicalDevice physicalDevice, const VkDisplayPlaneInfo2KHR* pDisplayPlaneInfo, VkDisplayPlaneCapabilities2KHR* pCapabilities)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	void GetImageMemoryRequirements2KHR(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void GetBufferMemoryRequirements2KHR(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void GetImageSparseMemoryRequirements2KHR(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo, uint32_t* pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements2* pSparseMemoryRequirements)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	VkResult CreateSamplerYcbcrConversionKHR(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSamplerYcbcrConversion* pYcbcrConversion)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	void DestroySamplerYcbcrConversionKHR(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion, const VkAllocationCallbacks* pAllocator)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	VkResult BindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	VkResult BindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	void GetDescriptorSetLayoutSupportKHR(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo, VkDescriptorSetLayoutSupport* pSupport)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void CmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void CmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void DebugReportMessageEXT(VkInstance instance, VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	VkResult DebugMarkerSetObjectTagEXT(VkDevice device, const VkDebugMarkerObjectTagInfoEXT* pTagInfo)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	VkResult DebugMarkerSetObjectNameEXT(VkDevice device, const VkDebugMarkerObjectNameInfoEXT* pNameInfo)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	void CmdDebugMarkerBeginEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT* pMarkerInfo)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void CmdDebugMarkerEndEXT(VkCommandBuffer commandBuffer)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void CmdDebugMarkerInsertEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT* pMarkerInfo)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void CmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void CmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	VkResult GetShaderInfoAMD(VkDevice device, VkPipeline pipeline, VkShaderStageFlagBits shaderStage, VkShaderInfoTypeAMD infoType, size_t* pInfoSize, void* pInfo)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	VkResult GetPhysicalDeviceExternalImageFormatPropertiesNV(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags, VkExternalMemoryHandleTypeFlagsNV externalHandleType, VkExternalImageFormatPropertiesNV* pExternalImageFormatProperties)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	void CmdBeginConditionalRenderingEXT(VkCommandBuffer commandBuffer, const VkConditionalRenderingBeginInfoEXT* pConditionalRenderingBegin)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void CmdEndConditionalRenderingEXT(VkCommandBuffer commandBuffer)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void CmdProcessCommandsNVX(VkCommandBuffer commandBuffer, const VkCmdProcessCommandsInfoNVX* pProcessCommandsInfo)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void CmdReserveSpaceForCommandsNVX(VkCommandBuffer commandBuffer, const VkCmdReserveSpaceForCommandsInfoNVX* pReserveSpaceInfo)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	VkResult CreateIndirectCommandsLayoutNVX(VkDevice device, const VkIndirectCommandsLayoutCreateInfoNVX* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkIndirectCommandsLayoutNVX* pIndirectCommandsLayout)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	void DestroyIndirectCommandsLayoutNVX(VkDevice device, VkIndirectCommandsLayoutNVX indirectCommandsLayout, const VkAllocationCallbacks* pAllocator)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	VkResult CreateObjectTableNVX(VkDevice device, const VkObjectTableCreateInfoNVX* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkObjectTableNVX* pObjectTable)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	void DestroyObjectTableNVX(VkDevice device, VkObjectTableNVX objectTable, const VkAllocationCallbacks* pAllocator)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	VkResult RegisterObjectsNVX(VkDevice device, VkObjectTableNVX objectTable, uint32_t objectCount, const VkObjectTableEntryNVX* const* ppObjectTableEntries, const uint32_t* pObjectIndices)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	VkResult UnregisterObjectsNVX(VkDevice device, VkObjectTableNVX objectTable, uint32_t objectCount, const VkObjectEntryTypeNVX* pObjectEntryTypes, const uint32_t* pObjectIndices)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	void GetPhysicalDeviceGeneratedCommandsPropertiesNVX(VkPhysicalDevice physicalDevice, VkDeviceGeneratedCommandsFeaturesNVX* pFeatures, VkDeviceGeneratedCommandsLimitsNVX* pLimits)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void CmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkViewportWScalingNV* pViewportWScalings)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	VkResult ReleaseDisplayEXT(VkPhysicalDevice physicalDevice, VkDisplayKHR display)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	VkResult GetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceCapabilities2EXT* pSurfaceCapabilities)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	VkResult DisplayPowerControlEXT(VkDevice device, VkDisplayKHR display, const VkDisplayPowerInfoEXT* pDisplayPowerInfo)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	VkResult RegisterDeviceEventEXT(VkDevice device, const VkDeviceEventInfoEXT* pDeviceEventInfo, const VkAllocationCallbacks* pAllocator, VkFence* pFence)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	VkResult RegisterDisplayEventEXT(VkDevice device, VkDisplayKHR display, const VkDisplayEventInfoEXT* pDisplayEventInfo, const VkAllocationCallbacks* pAllocator, VkFence* pFence)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	VkResult GetSwapchainCounterEXT(VkDevice device, VkSwapchainKHR swapchain, VkSurfaceCounterFlagBitsEXT counter, uint64_t* pCounterValue)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	VkResult GetRefreshCycleDurationGOOGLE(VkDevice device, VkSwapchainKHR swapchain, VkRefreshCycleDurationGOOGLE* pDisplayTimingProperties)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	VkResult GetPastPresentationTimingGOOGLE(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pPresentationTimingCount, VkPastPresentationTimingGOOGLE* pPresentationTimings)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	void CmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle, uint32_t discardRectangleCount, const VkRect2D* pDiscardRectangles)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void SetHdrMetadataEXT(VkDevice device, uint32_t swapchainCount, const VkSwapchainKHR* pSwapchains, const VkHdrMetadataEXT* pMetadata)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	VkResult SetDebugUtilsObjectNameEXT(VkDevice device, const VkDebugUtilsObjectNameInfoEXT* pNameInfo)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	VkResult SetDebugUtilsObjectTagEXT(VkDevice device, const VkDebugUtilsObjectTagInfoEXT* pTagInfo)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	void QueueBeginDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void QueueEndDebugUtilsLabelEXT(VkQueue queue)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void QueueInsertDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void CmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void CmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void CmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pMessenger)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks* pAllocator)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void SubmitDebugUtilsMessageEXT(VkInstance instance, VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void CmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer, const VkSampleLocationsInfoEXT* pSampleLocationsInfo)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void GetPhysicalDeviceMultisamplePropertiesEXT(VkPhysicalDevice physicalDevice, VkSampleCountFlagBits samples, VkMultisamplePropertiesEXT* pMultisampleProperties)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	VkResult CreateValidationCacheEXT(VkDevice device, const VkValidationCacheCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkValidationCacheEXT* pValidationCache)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	void DestroyValidationCacheEXT(VkDevice device, VkValidationCacheEXT validationCache, const VkAllocationCallbacks* pAllocator)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	VkResult MergeValidationCachesEXT(VkDevice device, VkValidationCacheEXT dstCache, uint32_t srcCacheCount, const VkValidationCacheEXT* pSrcCaches)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	VkResult GetValidationCacheDataEXT(VkDevice device, VkValidationCacheEXT validationCache, size_t* pDataSize, void* pData)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	VkResult GetMemoryHostPointerPropertiesEXT(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, const void* pHostPointer, VkMemoryHostPointerPropertiesEXT* pMemoryHostPointerProperties)
	{
		TRACE("()");
		UNIMPLEMENTED();
		return VK_SUCCESS;
	}

	void CmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void CmdSetCheckpointNV(VkCommandBuffer commandBuffer, const void* pCheckpointMarker)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}

	void GetQueueCheckpointDataNV(VkQueue queue, uint32_t* pCheckpointDataCount, VkCheckpointDataNV* pCheckpointData)
	{
		TRACE("()");
		UNIMPLEMENTED();
	}
}