#ifndef BUFFER_H
#define BUFFER_H
#include "Instance.h"
#include "Image.h"
#include "CommandAllocator.h"

namespace vulkan
{
	struct Buffer
	{
		Device *device;
		VkDeviceSize size;
		VkBufferUsageFlags usage;
		VkDeviceSize offset;
	};

	struct BufferView
	{
		VkBufferViewCreateFlags    flags;
		VkBuffer                   buffer;
		VkFormat                   format;
		VkDeviceSize               offset;
		VkDeviceSize               range;
	};

	struct VertexBinding
	{
		Buffer buffer;
		VkDeviceSize offset;
	};

	struct Framebuffer
	{
		uint32_t width;
		uint32_t height;
		uint32_t layers;

		uint32_t attachmentCount;
		struct ImageView attachments;
		struct RenderPass *renderpass;
	};

	struct CommandPool
	{
		VkAllocationCallbacks alloc;
		uint32_t queueFamilyIndex;
		struct CommandBuffer *buffers;
	};

	struct CommandBuffer
	{
		Device *device;
		CommandPool *pool;
		VkCommandBufferUsageFlags usageFlags;
		VkCommandBufferLevel level;
		backend::CommandAllocator *cmdAllocator;
		backend::CommandIterator *cmdIterator;

		struct State
		{
			struct Pipeline *pipeline;
			struct Framebuffer *framebuffer;
			struct RenderPass *renderPass;
			VertexBinding *vertBind;
			VkRect2D renderArea;
			VkAttachmentDescription *attachments;
		}state;
	};

	struct Fence
	{
		bool submitted;
	};

	struct Semaphore
	{
		VkSemaphoreCreateFlags flags;
	};

	struct Event
	{
		VkEventCreateFlags flags;
	};

	struct QueryPool
	{
		VkQueryPoolCreateFlags           flags;
		VkQueryType                      queryType;
		uint32_t                         queryCount;
		VkQueryPipelineStatisticFlags    pipelineStatistics;
	};

	struct PipelineCache
	{
		VkPipelineCacheCreateFlags    flags;
		size_t                        initialDataSize;
		const void*                   pInitialData;
	};

	struct DescriptorSetLayout
	{
		VkDescriptorSetLayoutCreateFlags       flags;
		uint32_t                               bindingCount;
		const VkDescriptorSetLayoutBinding*    pBindings;
	};

	struct DescriptorPool {
		VkDescriptorPoolCreateFlags    flags;
		uint32_t                       maxSets;
		uint32_t                       poolSizeCount;
		const VkDescriptorPoolSize*    pPoolSizes;
	};

	struct DescriptorSet
	{
		const VkDescriptorSetLayout*    pSetLayouts;
	};
}
#endif
