// Copyright 2019 The SwiftShader Authors. All Rights Reserved.
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

#include <vulkan/vulkan_core.h>

#include <vector>

class Driver;

// Device provides a wrapper around a VkDevice with a number of helper functions
// for common test operations.
class Device
{
public:
	Device();

	// CreateComputeDevice enumerates the physical devices, looking for a device
	// that supports compute. If one is found, then a device is created and
	// assigned to outDevice.
	// If one is not found, VK_SUCCESS will still be returned (as there was no
	// Vulkan error), but calling Device::IsValid() will return false.
	static VkResult CreateComputeDevice(
			Driver const *driver, VkInstance instance, Device *outDevice);

	// IsValid returns true if the Device is initialized and can be used.
	bool IsValid() const;

	VkResult CreateBuffer(VkDeviceMemory memory, VkDeviceSize size,
			VkDeviceSize offset, VkBuffer *out) const;

	VkResult CreateShaderModule(const std::vector<uint32_t> &code,
			VkShaderModule *out) const;

	VkResult CreateDescriptorSetLayout(
			const std::vector<VkDescriptorSetLayoutBinding> &bindings,
			VkDescriptorSetLayout *out) const;

	VkResult CreatePipelineLayout(VkDescriptorSetLayout layout,
			VkPipelineLayout *out) const;

	VkResult CreateComputePipeline(VkShaderModule module,
			VkPipelineLayout pipelineLayout,
			VkPipeline *out) const;

	VkResult CreateDescriptorPool(VkDescriptorPool *out) const;

	VkResult AllocateDescriptorSet(VkDescriptorPool pool,
			VkDescriptorSetLayout layout,
			VkDescriptorSet *out) const;

	void UpdateDescriptorSets(VkDescriptorSet descriptorSet,
		const std::vector<VkDescriptorBufferInfo> &bufferInfos) const;

	VkResult AllocateMemory(size_t size, int flags, VkDeviceMemory* out) const;

	VkResult MapMemory(VkDeviceMemory memory, VkDeviceSize offset,
			VkDeviceSize size, VkMemoryMapFlags flags, void **ppData) const;

	void UnmapMemory(VkDeviceMemory memory) const;

	VkResult CreateCommandPool(VkCommandPool* out) const;

	VkResult AllocateCommandBuffer(VkCommandPool pool, VkCommandBuffer* out) const;

	VkResult BeginCommandBuffer(VkCommandBufferUsageFlagBits usage, VkCommandBuffer commandBuffer) const;

	VkResult QueueSubmitAndWait(VkCommandBuffer commandBuffer) const;

private:
	Device(Driver const *driver, VkDevice device, VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex);

	static VkResult GetPhysicalDevices(
			Driver const *driver, VkInstance instance,
			std::vector<VkPhysicalDevice> &out);

	static int GetComputeQueueFamilyIndex(
			Driver const *driver, VkPhysicalDevice device);

	static std::vector<VkQueueFamilyProperties>
		GetPhysicalDeviceQueueFamilyProperties(
			Driver const *driver, VkPhysicalDevice device);

	Driver const *driver;
	VkDevice device;
	VkPhysicalDevice physicalDevice;
	uint32_t queueFamilyIndex;
};
