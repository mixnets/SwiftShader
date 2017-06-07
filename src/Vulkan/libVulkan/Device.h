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
}
#endif // ! DEVICE_H

