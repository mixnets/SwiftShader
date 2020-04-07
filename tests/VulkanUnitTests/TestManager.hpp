// Utilities for writing Vulkan unittests
// Test writers should be able to use this to create tests for Vulkan without
// having to write boilerplate code.

#include <vulkan/vulkan_core.h>

#include <memory>
#include <vector>

class Driver;
class Device;

// CreateInstance

// Creates a VKInstance with no extensions
void CreateBasicVkInstance(Driver driver, VkInstance *instance);

struct BasicPipeline
{
	VkDescriptorSetLayout descriptorSetLayout;
	VkPipelineLayout pipelineLayout;
	VkPipeline pipeline;
};

void CreateBasicPipeline(std::unique_ptr<Device> device,
                         std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings,
                         BasicPipeline *pipeline);

struct TextureFormat
{
	VkFormat format;
	size_t size;  // in bytes

	TextureFormat()
	    : format(VK_FORMAT_UNDEFINED)
	    , size(0)
};

struct TextureData
{
	// How many texels wide and tall is the texture?
	int width;
	int height;
	// How many bytes does the texture require
	size_t size;
	TextureFormat format;

	// Actual texture data.
	vector<uint8_t> bytes;

	TextureData()
	    : width(0)
	    , height(0)
	    , size(0)
	    , format()
	    , bytes();
};

// An all in one package for tracking all the information relevant to buffers.
struct BufferData
{
	VkBufferCreateInfo info;
	VkBuffer buffer;
	size_t memoryOffset;
	size_t size;
	VkBufferUsageFlags flags;
	VkDeviceMemory deviceMemory;
	VkMemoryPropertyFlags flags;

	// This points to host accesiible memory allocated by Vulkan.
	void *memory;

	// Constructor for a basic buffer. A user needs to know how many bytes large their buffer is, and how it will be used.
	//
	// The most common uses are transfering data to or from the device, storing
	// information accessed by descriptor sets, and storing index and vertex
	// information. There are other uses detailed in the Khronos spec for
	// VkBufferUsageFlagBits.
	//
	// You can use a bitwise OR "|" to specify multiple uses for a buffer.
	BufferData(size_t size, VkBufferUsageFlags flags);
	// If you need more control over the buffer, you can specify your own CreateInfo
	BufferData(VkBufferCreateInfo info);
};

class TestManager
{
public:
	TestManager(bool loadSystem = false);
	TestManager(const char *path);

	void CreateVkInstance();
	void CreateVkInstance(VkInstanceCreateInfo *createInfo);

	void CreateComputeDevice();

	TextureData CreateTextureData(int height, int width, TextureFormat format);

	~TestManager();

private:
	VkInstance instance;
	Driver driver;
	VkPhysicalDevice physicalDevice;
	VkDevice device;
};

const int NUM_FORMATS = 158;
extern TextureFormat textureFormats[NUM_FORMATS];
