// Copyright 2020 The SwiftShader Authors. All Rights Reserved.
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

#include "../VulkanUnitTests/Device.hpp"
#include "../VulkanUnitTests/Driver.hpp"

#include "benchmark/benchmark.h"

#include <cassert>
#include <vector>

struct Format
{
	VkFormat format;
	const char *name;
};

const std::vector<Format> formats = {
	{ VK_FORMAT_R8G8B8A8_UNORM, "VK_FORMAT_R8G8B8A8_UNORM" },
	{ VK_FORMAT_R32_SFLOAT, "VK_FORMAT_R32_SFLOAT" },
};

class VulkanBenchmark : public benchmark::Fixture
{
public:
	void SetUp(::benchmark::State &state)
	{
		format = formats[state.range(0)].format;
		state.counters[formats[state.range(0)].name] = (double)format;

		assert(driver.loadSwiftShader());

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

		VkInstance instance = VK_NULL_HANDLE;
		driver.vkCreateInstance(&createInfo, nullptr, &instance);
		driver.resolve(instance);

		const float queuePrioritory = 1.0f;
		const VkDeviceQueueCreateInfo deviceQueueCreateInfo = {
			VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,  // sType
			nullptr,                                     // pNext
			0,                                           // flags
			queueFamilyIndex,                            // queueFamilyIndex
			1,                                           // queueCount
			&queuePrioritory,                            // pQueuePriorities
		};

		const VkDeviceCreateInfo deviceCreateInfo = {
			VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,  // sType
			nullptr,                               // pNext
			0,                                     // flags
			1,                                     // queueCreateInfoCount
			&deviceQueueCreateInfo,                // pQueueCreateInfos
			0,                                     // enabledLayerCount
			nullptr,                               // ppEnabledLayerNames
			0,                                     // enabledExtensionCount
			nullptr,                               // ppEnabledExtensionNames
			nullptr,                               // pEnabledFeatures
		};

		uint32_t count = 1;
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		driver.vkEnumeratePhysicalDevices(instance, &count, &physicalDevice);

		driver.vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device);

		driver.vkGetDeviceQueue(device, queueFamilyIndex, 0, &queue);
	}

	void TearDown(const ::benchmark::State &state) {}

	VkFormat format;

	Driver driver;

	VkInstance instance = VK_NULL_HANDLE;
	VkDevice device = VK_NULL_HANDLE;
	VkQueue queue = VK_NULL_HANDLE;
	const uint32_t queueFamilyIndex = 0;
};

BENCHMARK_DEFINE_F(VulkanBenchmark, Clear)
(benchmark::State &state)
{
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.flags = 0;
	imageInfo.pNext = nullptr;
	imageInfo.format = format;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
	imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.extent = { 1024, 1024, 1 };
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;

	VkImage image = VK_NULL_HANDLE;
	driver.vkCreateImage(device, &imageInfo, nullptr, &image);

	VkMemoryRequirements memoryRequirements = {};
	driver.vkGetImageMemoryRequirements(device, image, &memoryRequirements);

	VkMemoryAllocateInfo allocateInfo = {};
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = memoryRequirements.size;
	allocateInfo.memoryTypeIndex = 0;

	VkDeviceMemory memory = VK_NULL_HANDLE;
	driver.vkAllocateMemory(device, &allocateInfo, nullptr, &memory);

	driver.vkBindImageMemory(device, image, memory, 0);

	const VkCommandPoolCreateInfo commandPoolCreateInfo = {
		VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,  // sType
		nullptr,                                     // pNext
		0,                                           // flags
		queueFamilyIndex,                            // queueFamilyIndex
	};

	VkCommandPool commandPool = VK_NULL_HANDLE;
	driver.vkCreateCommandPool(device, &commandPoolCreateInfo, nullptr, &commandPool);

	const VkCommandBufferAllocateInfo commandBufferAllocateInfo = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,  // sType
		nullptr,                                         // pNext
		commandPool,                                     // commandPool
		VK_COMMAND_BUFFER_LEVEL_PRIMARY,                 // level
		1,                                               // commandBufferCount
	};

	VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
	driver.vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &commandBuffer);

	const VkCommandBufferBeginInfo commandBufferBeginInfo = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,  // sType
		nullptr,                                      // pNext
		0,                                            // flags
		nullptr,                                      // pInheritanceInfo
	};

	driver.vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);

	VkClearColorValue clearColorValue = {};
	clearColorValue.float32[0] = 0.0f;
	clearColorValue.float32[1] = 1.0f;
	clearColorValue.float32[2] = 0.0f;
	clearColorValue.float32[3] = 1.0f;

	VkImageSubresourceRange range = {};
	range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	range.baseMipLevel = 0;
	range.levelCount = 1;
	range.baseArrayLayer = 0;
	range.layerCount = 1;

	driver.vkCmdClearColorImage(commandBuffer, image, VK_IMAGE_LAYOUT_GENERAL, &clearColorValue, 1, &range);

	driver.vkEndCommandBuffer(commandBuffer);

	const VkSubmitInfo submitInfo = {
		VK_STRUCTURE_TYPE_SUBMIT_INFO,  // sType
		nullptr,                        // pNext
		0,                              // waitSemaphoreCount
		nullptr,                        // pWaitSemaphores
		nullptr,                        // pWaitDstStageMask
		1,                              // commandBufferCount
		&commandBuffer,                 // pCommandBuffers
		0,                              // signalSemaphoreCount
		nullptr,                        // pSignalSemaphores
	};

	driver.vkQueueSubmit(queue, 1, &submitInfo, 0);

	driver.vkQueueWaitIdle(queue);

	for(auto _ : state)
	{
		driver.vkQueueSubmit(queue, 1, &submitInfo, 0);

		driver.vkQueueWaitIdle(queue);
	}
}
BENCHMARK_REGISTER_F(VulkanBenchmark, Clear)->Unit(benchmark::kMillisecond)->DenseRange(0, 1, 1);
