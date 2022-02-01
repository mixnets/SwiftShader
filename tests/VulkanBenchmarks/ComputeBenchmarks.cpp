// Copyright 2021 The SwiftShader Authors. All Rights Reserved.
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

#include "Util.hpp"
#include "VulkanTester.hpp"

#include "benchmark/benchmark.h"
#include "spirv-tools/libspirv.hpp"

#include <cstring>
#include <sstream>

static size_t alignUp(size_t val, size_t alignment)
{
	return alignment * ((val + alignment - 1) / alignment);
}

class ComputeBenchmark
{
protected:
	ComputeBenchmark()
	{
		tester.initialize();
	}

	VulkanTester tester;
};

std::vector<uint32_t> compileSpirv(const char *assembly)
{
	spvtools::SpirvTools core(SPV_ENV_VULKAN_1_0);

	core.SetMessageConsumer([](spv_message_level_t, const char *, const spv_position_t &p, const char *m) {
		assert(false);
	});

	std::vector<uint32_t> spirv;
	core.Assemble(assembly, &spirv);
	core.Validate(spirv);

	// Warn if the disassembly does not match the source assembly.
	// We do this as debugging tests in the debugger is often made much harder
	// if the SSA names (%X) in the debugger do not match the source.
	std::string disassembled;
	core.Disassemble(spirv, &disassembled, SPV_BINARY_TO_TEXT_OPTION_NO_HEADER);
	if(disassembled != assembly)
	{
		printf("-- WARNING: Disassembly does not match assembly: ---\n\n");

		auto splitLines = [](const std::string &str) -> std::vector<std::string> {
			std::stringstream ss(str);
			std::vector<std::string> out;
			std::string line;
			while(std::getline(ss, line, '\n')) { out.push_back(line); }
			return out;
		};

		auto srcLines = splitLines(std::string(assembly));
		auto disLines = splitLines(disassembled);

		for(size_t line = 0; line < srcLines.size() && line < disLines.size(); line++)
		{
			auto srcLine = (line < srcLines.size()) ? srcLines[line] : "<missing>";
			auto disLine = (line < disLines.size()) ? disLines[line] : "<missing>";
			if(srcLine != disLine)
			{
				printf("%zu: '%s' != '%s'\n", line, srcLine.c_str(), disLine.c_str());
			}
		}
		printf("\n\n---\nExpected:\n\n%s", disassembled.c_str());
	}

	return spirv;
}

// Base class for compute benchmarks that read from an input buffer and write to an
// output buffer of same length.
class BufferToBufferComputeBenchmark : public ComputeBenchmark
{
public:
	BufferToBufferComputeBenchmark(const benchmark::State &state)
	    : state(state)
	{
		device = tester.getDevice();
	}

	virtual ~BufferToBufferComputeBenchmark()
	{
		device.destroyCommandPool(commandPool);
		device.destroyDescriptorPool(descriptorPool);
		device.destroyPipeline(pipeline);
		device.destroyDescriptorSetLayout(descriptorSetLayout);
		device.destroyBuffer(bufferIn);
		device.destroyBuffer(bufferOut);
		device.freeMemory(deviceMemory);
	}

	void run();

protected:
	void initialize(const std::string &shader);

	uint32_t localSizeX = 128;
	uint32_t localSizeY = 1;
	uint32_t localSizeZ = 1;

private:
	const benchmark::State &state;

	// Weak references
	vk::Device device;
	vk::Queue queue;
	vk::CommandBuffer commandBuffer;

	// Owned resources
	vk::CommandPool commandPool;
	vk::DescriptorPool descriptorPool;
	vk::Pipeline pipeline;
	vk::DescriptorSetLayout descriptorSetLayout;
	vk::DeviceMemory deviceMemory;
	vk::Buffer bufferIn;
	vk::Buffer bufferOut;
};

void BufferToBufferComputeBenchmark::initialize(const std::string &shader)
{
	auto code = compileSpirv(shader.c_str());

	auto &device = tester.getDevice();
	auto &physicalDevice = tester.getPhysicalDevice();
	queue = device.getQueue(0, 0);  /////////////

	// struct Buffers
	// {
	//     uint32_t pad0[63];
	//     uint32_t magic0;
	//     uint32_t in[NUM_ELEMENTS]; // Aligned to 0x100
	//     uint32_t magic1;
	//     uint32_t pad1[N];
	//     uint32_t magic2;
	//     uint32_t out[NUM_ELEMENTS]; // Aligned to 0x100
	//     uint32_t magic3;
	// };
	static constexpr uint32_t magic0 = 0x01234567;
	static constexpr uint32_t magic1 = 0x89abcdef;
	static constexpr uint32_t magic2 = 0xfedcba99;
	static constexpr uint32_t magic3 = 0x87654321;
	size_t numElements = state.range(0);
	size_t alignElements = 0x100 / sizeof(uint32_t);
	size_t magic0Offset = alignElements - 1;
	size_t inOffset = 1 + magic0Offset;
	size_t magic1Offset = numElements + inOffset;
	size_t magic2Offset = alignUp(magic1Offset + 1, alignElements) - 1;
	size_t outOffset = 1 + magic2Offset;
	size_t magic3Offset = numElements + outOffset;
	size_t buffersTotalElements = alignUp(1 + magic3Offset, alignElements);
	size_t buffersSize = sizeof(uint32_t) * buffersTotalElements;

	//////////////////vk::MemoryRequirements memoryRequirements = device.getBufferMemoryRequirements(buffer);
	vk::MemoryAllocateInfo allocateInfo;
	allocateInfo.allocationSize = buffersSize;
	allocateInfo.memoryTypeIndex = 0;  ///////////////////Util::getMemoryTypeIndex(physicalDevice, memoryRequirements.memoryTypeBits);
	deviceMemory = device.allocateMemory(allocateInfo);

	uint32_t *buffers = (uint32_t *)device.mapMemory(deviceMemory, 0, buffersSize);

	memset(buffers, 0, buffersSize);

	buffers[magic0Offset] = magic0;
	buffers[magic1Offset] = magic1;
	buffers[magic2Offset] = magic2;
	buffers[magic3Offset] = magic3;

	for(size_t i = 0; i < numElements; i++)
	{
		buffers[inOffset + i] = (uint32_t)i;  //input((uint32_t)i);
	}

	device.unmapMemory(deviceMemory);
	buffers = nullptr;

	vk::BufferCreateInfo bufferCreateInfo({}, sizeof(uint32_t) * numElements, vk::BufferUsageFlagBits::eStorageBuffer);
	bufferIn = device.createBuffer(bufferCreateInfo);
	device.bindBufferMemory(bufferIn, deviceMemory, sizeof(uint32_t) * inOffset);

	bufferOut = device.createBuffer(bufferCreateInfo);
	device.bindBufferMemory(bufferOut, deviceMemory, sizeof(uint32_t) * outOffset);

	vk::ShaderModuleCreateInfo moduleCreateInfo;
	moduleCreateInfo.codeSize = code.size() * sizeof(uint32_t);
	moduleCreateInfo.pCode = (uint32_t *)code.data();
	vk::ShaderModule shaderModule = device.createShaderModule(moduleCreateInfo);

	vk::DescriptorSetLayoutBinding in;
	in.binding = 0;
	in.descriptorCount = 1;
	in.descriptorType = vk::DescriptorType::eStorageBuffer;
	in.stageFlags = vk::ShaderStageFlagBits::eCompute;

	vk::DescriptorSetLayoutBinding out;
	out.binding = 1;
	out.descriptorCount = 1;
	out.descriptorType = vk::DescriptorType::eStorageBuffer;
	out.stageFlags = vk::ShaderStageFlagBits::eCompute;

	std::vector<vk::DescriptorSetLayoutBinding> setLayoutBindings = { in, out };
	vk::DescriptorSetLayoutCreateInfo layoutInfo;
	layoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
	layoutInfo.pBindings = setLayoutBindings.data();
	descriptorSetLayout = device.createDescriptorSetLayout(layoutInfo);

	vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
	pipelineLayoutCreateInfo.setLayoutCount = 1;
	pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;
	vk::PipelineLayout pipelineLayout = device.createPipelineLayout(pipelineLayoutCreateInfo);

	vk::ComputePipelineCreateInfo computePipelineCreateInfo;
	computePipelineCreateInfo.layout = pipelineLayout;
	computePipelineCreateInfo.stage.stage = vk::ShaderStageFlagBits::eCompute;
	computePipelineCreateInfo.stage.module = shaderModule;
	computePipelineCreateInfo.stage.pName = "main";
	pipeline = device.createComputePipeline({}, computePipelineCreateInfo).value;

	device.destroyShaderModule(shaderModule);

	std::array<vk::DescriptorPoolSize, 1> poolSizes = {};
	poolSizes[0].type = vk::DescriptorType::eStorageBuffer;
	poolSizes[0].descriptorCount = 2;
	vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo;
	descriptorPoolCreateInfo.maxSets = 1;
	descriptorPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	descriptorPoolCreateInfo.pPoolSizes = poolSizes.data();

	descriptorPool = device.createDescriptorPool(descriptorPoolCreateInfo);

	vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo;
	descriptorSetAllocateInfo.descriptorPool = descriptorPool;
	descriptorSetAllocateInfo.descriptorSetCount = 1;
	descriptorSetAllocateInfo.pSetLayouts = &descriptorSetLayout;
	auto descriptorSets = device.allocateDescriptorSets(descriptorSetAllocateInfo);

	vk::DescriptorBufferInfo inBufferInfo;
	inBufferInfo.buffer = bufferIn;
	inBufferInfo.offset = 0;
	inBufferInfo.range = VK_WHOLE_SIZE;

	vk::DescriptorBufferInfo outBufferInfo;
	outBufferInfo.buffer = bufferOut;
	outBufferInfo.offset = 0;
	outBufferInfo.range = VK_WHOLE_SIZE;

	std::array<vk::WriteDescriptorSet, 2> descriptorWrites = {};

	descriptorWrites[0].dstSet = descriptorSets[0];
	descriptorWrites[0].dstBinding = 0;
	descriptorWrites[0].dstArrayElement = 0;
	descriptorWrites[0].descriptorType = vk::DescriptorType::eStorageBuffer;
	descriptorWrites[0].descriptorCount = 1;
	descriptorWrites[0].pBufferInfo = &inBufferInfo;

	descriptorWrites[1].dstSet = descriptorSets[0];
	descriptorWrites[1].dstBinding = 1;
	descriptorWrites[1].dstArrayElement = 0;
	descriptorWrites[1].descriptorType = vk::DescriptorType::eStorageBuffer;
	descriptorWrites[1].descriptorCount = 1;
	descriptorWrites[1].pBufferInfo = &outBufferInfo;

	device.updateDescriptorSets(static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

	vk::CommandPoolCreateInfo commandPoolCreateInfo;
	commandPoolCreateInfo.queueFamilyIndex = 0;  ////////////////
	commandPoolCreateInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
	commandPool = device.createCommandPool(commandPoolCreateInfo);

	vk::CommandBufferAllocateInfo commandBufferAllocateInfo;
	commandBufferAllocateInfo.commandPool = commandPool;
	commandBufferAllocateInfo.commandBufferCount = 1;
	commandBufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
	auto commandBuffers = device.allocateCommandBuffers(commandBufferAllocateInfo);

	// Record the command buffer
	commandBuffer = commandBuffers[0];

	vk::CommandBufferBeginInfo commandBufferBeginInfo;
	commandBuffer.begin(commandBufferBeginInfo);

	commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline);

	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pipelineLayout, 0, 1, &descriptorSets[0], 0, nullptr);

	commandBuffer.dispatch((uint32_t)(numElements / localSizeX), 1, 1);

	commandBuffer.end();

	// Destroy objects we don't have to hold on to after command buffer recording.
	// "A VkPipelineLayout object must not be destroyed while any command buffer that uses it is in the recording state."
	device.destroyPipelineLayout(pipelineLayout);
}

void BufferToBufferComputeBenchmark::run()
{
	vk::SubmitInfo submitInfo;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	queue.submit(submitInfo);
	queue.waitIdle();
}

class Memcpy : public BufferToBufferComputeBenchmark
{
public:
	Memcpy(const benchmark::State &state)
	    : BufferToBufferComputeBenchmark(state)
	{
		std::stringstream src;
		// #version 450
		// layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
		// layout(binding = 0, std430) buffer InBuffer
		// {
		//     int Data[];
		// } In;
		// layout(binding = 1, std430) buffer OutBuffer
		// {
		//     int Data[];
		// } Out;
		// void main()
		// {
		//     Out.Data[gl_GlobalInvocationID.x] = In.Data[gl_GlobalInvocationID.x];
		// }
		// clang-format off
		src <<
			"OpCapability Shader\n"
			"OpMemoryModel Logical GLSL450\n"
			"OpEntryPoint GLCompute %1 \"main\" %2\n"
			"OpExecutionMode %1 LocalSize " <<
			localSizeX << " " <<
			localSizeY << " " <<
			localSizeZ << "\n" <<
			"OpDecorate %3 ArrayStride 4\n"
			"OpMemberDecorate %4 0 Offset 0\n"
			"OpDecorate %4 BufferBlock\n"
			"OpDecorate %5 DescriptorSet 0\n"
			"OpDecorate %5 Binding 1\n"
			"OpDecorate %2 BuiltIn GlobalInvocationId\n"
			"OpDecorate %6 DescriptorSet 0\n"
			"OpDecorate %6 Binding 0\n"
			"%7 = OpTypeVoid\n"
			"%8 = OpTypeFunction %7\n"             // void()
			"%9 = OpTypeInt 32 1\n"                // int32
			"%10 = OpTypeInt 32 0\n"                // uint32
			"%3 = OpTypeRuntimeArray %9\n"         // int32[]
			"%4 = OpTypeStruct %3\n"               // struct{ int32[] }
			"%11 = OpTypePointer Uniform %4\n"      // struct{ int32[] }*
			"%5 = OpVariable %11 Uniform\n"        // struct{ int32[] }* in
			"%12 = OpConstant %9 0\n"               // int32(0)
			"%13 = OpConstant %10 0\n"              // uint32(0)
			"%14 = OpTypeVector %10 3\n"            // vec3<int32>
			"%15 = OpTypePointer Input %14\n"       // vec3<int32>*
			"%2 = OpVariable %15 Input\n"          // gl_GlobalInvocationId
			"%16 = OpTypePointer Input %10\n"       // uint32*
			"%6 = OpVariable %11 Uniform\n"        // struct{ int32[] }* out
			"%17 = OpTypePointer Uniform %9\n"      // int32*
			"%1 = OpFunction %7 None %8\n"         // -- Function begin --
			"%18 = OpLabel\n"
			"%19 = OpAccessChain %16 %2 %13\n"      // &gl_GlobalInvocationId.x
			"%20 = OpLoad %10 %19\n"                // gl_GlobalInvocationId.x
			"%21 = OpAccessChain %17 %6 %12 %20\n"  // &in.arr[gl_GlobalInvocationId.x]
			"%22 = OpLoad %9 %21\n"                 // in.arr[gl_GlobalInvocationId.x]
			"%23 = OpAccessChain %17 %5 %12 %20\n"  // &out.arr[gl_GlobalInvocationId.x]
			"OpStore %23 %22\n"               // out.arr[gl_GlobalInvocationId.x] = in[gl_GlobalInvocationId.x]
			"OpReturn\n"
			"OpFunctionEnd\n";
		// clang-format on

		initialize(src.str());
	}
};

static void ComputeMemcpy(benchmark::State &state)
{
	Memcpy benchmark(state);

	// Execute once to have the Reactor routine generated.
	benchmark.run();

	for(auto _ : state)
	{
		benchmark.run();
	}
}

BENCHMARK(ComputeMemcpy)->Range(128, 4 * 1024 * 1024)->Unit(benchmark::kMillisecond)->MeasureProcessCPUTime();