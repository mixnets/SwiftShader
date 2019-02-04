// Copyright 2018 The SwiftShader Authors. All Rights Reserved.
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

// Vulkan unit tests that provide coverage for functionality not tested by
// the dEQP test suite. Also used as a smoke test.

#include "Driver.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "spirv-tools/libspirv.hpp"

#include <cstring>

class SwiftShaderVulkanTest : public testing::Test
{
};

TEST_F(SwiftShaderVulkanTest, ICD_Check)
{
    Driver driver;
    ASSERT_TRUE(driver.loadSwiftShader());

    auto createInstance = driver.vk_icdGetInstanceProcAddr(VK_NULL_HANDLE, "vkCreateInstance");
    EXPECT_NE(createInstance, nullptr);

    auto enumerateInstanceExtensionProperties =
        driver.vk_icdGetInstanceProcAddr(VK_NULL_HANDLE, "vkEnumerateInstanceExtensionProperties");
    EXPECT_NE(enumerateInstanceExtensionProperties, nullptr);

    auto enumerateInstanceLayerProperties =
        driver.vk_icdGetInstanceProcAddr(VK_NULL_HANDLE, "vkEnumerateInstanceLayerProperties");
    EXPECT_NE(enumerateInstanceLayerProperties, nullptr);

    auto enumerateInstanceVersion = driver.vk_icdGetInstanceProcAddr(VK_NULL_HANDLE, "vkEnumerateInstanceVersion");
    EXPECT_NE(enumerateInstanceVersion, nullptr);

    auto bad_function = driver.vk_icdGetInstanceProcAddr(VK_NULL_HANDLE, "bad_function");
    EXPECT_EQ(bad_function, nullptr);
}

TEST_F(SwiftShaderVulkanTest, Version)
{
    Driver driver;
    ASSERT_TRUE(driver.loadSwiftShader());

    uint32_t apiVersion = 0;
    VkResult result = driver.vkEnumerateInstanceVersion(&apiVersion);
    EXPECT_EQ(apiVersion, (uint32_t)VK_API_VERSION_1_1);

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
    result = driver.vkCreateInstance(&createInfo, nullptr, &instance);
    EXPECT_EQ(result, VK_SUCCESS);

    ASSERT_TRUE(driver.resolve(instance));

    uint32_t pPhysicalDeviceCount = 0;
    result = driver.vkEnumeratePhysicalDevices(instance, &pPhysicalDeviceCount, nullptr);
    EXPECT_EQ(result, VK_SUCCESS);
    EXPECT_EQ(pPhysicalDeviceCount, 1U);

    VkPhysicalDevice pPhysicalDevice = VK_NULL_HANDLE;
    result = driver.vkEnumeratePhysicalDevices(instance, &pPhysicalDeviceCount, &pPhysicalDevice);
    EXPECT_EQ(result, VK_SUCCESS);
    EXPECT_NE(pPhysicalDevice, (VkPhysicalDevice)VK_NULL_HANDLE);

    VkPhysicalDeviceProperties physicalDeviceProperties;
    driver.vkGetPhysicalDeviceProperties(pPhysicalDevice, &physicalDeviceProperties);
    EXPECT_EQ(physicalDeviceProperties.apiVersion, (uint32_t)VK_API_VERSION_1_1);
    EXPECT_EQ(physicalDeviceProperties.deviceID, 0xC0DEU);
    EXPECT_EQ(physicalDeviceProperties.deviceType, VK_PHYSICAL_DEVICE_TYPE_CPU);

    EXPECT_EQ(strncmp(physicalDeviceProperties.deviceName, "SwiftShader Device", VK_MAX_PHYSICAL_DEVICE_NAME_SIZE), 0);
}

template<typename FUNC, typename HANDLE, typename DATA>
VkResult enumerate(const Driver& driver, FUNC func, HANDLE handle, std::vector<DATA>& out)
{
    uint32_t count = 0;
    VkResult result = (driver.*func)(handle, &count, 0);
    if(result != VK_SUCCESS)
    {
        return result;
    }
    out.resize(count);
    return (driver.*func)(handle, &count, out.data());
}

VkResult getPhysicalDevices(const Driver& driver, VkInstance instance, std::vector<VkPhysicalDevice>& out)
{
    return enumerate(driver, &Driver::vkEnumeratePhysicalDevices, instance, out);
}

std::vector<VkQueueFamilyProperties> getPhysicalDeviceQueueFamilyProperties(const Driver& driver,
                                                                            VkPhysicalDevice device)
{
    std::vector<VkQueueFamilyProperties> out;
    uint32_t count = 0;
    driver.vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
    out.resize(count);
    driver.vkGetPhysicalDeviceQueueFamilyProperties(device, &count, out.data());
    return out;
}

bool getComputeQueueFamilyIndex(const Driver& driver, VkPhysicalDevice device, int* index)
{
    auto properties = getPhysicalDeviceQueueFamilyProperties(driver, device);
    for(uint32_t i = 0; i < properties.size(); i++)
    {
        if((properties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) != 0)
        {
            *index = (int)i;
            return true;
        }
    }
    return false;
}

VkResult createComputeDevice(const Driver& driver, VkInstance instance, VkPhysicalDevice* outPhysicalDevice,
                             VkDevice* outDevice, uint32_t* outQueueFamilyIndex)
{
    VkResult result;

    // Gather all physical devices
    std::vector<VkPhysicalDevice> physicalDevices;
    result = getPhysicalDevices(driver, instance, physicalDevices);
    if(result != VK_SUCCESS)
    {
        return result;
    }

    // Inspect each physical device's queue families for compute support.
    for(auto physicalDevice : physicalDevices)
    {
        int queueFamilyIndex = 0;
        if(!getComputeQueueFamilyIndex(driver, physicalDevice, &queueFamilyIndex))
        {
            continue;
        }

        const float queuePrioritory = 1.0f;
        const VkDeviceQueueCreateInfo deviceQueueCreateInfo = {
            VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,  // sType
            nullptr,                                     // pNext
            0,                                           // flags
            (uint32_t)queueFamilyIndex,                  // queueFamilyIndex
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

        VkDevice device;
        result = driver.vkCreateDevice(physicalDevice, &deviceCreateInfo, 0, &device);
        if(result != VK_SUCCESS)
        {
            return result;
        }

        *outPhysicalDevice = physicalDevice;
        *outDevice = device;
        *outQueueFamilyIndex = (uint32_t)queueFamilyIndex;
        return VK_SUCCESS;
    }

    return VK_SUCCESS;
}

VkResult allocateMemory(const Driver& driver, VkPhysicalDevice physicalDevice, VkDevice device, size_t size, int flags,
                        VkDeviceMemory* out)
{
    VkPhysicalDeviceMemoryProperties properties;
    driver.vkGetPhysicalDeviceMemoryProperties(physicalDevice, &properties);

    for(uint32_t type = 0; type < properties.memoryTypeCount; type++)
    {
        if((flags & properties.memoryTypes[type].propertyFlags) == 0)
        {
            continue;  // Type mismatch
        }

        if(size > properties.memoryHeaps[properties.memoryTypes[type].heapIndex].size)
        {
            continue;  // Too small.
        }

        const VkMemoryAllocateInfo info = {
            VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,  // sType
            nullptr,                                 // pNext
            size,                                    // allocationSize
            type,                                    // memoryTypeIndex
        };

        return driver.vkAllocateMemory(device, &info, 0, out);
    }

    return VK_ERROR_OUT_OF_DEVICE_MEMORY;
}

VkResult createBuffer(const Driver& driver, VkDevice device, VkDeviceMemory memory, VkDeviceSize size,
                      VkDeviceSize offset, VkBuffer* out)
{
    const VkBufferCreateInfo info = {
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,  // sType
        nullptr,                               // pNext
        0,                                     // flags
        size,                                  // size
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,    // usage
        VK_SHARING_MODE_EXCLUSIVE,             // sharingMode
        0,                                     // queueFamilyIndexCount
        nullptr,                               // pQueueFamilyIndices
    };

    VkBuffer buffer;
    VkResult result = driver.vkCreateBuffer(device, &info, 0, &buffer);
    if(result != VK_SUCCESS)
    {
        return result;
    }

    result = driver.vkBindBufferMemory(device, buffer, memory, offset);
    if(result != VK_SUCCESS)
    {
        return result;
    }

    *out = buffer;
    return VK_SUCCESS;
}

VkResult createShaderModule(const Driver& driver, VkDevice device, const std::vector<uint32_t>& code,
                            VkShaderModule* out)
{
    VkShaderModuleCreateInfo info = {
        VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,  // sType
        nullptr,                                      // pNext
        0,                                            // flags
        code.size() * 4,                              // codeSize
        code.data(),                                  // pCode
    };
    return driver.vkCreateShaderModule(device, &info, 0, out);
}

VkResult createDescriptorSetLayout(const Driver& driver, VkDevice device,
                                   const std::vector<VkDescriptorSetLayoutBinding>& bindings,
                                   VkDescriptorSetLayout* out)
{
    VkDescriptorSetLayoutCreateInfo info = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,  // sType
        nullptr,                                              // pNext
        0,                                                    // flags
        (uint32_t)bindings.size(),                            // bindingCount
        bindings.data(),                                      // pBindings
    };

    return driver.vkCreateDescriptorSetLayout(device, &info, 0, out);
}

VkResult createPipelineLayout(const Driver& driver, VkDevice device, VkDescriptorSetLayout layout,
                              VkPipelineLayout* out)
{
    VkPipelineLayoutCreateInfo info = {
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,  // sType
        nullptr,                                        // pNext
        0,                                              // flags
        1,                                              // setLayoutCount
        &layout,                                        // pSetLayouts
        0,                                              // pushConstantRangeCount
        nullptr,                                        // pPushConstantRanges
    };

    return driver.vkCreatePipelineLayout(device, &info, 0, out);
}

VkResult createComputePipeline(const Driver& driver, VkDevice device, VkShaderModule module,
                               VkPipelineLayout pipelineLayout, VkPipeline* out)
{
    VkComputePipelineCreateInfo info = {
        VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,  // sType
        nullptr,                                         // pNext
        0,                                               // flags
        {
            // stage
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,  // sType
            nullptr,                                              // pNext
            0,                                                    // flags
            VK_SHADER_STAGE_COMPUTE_BIT,                          // stage
            module,                                               // module
            "main",                                               // pName
            nullptr,                                              // pSpecializationInfo
        },
        pipelineLayout,  // layout
        0,               // basePipelineHandle
        0,               // basePipelineIndex
    };

    auto res = driver.vkCreateComputePipelines(device, 0, 1, &info, 0, out);
    EXPECT_NE(out, nullptr);
    return res;
}

VkResult createDescriptorPool(const Driver& driver, VkDevice device, VkDescriptorPool* out)
{
    VkDescriptorPoolSize size = {
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  // type
        2,                                  // descriptorCount
    };

    VkDescriptorPoolCreateInfo info = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,  // sType
        nullptr,                                        // pNext
        0,                                              // flags
        1,                                              // maxSets
        1,                                              // poolSizeCount
        &size,                                          // pPoolSizes
    };

    return driver.vkCreateDescriptorPool(device, &info, 0, out);
}

VkResult allocateDescriptorSet(const Driver& driver, VkDevice device, VkDescriptorPool pool,
                               VkDescriptorSetLayout layout, VkDescriptorSet* out)
{
    VkDescriptorSetAllocateInfo info = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,  // sType
        nullptr,                                         // pNext
        pool,                                            // descriptorPool
        1,                                               // descriptorSetCount
        &layout,                                         // pSetLayouts
    };
    return driver.vkAllocateDescriptorSets(device, &info, out);
}

void updateDescriptorSets(const Driver& driver, VkDevice device, VkDescriptorSet descriptorSet,
                          const std::vector<VkDescriptorBufferInfo>& bufferInfos)
{
    std::vector<VkWriteDescriptorSet> writes;
    writes.reserve(bufferInfos.size());
    for(uint32_t i = 0; i < bufferInfos.size(); i++)
    {
        writes.push_back(VkWriteDescriptorSet{
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,  // sType
            nullptr,                                 // pNext
            descriptorSet,                           // dstSet
            i,                                       // dstBinding
            0,                                       // dstArrayElement
            1,                                       // descriptorCount
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,       // descriptorType
            nullptr,                                 // pImageInfo
            &bufferInfos[i],                         // pBufferInfo
            nullptr,                                 // pTexelBufferView
        });
    }
    driver.vkUpdateDescriptorSets(device, writes.size(), writes.data(), 0, nullptr);
}

VkResult createCommandPool(const Driver& driver, VkDevice device, uint32_t queueFamilyIndex, VkCommandPool* out)
{
    VkCommandPoolCreateInfo info = {
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,  // sType
        nullptr,                                     // pNext
        0,                                           // flags
        queueFamilyIndex,                            // queueFamilyIndex
    };
    return driver.vkCreateCommandPool(device, &info, 0, out);
}

VkResult allocateCommandBuffer(const Driver& driver, VkDevice device, VkCommandPool pool, VkCommandBuffer* out)
{
    VkCommandBufferAllocateInfo info = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,  // sType
        nullptr,                                         // pNext
        pool,                                            // commandPool
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,                 // level
        1,                                               // commandBufferCount
    };
    return driver.vkAllocateCommandBuffers(device, &info, out);
}

VkResult beginCommandBuffer(const Driver& driver, VkCommandBufferUsageFlagBits usage, VkCommandBuffer commandBuffer)
{
    VkCommandBufferBeginInfo info = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,  // sType
        nullptr,                                      // pNext
        usage,                                        // flags
        nullptr,                                      // pInheritanceInfo
    };

    return driver.vkBeginCommandBuffer(commandBuffer, &info);
}

VkResult queueSubmitAndWait(const Driver& driver, VkDevice device, uint32_t queueFamilyIndex,
                            VkCommandBuffer commandBuffer)
{
    VkQueue queue;
    driver.vkGetDeviceQueue(device, queueFamilyIndex, 0, &queue);

    VkSubmitInfo info = {
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

    VkResult result = driver.vkQueueSubmit(queue, 1, &info, 0);
    if(result != VK_SUCCESS)
    {
        return result;
    }

    return driver.vkQueueWaitIdle(queue);
}

std::vector<uint32_t> compile(const char* source)
{
    spvtools::SpirvTools core(SPV_ENV_VULKAN_1_0);

    core.SetMessageConsumer([](spv_message_level_t, const char*, const spv_position_t& p, const char* m) {
        FAIL() << p.line << ":" << p.column << ": " << m;
    });

    std::vector<uint32_t> spirv;
    EXPECT_TRUE(core.Assemble(source, &spirv));
    EXPECT_TRUE(core.Validate(spirv));
    return spirv;
}

#define VK_ASSERT(x) ASSERT_EQ(x, VK_SUCCESS)

TEST_F(SwiftShaderVulkanTest, Compute)
{
    Driver driver;
    ASSERT_TRUE(driver.loadSwiftShader());

    auto code = compile(
        "                   OpCapability Shader \n"
        "                   OpMemoryModel Logical GLSL450 \n"
        "                   OpEntryPoint GLCompute %main \"main\" %gl_GlobalInvocationID \n"
        "                   OpExecutionMode %main LocalSize 8 1 1 \n"
        "                   OpDecorate %_runtimearr_int ArrayStride 4 \n"
        "                   OpMemberDecorate %b 0 Offset 0 \n"
        "                   OpDecorate %b BufferBlock \n"
        "                   OpDecorate %outArray DescriptorSet 0 \n"
        "                   OpDecorate %outArray Binding 1 \n"
        "                   OpDecorate %gl_GlobalInvocationID BuiltIn GlobalInvocationId \n"
        "                   OpDecorate %_runtimearr_int_0 ArrayStride 4 \n"
        "                   OpMemberDecorate %a 0 Offset 0 \n"
        "                   OpDecorate %a BufferBlock \n"
        "                   OpDecorate %inArray DescriptorSet 0 \n"
        "                   OpDecorate %inArray Binding 0 \n"
        "           %void = OpTypeVoid \n"
        "              %3 = OpTypeFunction %void \n"
        "            %int = OpTypeInt 32 1 \n"
        "    %_runtimearr_int = OpTypeRuntimeArray %int \n"
        "              %b = OpTypeStruct %_runtimearr_int \n"
        "    %_ptr_Uniform_b = OpTypePointer Uniform %b \n"
        "       %outArray = OpVariable %_ptr_Uniform_b Uniform \n"
        "          %int_0 = OpConstant %int 0 \n"
        "           %uint = OpTypeInt 32 0 \n"
        "         %v3uint = OpTypeVector %uint 3 \n"
        "    %_ptr_Input_v3uint = OpTypePointer Input %v3uint \n"
        "    %gl_GlobalInvocationID = OpVariable %_ptr_Input_v3uint Input \n"
        "         %uint_0 = OpConstant %uint 0 \n"
        "    %_ptr_Input_uint = OpTypePointer Input %uint \n"
        "    %_runtimearr_int_0 = OpTypeRuntimeArray %int \n"
        "              %a = OpTypeStruct %_runtimearr_int_0 \n"
        "    %_ptr_Uniform_a = OpTypePointer Uniform %a \n"
        "        %inArray = OpVariable %_ptr_Uniform_a Uniform \n"
        "    %_ptr_Uniform_int = OpTypePointer Uniform %int \n"
        "          %int_1 = OpConstant %int 1 \n"
        "           %main = OpFunction %void None %3 \n"
        "              %5 = OpLabel \n"
        "             %23 = OpAccessChain %_ptr_Input_uint %gl_GlobalInvocationID %uint_0 \n"
        "             %24 = OpLoad %uint %23 \n"
        "             %25 = OpAccessChain %_ptr_Uniform_int %inArray %int_0 %24 \n"
        "             %26 = OpLoad %int %25 \n"
        "             %27 = OpAccessChain %_ptr_Uniform_int %outArray %int_0 %24 \n"
        "                   OpStore %27 %26 \n"
        "                   OpReturn \n"
        "                   OpFunctionEnd \n");

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
    VK_ASSERT(driver.vkCreateInstance(&createInfo, nullptr, &instance));

    ASSERT_TRUE(driver.resolve(instance));

    VkDevice device;
    VkPhysicalDevice physicalDevice;
    uint32_t queueFamilyIndex = 0;
    VK_ASSERT(createComputeDevice(driver, instance, &physicalDevice, &device, &queueFamilyIndex));

    constexpr int NUM_ELEMENTS = 256;

    struct Buffers
    {
        uint32_t in[NUM_ELEMENTS];
        uint32_t out[NUM_ELEMENTS];
    };

    VkDeviceMemory memory;
    VK_ASSERT(allocateMemory(driver, physicalDevice, device, sizeof(Buffers),
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &memory));

    Buffers* buffers;
    VK_ASSERT(driver.vkMapMemory(device, memory, 0, sizeof(Buffers), 0, (void**)&buffers));

    memset(buffers, 0, sizeof(Buffers));

    for(int i = 0; i < NUM_ELEMENTS; i++)
    {
        buffers->in[i] = (uint32_t)i;
    }

    driver.vkUnmapMemory(device, memory);
    buffers = nullptr;

    VkBuffer bufferIn;
    VK_ASSERT(createBuffer(driver, device, memory, sizeof(Buffers::in), offsetof(Buffers, in), &bufferIn));

    VkBuffer bufferOut;
    VK_ASSERT(createBuffer(driver, device, memory, sizeof(Buffers::out), offsetof(Buffers, out), &bufferOut));

    VkShaderModule shaderModule;
    VK_ASSERT(createShaderModule(driver, device, code, &shaderModule));

    std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings;
    descriptorSetLayoutBindings.push_back(VkDescriptorSetLayoutBinding{
        0,                                  // binding
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  // descriptorType
        1,                                  // descriptorCount
        VK_SHADER_STAGE_COMPUTE_BIT,        // stageFlags
        0,                                  // pImmutableSamplers
    });
    descriptorSetLayoutBindings.push_back(VkDescriptorSetLayoutBinding{
        1,                                  // binding
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  // descriptorType
        1,                                  // descriptorCount
        VK_SHADER_STAGE_COMPUTE_BIT,        // stageFlags
        0,                                  // pImmutableSamplers
    });
    VkDescriptorSetLayout descriptorSetLayout;
    VK_ASSERT(createDescriptorSetLayout(driver, device, descriptorSetLayoutBindings, &descriptorSetLayout));

    VkPipelineLayout pipelineLayout;
    VK_ASSERT(createPipelineLayout(driver, device, descriptorSetLayout, &pipelineLayout));

    VkPipeline pipeline;
    VK_ASSERT(createComputePipeline(driver, device, shaderModule, pipelineLayout, &pipeline));

    VkDescriptorPool descriptorPool;
    VK_ASSERT(createDescriptorPool(driver, device, &descriptorPool));

    VkDescriptorSet descriptorSet;
    VK_ASSERT(allocateDescriptorSet(driver, device, descriptorPool, descriptorSetLayout, &descriptorSet));

    std::vector<VkDescriptorBufferInfo> descriptorBufferInfos;
    descriptorBufferInfos.push_back(VkDescriptorBufferInfo{
        bufferIn,       // buffer
        0,              // offset
        VK_WHOLE_SIZE,  // range
    });
    descriptorBufferInfos.push_back(VkDescriptorBufferInfo{
        bufferOut,      // buffer
        0,              // offset
        VK_WHOLE_SIZE,  // range
    });
    updateDescriptorSets(driver, device, descriptorSet, descriptorBufferInfos);

    VkCommandPool commandPool;
    VK_ASSERT(createCommandPool(driver, device, queueFamilyIndex, &commandPool));

    VkCommandBuffer commandBuffer;
    VK_ASSERT(allocateCommandBuffer(driver, device, commandPool, &commandBuffer));

    VK_ASSERT(beginCommandBuffer(driver, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, commandBuffer));

    driver.vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);

    driver.vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1, &descriptorSet,
                                   0, nullptr);

    driver.vkCmdDispatch(commandBuffer, NUM_ELEMENTS / 8, 1, 1);

    VK_ASSERT(driver.vkEndCommandBuffer(commandBuffer));

    VK_ASSERT(queueSubmitAndWait(driver, device, queueFamilyIndex, commandBuffer));

    VK_ASSERT(driver.vkMapMemory(device, memory, 0, sizeof(Buffers), 0, (void**)&buffers));

    for(int i = 0; i < NUM_ELEMENTS; ++i)
    {
        EXPECT_EQ(buffers->in[i], buffers->out[i]) << "Unexpected output at " << i;
    }

    driver.vkUnmapMemory(device, memory);
    buffers = nullptr;
}