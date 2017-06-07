#include <stdio.h>
#include "libVulkan.hpp"
#include "Context.h"
#include "utils.h"
#include "Device.h"
#include <assert.h>

#define ALIGNMENT 8

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
		if (pProperties == NULL) {
			*pPropertyCount = GLOBAL_EXT_SIZE;
			return VK_SUCCESS;
		}

		if (*pPropertyCount < GLOBAL_EXT_SIZE)
			return VK_INCOMPLETE;

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

		// Physical device stuff goes here

		vkutils::Free(&myInstance->alloc, instance);
	}

	VkResult EnumeratePhysicalDevices(VkInstance instance, uint32_t * pPhysicalDeviceCount, VkPhysicalDevice * pPhysicalDevices)
	{
		return VK_SUCCESS;
	}
}
