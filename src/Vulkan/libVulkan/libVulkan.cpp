#include <stdio.h>
#include "libVulkan.hpp"
#include "Context.h"
#include "utils.h"
#include "Device.h"
#include <assert.h>
#include <windows.h>

namespace vulkan
{
	PFN_vkVoidFunction GetInstanceProcAddr(VkInstance instance, const char* pName)
	{
		return reinterpret_cast<PFN_vkVoidFunction>(vkutils::findEntry(pName));
	}

	PFN_vkVoidFunction GetDeviceProcAddr(VkDevice device, const char* pName)
	{
		return reinterpret_cast<PFN_vkVoidFunction>(vkutils::findEntry(pName));
	}

	VkResult EnumerateInstanceExtensionProperties(const char * pLayerName, uint32_t * pPropertyCount, VkExtensionProperties * pProperties)
	{
		if (pProperties == NULL)
		{
			*pPropertyCount = GLOBAL_EXT_SIZE;
			return VK_SUCCESS;
		}

		*pPropertyCount = min(*pPropertyCount, GLOBAL_EXT_SIZE);
		memcpy(pProperties, global_ext, *pPropertyCount);

		if (*pPropertyCount < GLOBAL_EXT_SIZE)
		{
			return VK_INCOMPLETE;
		}

		return VK_SUCCESS;
	}

	VkResult CreateInstance(const VkInstanceCreateInfo * pCreateInfo, const VkAllocationCallbacks * pAllocator, VkInstance * pInstance)
	{
		Instance *instance = nullptr;

		assert(pCreateInfo->sType == VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO);

		uint32_t ClientVersion;

		// Per spec, must not be 0
		if (pCreateInfo->pApplicationInfo && pCreateInfo->pApplicationInfo->apiVersion != 0)
		{
			ClientVersion = pCreateInfo->pApplicationInfo->apiVersion;
		}
		else
		{
			ClientVersion = VK_MAKE_VERSION(1, 0, 0);
		}

		if (VK_MAKE_VERSION(1, 0, 0) > ClientVersion)
		{
			return VK_ERROR_INCOMPATIBLE_DRIVER;
		}

		for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; ++i)
		{
			bool foundExt = false;

			for (uint32_t j = 0; j < GLOBAL_EXT_SIZE; ++j)
			{
				if (strcmp(pCreateInfo->ppEnabledExtensionNames[i], global_ext[j].extensionName) == 0)
				{
					foundExt = true;
					break;
				}
			}

			if (!foundExt)
			{
				return VK_ERROR_EXTENSION_NOT_PRESENT;
			}
		}


		instance = reinterpret_cast<vulkan::Instance *>(vkutils::Alloc(&default_alloc, pAllocator, sizeof(*instance), ALIGNMENT,
			VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE));

		if (!instance)
		{
			return VK_ERROR_OUT_OF_HOST_MEMORY;
		}

		if (pAllocator)
		{
			instance->alloc = *pAllocator;
		}
			
		else
		{
			instance->alloc = default_alloc;
		}

		instance->apiVersion = ClientVersion;
		// We do not yet have a valid physical device
		instance->physicalDeviceNum = -1;

		*pInstance = Instance_to_handle(instance);


		return VK_SUCCESS;
	}

	void DestroyInstance(VkInstance instance, const VkAllocationCallbacks * pAllocator)
	{
		GET_FROM_HANDLE(Instance, myInstance, instance);

		if (!myInstance)
		{
			return;
		}

		// Physical device stuff goes here later

		vkutils::Free(&myInstance->alloc, instance);
	}

	void InitPhysicalDevice(struct PhysicalDevice *pDevice, struct Instance *pInstance)
	{
		pDevice->instance = pInstance;
		pDevice->name = "SwiftShaderDevice";
		pDevice->memory = { 0 };
	}

	VkResult EnumeratePhysicalDevices(VkInstance instance, uint32_t * pPhysicalDeviceCount, VkPhysicalDevice * pPhysicalDevices)
	{
		GET_FROM_HANDLE(Instance, myInstance, instance);

		vkutils::outarray<VkPhysicalDevice *> out;
		vkutils::OutArrayInit(&out.base, pPhysicalDevices, pPhysicalDeviceCount);

		if (myInstance->physicalDeviceNum < 0) 
		{
			if (pPhysicalDevices == NULL)
			{
				*pPhysicalDeviceCount = 1;
				myInstance->physicalDeviceNum = *pPhysicalDeviceCount;
			}

			return VK_SUCCESS;
		}

		if (myInstance->physicalDeviceNum > 0)
		{
			assert(myInstance->physicalDeviceNum == 1);

			InitPhysicalDevice(&myInstance->physicalDevice, myInstance);

			for(auto element = vkutils::GetNext(&out.base, sizeof(out.meta[0])); element != NULL; element = NULL)
			{
				  element = PhysicalDevice_to_handle(&myInstance->physicalDevice);
				  *pPhysicalDevices = reinterpret_cast<VkPhysicalDevice>(element);
			}
			 
 			return VK_SUCCESS;
		}

		return VK_INCOMPLETE;
	}

	void GetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice, uint32_t * pQueueFamilyPropertyCount, VkQueueFamilyProperties * pQueueFamilyProperties)
	{
		vkutils::outarray<VkQueueFamilyProperties> out;
		vkutils::OutArrayInit(&out.base, pQueueFamilyProperties, pQueueFamilyPropertyCount);

		for (VkQueueFamilyProperties* element = reinterpret_cast<VkQueueFamilyProperties  *>(vkutils::GetNext(&out.base, sizeof(out.meta[0]))); element != NULL; element = NULL)
		{
			*element = queue_family_properties;
			*pQueueFamilyProperties = *element;
		}
	}

	void GetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures * pFeatures)
	{
		GET_FROM_HANDLE(PhysicalDevice, pDevice, physicalDevice);

		*pFeatures = VkPhysicalDeviceFeatures {
			    true,
				true,
				true,
				true,
				true,
				true,
				true,
				true,
				true,
				true,
			  true,
				true,
			  true,
				true,
				false,
				true,
				true,
				true,
				true,
				true,
				false,
				false, 
				true,
				true,
				true,
				true,
				true,
				true,
				true,
				false,
				false,
				true,
				true,
				true,
				true
		};
	}

	void GetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties * pProperties)
	{
		GET_FROM_HANDLE(PhysicalDevice, pDevice, physicalDevice);

		VkSampleCountFlags sampleCounts = 0;

		VkPhysicalDeviceLimits limits = {
			(1 << 14),
		    (1 << 14),
			(1 << 11),
			(1 << 14),
			(1 << 11),
			128 * 1024 * 1024,
			(1ul << 27),
			16,
			128,
			UINT32_MAX,
			64 * 1024,
			64,
			0,
			8,
			64,
			64,
			64,
			64,
			64,
			64,
			128,
			256,
			256,
			8,
			256,
			8,
			256,
			256,
			256,
		    31,
			31,
			2047,
			2048,
			128,
			64,
			32,
			128,
			128,
			128,
			2048,
			128,
			128,
			32,
			64,
			128,
			256,
			1024,
			128,
			8,
			1,
			8,
			32768,
			{ 65535, 65535, 65535 },
			16,
			{ 16, 16, 16,},
		    4,
		    4,
	        4,
		    UINT32_MAX,
		    UINT32_MAX,
		    16,
		    16,
		    16,
	        { (1 << 14), (1 << 14) },
		    { INT16_MIN, INT16_MAX },
		    13,
		    4096,
		    1,
		    16,
		    4,
		   -8,
		    7,
		   -32,
		    31,
		   -0.5,
		    0.4375,
		    4,
		   (1 << 14),
		   (1 << 14),
		   (1 << 11),
		   sampleCounts,
		   sampleCounts,
		   sampleCounts,
		   sampleCounts,
		   8, 
		   sampleCounts,
		   VK_SAMPLE_COUNT_1_BIT,
		   sampleCounts,
		   sampleCounts,
		   VK_SAMPLE_COUNT_1_BIT,
		   1,
		   false,
		   60,
		   8,
		   8,
		   8,
		   1,
		 { 0.125, 255.875 },
		 { 0.0, 7.9921875 },
		 (1.0 / 8.0),
		 (1.0 / 128.0),
		 false,
		 true,
		 128,
		 128,
		 64
		};

		*pProperties = VkPhysicalDeviceProperties{
			VK_MAKE_VERSION(1, 0, 0),
			1,
			0x1111,
			1,
			VK_PHYSICAL_DEVICE_TYPE_CPU,
			"SwiftShader Device",
			{0}, 
			limits,
			{0}
		};

		strcpy(pProperties->deviceName, pDevice->name);
	}

	VkResult EnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char * pLayerName, uint32_t * pPropertyCount, VkExtensionProperties * pProperties)
	{
		if (pProperties == NULL)
		{
			*pPropertyCount = DEVICE_EXT_SIZE;
			return VK_SUCCESS;
		}

		*pPropertyCount = min(*pPropertyCount, DEVICE_EXT_SIZE);
		memcpy(pProperties, device_extensions, *pPropertyCount);

		if (*pPropertyCount < DEVICE_EXT_SIZE)
		{
			return VK_INCOMPLETE;
		}

		return VK_SUCCESS;

	}

	VkResult CreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo * pCreateInfo, const VkAllocationCallbacks * pAllocator, VkDevice * pDevice)
	{
		GET_FROM_HANDLE(PhysicalDevice, physDevice, physicalDevice);
		struct Device *device;

		assert(pCreateInfo->sType == VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO);

		for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; ++i)
		{
			bool found = false; 

			for (uint32_t j = 0; j < DEVICE_EXT_SIZE; ++j)
			{
				if (!strcmp(pCreateInfo->ppEnabledExtensionNames[i], device_extensions[j].extensionName))
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				return VK_ERROR_EXTENSION_NOT_PRESENT;
			}
		}

		device = reinterpret_cast<Device *>(vkutils::Alloc(&physDevice->instance->alloc, pAllocator, sizeof(*device), 8, VK_SYSTEM_ALLOCATION_SCOPE_DEVICE));

		if (device == NULL)
		{
			return VK_ERROR_OUT_OF_HOST_MEMORY;
		}

		device->instance = physDevice->instance;

		if (pAllocator)
		{
			device->alloc = *pAllocator;
		}
		else
		{
			device->alloc = physDevice->instance->alloc; 
		}

		device->robustBufferAccess = false;
		*pDevice = Device_to_handle(device); 

		return VK_SUCCESS;
	}

	void GetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties * pMemoryProperties)
	{
		GET_FROM_HANDLE(PhysicalDevice, physDevice, physicalDevice);

		pMemoryProperties->memoryTypeCount = physDevice->memory.type_count;

		for (uint32_t i = 0; i < physDevice->memory.type_count; ++i)
		{
			pMemoryProperties->memoryTypes[i] = {
				physDevice->memory.types[i].propertyFlags, 
				physDevice->memory.types[i].heapIndex
			};
		}

		pMemoryProperties->memoryHeapCount = physDevice->memory.heap_count;

		for (uint32_t i = 0; i < physDevice->memory.heap_count; ++i)
		{
			pMemoryProperties->memoryHeaps[i] = {
				physDevice->memory.heaps[i].size, 
				physDevice->memory.heaps[i].flags
			};
		}
	}


}
