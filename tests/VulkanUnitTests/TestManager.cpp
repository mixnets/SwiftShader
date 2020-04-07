#include "TestManager.hpp"

/**** Vulkan Code ****/
// The code in this section shows how touse the Vulkan API and provides a simpler way for test authors to invoke the API functions they need and track their data.
//
// These functions are ordered in the sequence they would be called by a test user.
// If you add a new function, please make sure that it preserves this ordering logic.
//
// There are sections for functions that are not Vulkan API related below.

/** Create a TestManager and load a driver **/
// We first need a way to talk to the Vulkan API. We're going to do that through
// a Driver object stored in TestManager.
//
// By default we load SwiftShader's driver, but a user may select the system's driver (available only on Linux) or specify a path to the driver they wish to load.
void TestManager::TestManager(bool loadSystemDriver /*= false*/)
{
	if(loadSystemDriver)
	{
		ASSERT_TRUE(driver.loadSystem());
	}
	else
	{
		ASSERT_TRUE(driver.loadSwiftShader());
	}
	instance = VK_NULL_HANDLE;
}

void TestManager::TestManager(const char *path)
{
	ASSERT_TRUE(driver.load(path));
	instance = VK_NULL_HANDLE;
}

/*** Create a Vulkan instance ***/
// VkInstances are handles to a collection of Vulkan state. Unlike OpenGL, Vulkan
// has no global state. So we need a VkInstance handle to track and modify the state.
// Users can either create a basic instance with no extensions or application info,
// or specify their own instance info.
void TestManager::CreateVkInstance()
{
	// Whenever you call any vkCreate* function, you need to pass it a CreateInfo struct.
	// CreateInfo structs are linked lists that specify creation parameters and
	// extension info to the driver.
	//
	// For a basic VkInstance, you don't need any additional information.
	const VkInstanceCreateInfo createInfo = {
		VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,  // sType
		nullptr,                                 // pNext
		0,                                       // flags
		nullptr,                                 // pApplicationInfo
		0,                                       // enabledLayerCount
		nullptr,                                 // ppEnabledLayerNames
		0,                                       // enabledExtensionCount
		nullptr,                                 // ppEnabledExtensionNames
	};

	// Tell the driver to create an instance, and store it in a variable tracked by the test manager.
	VK_ASSERT(driver.vkCreateInstance(&createInfo, nullptr, &instance));

	ASSERT_TRUE(driver.resolve(instance));
}

void TestManager::CreateVkInstance(VkInstanceCreateInfo *createInfo)
{
	VK_ASSERT(driver.vkCreateInstance(createInfo, nullptr, &instance));
	ASSERT_TRUE(driver.resolve(instance));
}

/** Create a logical device **/
// Now that we have a driver and a vulkan instance, we need something that
// lets us talk to the hardware.
//
// VkQueueFlags specifies what queue family we're looking for. Normally this
// will either be VK_QUEUE_GRAPHICS_BIT or VK_QUEUE_COMPUTE_BIT.
void TestManager::CreateDevice(VkQueueFlags flags)
{
	// There are two kinds of devices that Vulkan talks about:
	//    * Physical Devices
	//    * Logical Devices
	// Physical devices represent the actual hardware and its properties. A
	// logical device represents the interface between our application and the
	// physical device.

	// A system may have multiple physical devices. So we'll need to select one.
	//
	// The method to do this is:
	//   * Get each available physical device
	//   * Get each device's queue family properties. Queues
	//     are the objects Vulkan executes commands on, and
	//     queue families are a collection of queues that
	//     can all execute the same type of commands. A
	//     family's properties tell us about the commands
	//     they can execute.
	//   * Find the first physical device that has a queue
	//     family with the desired properties.
	std::vector<VkPhysicalDevices> physicalDevices;
	uint32_t count;

	// Get a count of the number of physical devices
	VK_ASSERT(driver->vkEnumeratePhysicalDevices(instance, &count, 0));
	out.resize(count);
	// Actually retrieve each physical device
	VK_ASSERT(driver->vkEnumeratePhysicalDevices(instance, &count, out.data()));

	// Enumerate over all the physical devices and select the first one that
	// has the properties we want.
	for(VkPhysicalDevice deviceIterator : physicalDevices)
	{
		int queueFamilyIndex = GetQueueFamilyIndex(physicalDevice, flags);
		if(queueFamilyIndex >= 0)
		{
			this->physicalDevice = deviceIterator;
			break;
		}
	}

	// Set up the data we need to pass to Vulkan in order to create a logical device
	const float queuePriority = 1.0f;
	const VkDeviceQueueCreateInfo deviceQueueCreateInfo = {
		VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,  // sType
		nullptr,                                     // pNext
		0,                                           // flags
		(uint32_t)queueFamilyIndex,                  // queueFamilyIndex
		1,                                           // queueCount
		&queuePriority,                              // pQueuePriorities
	};

	const VkDeviceCreateInfo deviceCreateInfo = {
		VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,  // sType
		nullptr,                               // pNext
		0,                                     // flags
		1,                                     // queueCreateInfoCount
		&deviceQueueCreateInfo,                // pQueueCreateInfos
		0,                                     // enabledLayerCount
		nullptr,                               // ppEnabledLayterNames
		0,                                     // enabledExtensionCount
		nullptr,                               // ppEnabledExtensionNames
		nullptr,                               // pEnabledFeatures
	};

	// Finally, create the logical device.
	VK_ASSERT(driver->vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &this->device));
}

// Helper function for finding the first queue family that has all the features
// we need.
int TestManager::GetQueueFamilyIndex(VkPhysicalDevice device, VkQueueFlags flags)
{
	// As if often the case when enumerating anything with Vulkan, we start by
	// counting how many of the things there are, then copying all those things
	// into a std::vector.
	std::vector<VkQueueFamilyProperties> properties;
	uint32_t count = 0;
	driver->vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
	out.resize(count)
	    driver->vkGetPhysicalDeviceQueueFamilyProperties(device, &count, properties.data());

	for(uint32_t i = 0; i < properties.size(); i++)
	{
		// queueFlags are bits that specify the features available within a queue.
		if((properies[i].queueFlags & flags) != 0)
		{
			// Return the index of the queue family that suits our needs. Vulkan
			// talks about queue families by their index, rather than passing
			// queue family objects around to the user.
			return static_cast<int>(i);
		}
	}

	return -1;
}

void TestManager::CreateBuffer(BufferData bufferData)
{
	VkPhysicalDeviceMemoryProperties properties;
	driver->vkGetPhysicalDeviceMemoryProperties(this->physicalDevice, &properties);

	// Look for the memory that has the type we need and is large enough to fit our buffer.
	// Then allocate enough space of that memory type to store our buffer in.
	for(uint32_t type = 0; type < properties.memoryTypeCount; type++)
	{
		if((bufferData.flags & properties.memoryTypes[type].propertyFlags) == 0)
		{
			continue;  // Wrong type
		}

		if(bufferData.size > properties.memoryHeaps[properties.memoryTypes[type].heapIndex].size)
		{
			continue;  // Too small
		}

		const VkMemoryAllocateInfo info = {
			VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,  // sType
			nullptr,                                 // pNext
			bufferData.size,                         // allocationSize
			type,                                    // memoryTypeIndex
		};

		VK_ASSERT(driver->vkAllocateMemory(this->device, &info, 0, &bufferData.deviceMemory));
	}

	// Now that we've allocated the memory, we need to actually map the device side
	// memory to our buffer.
	VK_ASSERT(driver->vkMapMemory(this->device, bufferData.deviceMemory, bufferData.memoryOffset, bufferData.size, 0, &bufferData.memory));
}

/**** Utility Code ****/
// These functions help manage our data and tests.

BufferData::BufferData(size_t size, VkBufferUsageFlags flags)
{
	this->size = size;
	this->memoryOffset = 0;
	this->flags = flags;
	info.flags = 0;
	info.pNext = nullptr;
	info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	info.size = size;
	info.usage = flags;
	// Sharing mode specifies if other queue families wil be using this buffer.
	// Our basic buffer should only need to be used within a single queue family.
	info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	// If a user does need a buffer that is shared across queue families,
	// then they need to also specify those queue families in the following
	// variables.
	info.pQueueFamilyIndices = nullptr;
	info.queueFamilyIndexCount = 0;

	memory = nullptr;
}

BufferData::BufferData(VkBufferCreateInfo info)
{
	this->info = info;
	this->size = info.size;
	this->flags = info.flags;
	this->memoryOffset = 0;
	memory = nullptr;
}

// Create a checkerboard pattern for a texture that fits in the given format.
TextureData TestManager::CreateTextureData(int height, int width, TextureFormat format)
{
	TextureData textureData;

	textureData.height = height;
	textureData.width = width;
	textureData.size = height * width * format.size;
	textureData.bytes.resize(textureData.size);
	std::vector<uint8_t> texel(format.size);

	// Fill the texel with arbitrary data. Even though some formats don't have
	// consistent sizes for each channel, it's fine to treat them as if they do
	// here. We just need something non-zero to make sure data is actually
	// being passed and handled by the driver.
	for(int channel = 0; channel < format.size; channel++)
	{
		// 97 and 53 are primes chosen at random
		texel[channel] = (97 * channel + 53) % 256;
	}

	// This fills the texture data with a checkerboard pattern.
	// Using unsigned integers to ensure that there's nothing weird going on with modulo arithmetic

	uint8_t *data = textureData.bytes.data();
	const uint32_t gridSize = 128;
	const uint32_t halfGridSize = gridSize / 2;
	for(uint32_t y = 0; y < height; y++)
	{
		for(uint32_t x = 0; x < width; x++)
		{
			// What percent of the default value will be expressed for this texel?
			float colorGain = 0.5f;
			// Logic to create checkerboard pattern.
			uint32_t modX = x % gridSize;
			uint32_t modY = y % gridSize;
			if(modX < halfGridSize && modY < halfGridSize)
			{
				colorGain = 1.0f;
			}
			else if(modX >= halfGridSize && modY >= halfGridSize)
			{
				colorGain = 1.0f;
			}

			// Place the texel into the texture's data.
			for(int channel = 0; channel < format.size; channel++)
			{
				*data = static_cast<uint8_t>(texel[channel] * colorGain);
				data++;
			}
		}
	}

	return textureData;
}

// Used for enumerating through all supported formats to ensure there are no oddities in SwiftShader's handling of them.
TextureFormat textureFormats[NUM_FORMATS] = {
	// {VkFormat format, size_t sizeInBytes},
	{ VK_FORMAT_R4G4_UNORM_PACK8, 1 },
	{ VK_FORMAT_R4G4B4A4_UNORM_PACK16, 2 },
	{ VK_FORMAT_B4G4R4A4_UNORM_PACK16, 2 },
	{ VK_FORMAT_R5G6B5_UNORM_PACK16, 2 },
	{ VK_FORMAT_B5G6R5_UNORM_PACK16, 2 },
	{ VK_FORMAT_R5G5B5A1_UNORM_PACK16, 2 },
	{ VK_FORMAT_B5G5R5A1_UNORM_PACK16, 2 },
	{ VK_FORMAT_A1R5G5B5_UNORM_PACK16, 2 },
	{ VK_FORMAT_R8_UNORM, 1 },
	{ VK_FORMAT_R8_SNORM, 1 },
	{ VK_FORMAT_R8_USCALED, 1 },
	{ VK_FORMAT_R8_SSCALED, 1 },
	{ VK_FORMAT_R8_UINT, 1 },
	{ VK_FORMAT_R8_SINT, 1 },
	{ VK_FORMAT_R8_SRGB, 1 },
	{ VK_FORMAT_R8G8_UNORM, 2 },
	{ VK_FORMAT_R8G8_SNORM, 2 },
	{ VK_FORMAT_R8G8_USCALED, 2 },
	{ VK_FORMAT_R8G8_SSCALED, 2 },
	{ VK_FORMAT_R8G8_UINT, 2 },
	{ VK_FORMAT_R8G8_SINT, 2 },
	{ VK_FORMAT_R8G8_SRGB, 2 },
	{ VK_FORMAT_R8G8B8_UNORM, 3 },
	{ VK_FORMAT_R8G8B8_SNORM, 3 },
	{ VK_FORMAT_R8G8B8_USCALED, 3 },
	{ VK_FORMAT_R8G8B8_SSCALED, 3 },
	{ VK_FORMAT_R8G8B8_UINT, 3 },
	{ VK_FORMAT_R8G8B8_SINT, 3 },
	{ VK_FORMAT_R8G8B8_SRGB, 3 },
	{ VK_FORMAT_B8G8R8_UNORM, 3 },
	{ VK_FORMAT_B8G8R8_SNORM, 3 },
	{ VK_FORMAT_B8G8R8_USCALED, 3 },
	{ VK_FORMAT_B8G8R8_SSCALED, 3 },
	{ VK_FORMAT_B8G8R8_UINT, 3 },
	{ VK_FORMAT_B8G8R8_SINT, 3 },
	{ VK_FORMAT_B8G8R8_SRGB, 3 },
	{ VK_FORMAT_R8G8B8A8_UNORM, 4 },
	{ VK_FORMAT_R8G8B8A8_SNORM, 4 },
	{ VK_FORMAT_R8G8B8A8_USCALED, 4 },
	{ VK_FORMAT_R8G8B8A8_SSCALED, 4 },
	{ VK_FORMAT_R8G8B8A8_UINT, 4 },
	{ VK_FORMAT_R8G8B8A8_SINT, 4 },
	{ VK_FORMAT_R8G8B8A8_SRGB, 4 },
	{ VK_FORMAT_B8G8R8A8_UNORM, 4 },
	{ VK_FORMAT_B8G8R8A8_SNORM, 4 },
	{ VK_FORMAT_B8G8R8A8_USCALED, 4 },
	{ VK_FORMAT_B8G8R8A8_SSCALED, 4 },
	{ VK_FORMAT_B8G8R8A8_UINT, 4 },
	{ VK_FORMAT_B8G8R8A8_SINT, 4 },
	{ VK_FORMAT_B8G8R8A8_SRGB, 4 },
	{ VK_FORMAT_A8B8G8R8_UNORM_PACK32, 4 },
	{ VK_FORMAT_A8B8G8R8_SNORM_PACK32, 4 },
	{ VK_FORMAT_A8B8G8R8_USCALED_PACK32, 4 },
	{ VK_FORMAT_A8B8G8R8_SSCALED_PACK32, 4 },
	{ VK_FORMAT_A8B8G8R8_UINT_PACK32, 4 },
	{ VK_FORMAT_A8B8G8R8_SINT_PACK32, 4 },
	{ VK_FORMAT_A8B8G8R8_SRGB_PACK32, 4 },
	{ VK_FORMAT_A2R10G10B10_UNORM_PACK32, 4 },
	{ VK_FORMAT_A2R10G10B10_SNORM_PACK32, 4 },
	{ VK_FORMAT_A2R10G10B10_USCALED_PACK32, 4 },
	{ VK_FORMAT_A2R10G10B10_SSCALED_PACK32, 4 },
	{ VK_FORMAT_A2R10G10B10_UINT_PACK32, 4 },
	{ VK_FORMAT_A2R10G10B10_SINT_PACK32, 4 },
	{ VK_FORMAT_A2B10G10R10_UNORM_PACK32, 4 },
	{ VK_FORMAT_A2B10G10R10_SNORM_PACK32, 4 },
	{ VK_FORMAT_A2B10G10R10_USCALED_PACK32, 4 },
	{ VK_FORMAT_A2B10G10R10_SSCALED_PACK32, 4 },
	{ VK_FORMAT_A2B10G10R10_UINT_PACK32, 4 },
	{ VK_FORMAT_A2B10G10R10_SINT_PACK32, 4 },
	{ VK_FORMAT_R16_UNORM, 2 },
	{ VK_FORMAT_R16_SNORM, 2 },
	{ VK_FORMAT_R16_USCALED, 2 },
	{ VK_FORMAT_R16_SSCALED, 2 },
	{ VK_FORMAT_R16_UINT, 2 },
	{ VK_FORMAT_R16_SINT, 2 },
	{ VK_FORMAT_R16_SFLOAT, 2 },
	{ VK_FORMAT_R16G16_UNORM, 4 },
	{ VK_FORMAT_R16G16_SNORM, 4 },
	{ VK_FORMAT_R16G16_USCALED, 4 },
	{ VK_FORMAT_R16G16_SSCALED, 4 },
	{ VK_FORMAT_R16G16_UINT, 4 },
	{ VK_FORMAT_R16G16_SINT, 4 },
	{ VK_FORMAT_R16G16_SFLOAT, 2 },
	{ VK_FORMAT_R16G16B16_UNORM, 6 },
	{ VK_FORMAT_R16G16B16_SNORM, 6 },
	{ VK_FORMAT_R16G16B16_USCALED, 6 },
	{ VK_FORMAT_R16G16B16_SSCALED, 6 },
	{ VK_FORMAT_R16G16B16_UINT, 6 },
	{ VK_FORMAT_R16G16B16_SINT, 6 },
	{ VK_FORMAT_R16G16B16_SFLOAT, 6 },
	{ VK_FORMAT_R16G16B16A16_UNORM, 8 },
	{ VK_FORMAT_R16G16B16A16_SNORM, 8 },
	{ VK_FORMAT_R16G16B16A16_USCALED, 8 },
	{ VK_FORMAT_R16G16B16A16_SSCALED, 8 },
	{ VK_FORMAT_R16G16B16A16_UINT, 8 },
	{ VK_FORMAT_R16G16B16A16_SINT, 8 },
	{ VK_FORMAT_R16G16B16A16_SFLOAT, 8 },
	{ VK_FORMAT_R32_UINT, 4 },
	{ VK_FORMAT_R32_SINT, 4 },
	{ VK_FORMAT_R32_SFLOAT, 4 },
	{ VK_FORMAT_R32G32_UINT, 8 },
	{ VK_FORMAT_R32G32_SINT, 8 },
	{ VK_FORMAT_R32G32_SFLOAT, 8 },
	{ VK_FORMAT_R32G32B32_UINT, 12 },
	{ VK_FORMAT_R32G32B32_SINT, 12 },
	{ VK_FORMAT_R32G32B32_SFLOAT, 12 },
	{ VK_FORMAT_R32G32B32A32_UINT, 16 },
	{ VK_FORMAT_R32G32B32A32_SINT, 16 },
	{ VK_FORMAT_R32G32B32A32_SFLOAT, 16 },
	{ VK_FORMAT_R64_UINT, 8 },
	{ VK_FORMAT_R64_SINT, 8 },
	{ VK_FORMAT_R64_SFLOAT, 8 },
	{ VK_FORMAT_R64G64_UINT, 16 },
	{ VK_FORMAT_R64G64_SINT, 16 },
	{ VK_FORMAT_R64G64_SFLOAT, 16 },
	{ VK_FORMAT_R64G64B64_UINT, 24 },
	{ VK_FORMAT_R64G64B64_SINT, 24 },
	{ VK_FORMAT_R64G64B64_SFLOAT, 24 },
	{ VK_FORMAT_R64G64B64A64_UINT, 32 },
	{ VK_FORMAT_R64G64B64A64_SINT, 32 },
	{ VK_FORMAT_R64G64B64A64_SFLOAT, 32 },
	{ VK_FORMAT_B10G11R11_UFLOAT_PACK32, 4 },
	{ VK_FORMAT_E5B9G9R9_UFLOAT_PACK32, 4 },
	{ VK_FORMAT_D16_UNORM, 2 },
	{ VK_FORMAT_X8_D24_UNORM_PACK32, 4 },
	{ VK_FORMAT_D32_SFLOAT, 4 },
	{ VK_FORMAT_S8_UINT, 1 },
	{ VK_FORMAT_D32_SFLOAT_S8_UINT, 4 },
	{ VK_FORMAT_BC1_RGB_UNORM_BLOCK, 2 },
	{ VK_FORMAT_BC1_RGB_SRGB_BLOCK, 2 },
	{ VK_FORMAT_BC1_RGBA_UNORM_BLOCK, 2 },
	{ VK_FORMAT_BC1_RGBA_SRGB_BLOCK, 2 },
	{ VK_FORMAT_BC2_UNORM_BLOCK, 4 },
	{ VK_FORMAT_BC2_SRGB_BLOCK, 4 },
	{ VK_FORMAT_BC3_UNORM_BLOCK, 4 },
	{ VK_FORMAT_BC3_SRGB_BLOCK, 4 },
	{ VK_FORMAT_BC4_UNORM_BLOCK, 2 },
	{ VK_FORMAT_BC4_SNORM_BLOCK, 2 },
	{ VK_FORMAT_BC5_UNORM_BLOCK, 4 },
	{ VK_FORMAT_BC5_SNORM_BLOCK, 4 },
	{ VK_FORMAT_BC6H_UFLOAT_BLOCK, 4 },
	{ VK_FORMAT_BC6H_SFLOAT_BLOCK, 4 },
	{ VK_FORMAT_BC7_UNORM_BLOCK, 4 },
	{ VK_FORMAT_BC7_SRGB_BLOCK, 4 },
	{ VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK, 2 },
	{ VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK, 2 },
	{ VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK, 2 },
	{ VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK, 2 },
	{ VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK, 4 },
	{ VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK, 4 },
	{ VK_FORMAT_EAC_R11_UNORM_BLOCK, 2 },
	{ VK_FORMAT_EAC_R11_SNORM_BLOCK, 2 },
	{ VK_FORMAT_EAC_R11G11_UNORM_BLOCK, 4 },
	{ VK_FORMAT_EAC_R11G11_SNORM_BLOCK, 4 },
	{ VK_FORMAT_ASTC_4x4_UNORM_BLOCK, 4 },
	{ VK_FORMAT_ASTC_4x4_SRGB_BLOCK, 4 },
	{ VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM, 1 },
	{ VK_FORMAT_G8_B8R8_2PLANE_420_UNORM, 1 },
};
