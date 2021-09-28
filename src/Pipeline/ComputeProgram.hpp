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

#ifndef sw_ComputeProgram_hpp
#define sw_ComputeProgram_hpp

#include "SpirvShader.hpp"

#include "Reactor/Coroutine.hpp"
#include "Vulkan/VkDescriptorSet.hpp"
#include "Vulkan/VkPipeline.hpp"

#include <functional>

namespace vk {
class Device;
class PipelineLayout;
}  // namespace vk

namespace sw {

using namespace rr;

class DescriptorSetsLayout;
struct Constants;

// ComputeProgram builds a SPIR-V compute shader.
class ComputeProgram : public Coroutine<SpirvShader::YieldResult(
                           void *data,
                           int32_t workgroupX,
                           int32_t workgroupY,
                           int32_t workgroupZ,
                           void *workgroupMemory,
                           int32_t firstSubgroup,
                           int32_t subgroupCount)>
{
public:
	ComputeProgram(vk::Device *device, const SpirvShader *spirvShader, vk::PipelineLayout const *pipelineLayout, const vk::DescriptorSet::Bindings &descriptorSets);

	virtual ~ComputeProgram();

	// generate builds the shader program.
	//void generate();

	// run executes the compute shader routine for all workgroups.
	void run(
	    vk::DescriptorSet::Array const &descriptorSetObjects,
	    vk::DescriptorSet::Bindings const &descriptorSetBindings,
	    vk::DescriptorSet::DynamicOffsets const &descriptorDynamicOffsets,
	    vk::Pipeline::PushConstantStorage const &pushConstants,
	    uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ,
	    uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);

protected:
	void emit(SpirvRoutine *routine, const SpirvShader *spirvShader);
	void setWorkgroupBuiltins(const SpirvShader *spirvShader, Pointer<Byte> data, SpirvRoutine *routine, Int workgroupID[3]);
	void setSubgroupBuiltins(const SpirvShader *spirvShader, Pointer<Byte> data, SpirvRoutine *routine, Int workgroupID[3], SIMD::Int localInvocationIndex, Int subgroupIndex);

	vk::Device *const device;
	const vk::PipelineLayout *const pipelineLayout;  // Reference held by vk::Pipeline
	const vk::DescriptorSet::Bindings &descriptorSets;

	const SpirvShader::ExecutionModes executionModes;
	const size_t workgroupMemorySize;
	const bool containsControlBarriers;
	const bool containsImageWrite;
};

}  // namespace sw

#endif  // sw_ComputeProgram_hpp
