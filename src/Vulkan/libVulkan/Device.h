#ifndef  DEVICE_H
#define DEVICE_H

#include <Vulkan\vulkan.h>
#include <Vulkan\vk_platform.h>

#define GLOBAL_EXT_SIZE 4

namespace vulkan
{
	static const VkExtensionProperties global_ext[] = {
		{
			VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
			1,
		},
		{
			VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME,
			1,
		},
		{
			VK_KHR_SURFACE_EXTENSION_NAME,
			25,
		},
		{
			VK_KHX_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME,
			1,
		},
		{
			VK_KHX_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME,
			1,
		}
	};

	static void *
		default_alloc_func(void *pUserData, size_t size, size_t align,
			VkSystemAllocationScope allocationScope)
	{
		return malloc(size);
	}

	static void *
		default_realloc_func(void *pUserData, void *pOriginal, size_t size,
			size_t align, VkSystemAllocationScope allocationScope)
	{
		return realloc(pOriginal, size);
	}

	static void
		default_free_func(void *pUserData, void *pMemory)
	{
		free(pMemory);
	}

	static const VkAllocationCallbacks default_alloc = {
		NULL,
		default_alloc_func,
		default_realloc_func,
		default_free_func,
	};
}
#endif // ! DEVICE_H

