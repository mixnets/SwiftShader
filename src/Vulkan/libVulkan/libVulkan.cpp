#include <stdio.h>
#include "libVulkan.hpp"
#include "Context.h"
#include "utils.h"
#include "Device.h"

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
		Instance *instance; 

		return VkResult();
	}
}
