#ifndef PIPELINE_H
#define PIPELINE_H

#include "Instance.h"

namespace vulkan
{
	struct PipelineLayout
	{
		// Not really sure what to put in here since nothing gets passed
		uint32_t setCount;

		struct VertData
		{
			VkFormat format;
			uint32_t stride;
		}vertData;

		struct ViewportData
		{
			float x;
			float y;
			float width;
			float height;
			float minDepth;
			float maxDepth;
		}viewportData;
	};

	struct Pipeline
	{
		Device *device;
		PipelineLayout *layout;
		VkSubpassDescription *subpass;
	};
}
#endif
