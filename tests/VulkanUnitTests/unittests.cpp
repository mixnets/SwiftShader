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
#include "Device.hpp"

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

    Device device;
    VK_ASSERT(Device::CreateComputeDevice(&driver, instance, &device));
    ASSERT_TRUE(device.IsValid());

    constexpr int NUM_ELEMENTS = 256;

    struct Buffers
    {
        uint32_t in[NUM_ELEMENTS];
        uint32_t out[NUM_ELEMENTS];
    };

    VkDeviceMemory memory;
    VK_ASSERT(device.AllocateMemory(sizeof(Buffers),
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &memory));

    Buffers* buffers;
    VK_ASSERT(device.MapMemory(memory, 0, sizeof(Buffers), 0, (void**)&buffers));

    memset(buffers, 0, sizeof(Buffers));

    for(int i = 0; i < NUM_ELEMENTS; i++)
    {
        buffers->in[i] = (uint32_t)i;
    }

    device.UnmapMemory(memory);
    buffers = nullptr;

    VkBuffer bufferIn;
    VK_ASSERT(device.CreateBuffer(memory, sizeof(Buffers::in), offsetof(Buffers, in), &bufferIn));

    VkBuffer bufferOut;
    VK_ASSERT(device.CreateBuffer(memory, sizeof(Buffers::out), offsetof(Buffers, out), &bufferOut));

    VkShaderModule shaderModule;
    VK_ASSERT(device.CreateShaderModule(code, &shaderModule));

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
    VK_ASSERT(device.CreateDescriptorSetLayout(descriptorSetLayoutBindings, &descriptorSetLayout));

    VkPipelineLayout pipelineLayout;
    VK_ASSERT(device.CreatePipelineLayout(descriptorSetLayout, &pipelineLayout));

    VkPipeline pipeline;
    VK_ASSERT(device.CreateComputePipeline(shaderModule, pipelineLayout, &pipeline));

    VkDescriptorPool descriptorPool;
    VK_ASSERT(device.CreateDescriptorPool(&descriptorPool));

    VkDescriptorSet descriptorSet;
    VK_ASSERT(device.AllocateDescriptorSet(descriptorPool, descriptorSetLayout, &descriptorSet));

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
    device.UpdateDescriptorSets(descriptorSet, descriptorBufferInfos);

    VkCommandPool commandPool;
    VK_ASSERT(device.CreateCommandPool(&commandPool));

    VkCommandBuffer commandBuffer;
    VK_ASSERT(device.AllocateCommandBuffer(commandPool, &commandBuffer));

    VK_ASSERT(device.BeginCommandBuffer(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, commandBuffer));

    driver.vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);

    driver.vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1, &descriptorSet,
                                   0, nullptr);

    driver.vkCmdDispatch(commandBuffer, NUM_ELEMENTS / 8, 1, 1);

    VK_ASSERT(driver.vkEndCommandBuffer(commandBuffer));

    VK_ASSERT(device.QueueSubmitAndWait(commandBuffer));

    VK_ASSERT(device.MapMemory(memory, 0, sizeof(Buffers), 0, (void**)&buffers));

    for (int i = 0; i < NUM_ELEMENTS; ++i)
    {
        EXPECT_EQ(buffers->in[i], buffers->out[i]) << "Unexpected output at " << i;
    }

    device.UnmapMemory(memory);
    buffers = nullptr;
}