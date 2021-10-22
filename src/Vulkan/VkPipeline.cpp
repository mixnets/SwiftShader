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

#include "VkPipeline.hpp"

#include "VkDestroy.hpp"
#include "VkDevice.hpp"
#include "VkPipelineCache.hpp"
#include "VkPipelineLayout.hpp"
#include "VkRenderPass.hpp"
#include "VkShaderModule.hpp"
#include "VkStringify.hpp"
#include "Pipeline/ComputeProgram.hpp"
#include "Pipeline/SpirvShader.hpp"

#include "marl/trace.h"

#include "spirv-tools/optimizer.hpp"

#include <iostream>
#include <fstream>

namespace {

// optimizeSpirv() applies and freezes specializations into constants, and runs spirv-opt.
sw::SpirvBinary optimizeSpirv(const vk::PipelineCache::SpirvBinaryKey &key)
{
	const sw::SpirvBinary &code = key.getBinary();
	const VkSpecializationInfo *specializationInfo = key.getSpecializationInfo();
	bool optimize = key.getOptimization();

	spvtools::Optimizer opt{ vk::SPIRV_VERSION };

	opt.SetMessageConsumer([](spv_message_level_t level, const char *source, const spv_position_t &position, const char *message) {
		switch(level)
		{
		case SPV_MSG_FATAL: sw::warn("SPIR-V FATAL: %d:%d %s\n", int(position.line), int(position.column), message);
		case SPV_MSG_INTERNAL_ERROR: sw::warn("SPIR-V INTERNAL_ERROR: %d:%d %s\n", int(position.line), int(position.column), message);
		case SPV_MSG_ERROR: sw::warn("SPIR-V ERROR: %d:%d %s\n", int(position.line), int(position.column), message);
		case SPV_MSG_WARNING: sw::warn("SPIR-V WARNING: %d:%d %s\n", int(position.line), int(position.column), message);
		case SPV_MSG_INFO: sw::trace("SPIR-V INFO: %d:%d %s\n", int(position.line), int(position.column), message);
		case SPV_MSG_DEBUG: sw::trace("SPIR-V DEBUG: %d:%d %s\n", int(position.line), int(position.column), message);
		default: sw::trace("SPIR-V MESSAGE: %d:%d %s\n", int(position.line), int(position.column), message);
		}
	});

	// If the pipeline uses specialization, apply the specializations before freezing
	if(specializationInfo)
	{
		std::unordered_map<uint32_t, std::vector<uint32_t>> specializations;
		const uint8_t *specializationData = static_cast<const uint8_t *>(specializationInfo->pData);

		for(uint32_t i = 0; i < specializationInfo->mapEntryCount; i++)
		{
			const VkSpecializationMapEntry &entry = specializationInfo->pMapEntries[i];
			const uint8_t *value_ptr = specializationData + entry.offset;
			std::vector<uint32_t> value(reinterpret_cast<const uint32_t *>(value_ptr),
			                            reinterpret_cast<const uint32_t *>(value_ptr + entry.size));
			specializations.emplace(entry.constantID, std::move(value));
		}

		opt.RegisterPass(spvtools::CreateSetSpecConstantDefaultValuePass(specializations));
	}

	if(optimize)
	{
		// Full optimization list taken from spirv-opt.
		opt.RegisterPerformancePasses();
	}

	spvtools::OptimizerOptions optimizerOptions = {};
#if defined(NDEBUG)
	optimizerOptions.set_run_validator(false);
#else
	optimizerOptions.set_run_validator(true);
	spvtools::ValidatorOptions validatorOptions = {};
	validatorOptions.SetScalarBlockLayout(true);            // VK_EXT_scalar_block_layout
	validatorOptions.SetUniformBufferStandardLayout(true);  // VK_KHR_uniform_buffer_standard_layout
	optimizerOptions.set_validator_options(validatorOptions);
#endif

	sw::SpirvBinary optimized;


	   //    uint32_t pCode[] = { 119734787, 65536, 917504, 69, 0, 131089, 1, 589834, 1599492179, 1196379975, 1751074124, 1600942956, 1668183398, 1852795252, 1953066081, 12665, 458762, 1599492179, 1196379975, 1969177932, 1601332595, 1701869940, 0, 196622, 0, 1, 720911, 5, 1, 1852399981, 808464479, 1647849520, 926441272, 892418355, 13368, 2, 3, 393232, 1, 17, 5, 5, 5, 196611, 5, 500, 393221, 4, 1701869940, 778318638, 1734438249, 101, 393221, 5, 1768058177, 1450471013, 1869898597, 114, 589829, 6, 1701996868, 1869182051, 1282171246, 1952999273, 1684105299, 1852405615, 103, 393221, 7, 1701869940, 778318638, 1734438249, 101, 458757, 8, 1098151247, 1701405293, 1700164718, 1919906915, 0, 393221, 9, 1701869940, 778318638, 1734438249, 101, 655365, 10, 1148482895, 1667592809, 1852795252, 1766616161, 1400137831, 1868849512, 1735289207, 0, 393221, 11, 1701869940, 1816601646, 1818321519, 115, 524294, 11, 0, 1918989395, 1936674932, 1817144905, 1819235940, 6647157, 524294, 11, 1, 1918989395, 1936674932, 1699638857, 1819235959, 6647157, 327685, 12, 1869367076, 1936482658, 0, 524293, 1, 1852399981, 808464479, 1647849520, 926441272, 892418355, 13368, 262215, 2, 11, 26, 398848, 2, 5635, 1197430355, 1886744434, 17481, 262215, 3, 11, 27, 529920, 3, 5635, 1197430355, 1886744434, 1701996628, 1145660513, 0, 262215, 5, 34, 0, 262215, 5, 33, 1, 262215, 6, 34, 0, 262215, 6, 33, 2, 262215, 8, 34, 0, 262215, 8, 33, 3, 262215, 10, 34, 0, 262215, 10, 33, 4, 262215, 12, 34, 0, 262215, 12, 33, 0, 398848, 5, 5636, 1954047348, 862286453, 100, 398848, 6, 5636, 1954047348, 862286453, 100, 398848, 8, 5636, 1702131570, 1920300152, 6566757, 398848, 10, 5636, 1702131570, 1920300152, 6566757, 327752, 11, 0, 35, 0, 327752, 11, 1, 35, 4, 196679, 11, 2, 262165, 13, 32, 0, 262187, 13, 14, 5, 262165, 15, 32, 1, 262187, 15, 16, 0, 262187, 13, 17, 256, 262167, 18, 13, 3, 393260, 18, 19, 14, 14, 14, 262187, 15, 20, 1, 262187, 13, 21, 0, 196630, 22, 32, 589849, 4, 22, 2, 2, 0, 0, 1, 0, 262176, 23, 0, 4, 589849, 7, 22, 2, 2, 0, 0, 2, 1, 262176, 24, 0, 7, 589849, 9, 22, 2, 2, 0, 0, 2, 3, 262176, 25, 0, 9, 262174, 11, 13, 13, 262176, 26, 2, 11, 262176, 27, 1, 18, 131091, 28, 196641, 29, 28, 262176, 30, 2, 13, 262167, 31, 22, 4, 262167, 32, 22, 3, 262203, 23, 5, 0, 262203, 23, 6, 0, 262203, 24, 8, 0, 262203, 25, 10, 0, 262203, 26, 12, 2, 262203, 27, 2, 1, 262203, 27, 3, 1, 327734, 28, 1, 0, 29, 131320, 33, 262205, 18, 34, 2, 262205, 18, 35, 3, 327745, 30, 36, 12, 16, 262205, 13, 37, 36, 327761, 13, 38, 34, 0, 327808, 13, 39, 37, 38, 262268, 15, 40, 39, 262268, 13, 41, 40, 327817, 13, 42, 41, 17, 327814, 13, 43, 41, 17, 327817, 13, 44, 43, 17, 327814, 13, 45, 43, 17, 393296, 18, 46, 42, 44, 45, 327812, 18, 47, 46, 19, 327808, 18, 48, 47, 35, 327745, 30, 49, 12, 20, 262205, 13, 50, 49, 327808, 13, 51, 50, 38, 262268, 15, 52, 51, 262268, 13, 53, 52, 327817, 13, 54, 53, 17, 327814, 13, 55, 53, 17, 327817, 13, 56, 55, 17, 327814, 13, 57, 55, 17, 393296, 18, 58, 54, 56, 57, 327812, 18, 59, 58, 19, 327808, 18, 60, 59, 35, 262205, 4, 61, 5, 458847, 31, 62, 61, 48, 2, 21, 524367, 32, 63, 62, 62, 0, 1, 2, 262205, 7, 64, 8, 327779, 64, 60, 63, 0, 262205, 4, 65, 6, 458847, 31, 66, 65, 48, 2, 21, 327761, 22, 67, 66, 0, 262205, 9, 68, 10, 327779, 68, 60, 67, 0, 65789, 65592 };

	//opt.Run(pCode, sizeof(pCode) / sizeof(uint32_t) /* code.data(), code.size()*/, &optimized, optimizerOptions);

	opt.Run(code.data(), code.size(), &optimized, optimizerOptions);
	ASSERT(optimized.size() > 0);

	if(true)
	{
		spvtools::SpirvTools core(vk::SPIRV_VERSION);
		std::string preOpt;
		core.Disassemble(code, &preOpt, SPV_BINARY_TO_TEXT_OPTION_NONE);
		std::string postOpt;
		core.Disassemble(optimized, &postOpt, SPV_BINARY_TO_TEXT_OPTION_NONE);

		std::ofstream file;
		file.open("C:\\src\\SwiftShader\\out\\build\\x64-Debug\\Windows\\out.txt", std::ios_base::app);

		//std::cout << "PRE-OPT: " << preOpt << std::endl
		//          << "POST-OPT: " << postOpt << std::endl;
		file << postOpt << std::endl;

		file.close();
	}

	return optimized;
}

std::shared_ptr<sw::ComputeProgram> createProgram(vk::Device *device, std::shared_ptr<sw::SpirvShader> shader, const vk::PipelineLayout *layout)
{
	MARL_SCOPED_EVENT("createProgram");

	vk::DescriptorSet::Bindings descriptorSets;  // TODO(b/129523279): Delay code generation until dispatch time.
	// TODO(b/119409619): use allocator.
	auto program = std::make_shared<sw::ComputeProgram>(device, shader, layout, descriptorSets);
	program->generate();
	program->finalize("ComputeProgram");

	return program;
}

}  // anonymous namespace

namespace vk {

Pipeline::Pipeline(PipelineLayout *layout, Device *device)
    : layout(layout)
    , device(device)
    , robustBufferAccess(device->getEnabledFeatures().robustBufferAccess)
{
	layout->incRefCount();
}

void Pipeline::destroy(const VkAllocationCallbacks *pAllocator)
{
	destroyPipeline(pAllocator);

	vk::release(static_cast<VkPipelineLayout>(*layout), pAllocator);
}

GraphicsPipeline::GraphicsPipeline(const VkGraphicsPipelineCreateInfo *pCreateInfo, void *mem, Device *device)
    : Pipeline(vk::Cast(pCreateInfo->layout), device)
    , state(device, pCreateInfo, layout, robustBufferAccess)
    , inputs(pCreateInfo->pVertexInputState)
{
}

void GraphicsPipeline::destroyPipeline(const VkAllocationCallbacks *pAllocator)
{
	vertexShader.reset();
	fragmentShader.reset();
}

size_t GraphicsPipeline::ComputeRequiredAllocationSize(const VkGraphicsPipelineCreateInfo *pCreateInfo)
{
	return 0;
}

void GraphicsPipeline::getIndexBuffers(uint32_t count, uint32_t first, bool indexed, std::vector<std::pair<uint32_t, void *>> *indexBuffers) const
{
	indexBuffer.getIndexBuffers(state.getTopology(), count, first, indexed, state.hasPrimitiveRestartEnable(), indexBuffers);
}

bool GraphicsPipeline::containsImageWrite() const
{
	return (vertexShader.get() && vertexShader->containsImageWrite()) ||
	       (fragmentShader.get() && fragmentShader->containsImageWrite());
}

void GraphicsPipeline::setShader(const VkShaderStageFlagBits &stage, const std::shared_ptr<sw::SpirvShader> spirvShader)
{
	switch(stage)
	{
	case VK_SHADER_STAGE_VERTEX_BIT:
		ASSERT(vertexShader.get() == nullptr);
		vertexShader = spirvShader;
		break;

	case VK_SHADER_STAGE_FRAGMENT_BIT:
		ASSERT(fragmentShader.get() == nullptr);
		fragmentShader = spirvShader;
		break;

	case VK_SHADER_STAGE_GEOMETRY_BIT:
		break;

	default:
		UNSUPPORTED("Unsupported stage");
		break;
	}
}

const std::shared_ptr<sw::SpirvShader> GraphicsPipeline::getShader(const VkShaderStageFlagBits &stage) const
{
	switch(stage)
	{
	case VK_SHADER_STAGE_VERTEX_BIT:
		return vertexShader;
	case VK_SHADER_STAGE_FRAGMENT_BIT:
		return fragmentShader;
	default:
		UNSUPPORTED("Unsupported stage");
		return fragmentShader;
	}
}

void GraphicsPipeline::compileShaders(const VkAllocationCallbacks *pAllocator, const VkGraphicsPipelineCreateInfo *pCreateInfo, PipelineCache *pPipelineCache)
{
	for(auto pStage = pCreateInfo->pStages; pStage != pCreateInfo->pStages + pCreateInfo->stageCount; pStage++)
	{
		if(pStage->flags != 0)
		{
			// Vulkan 1.2: "flags must be 0"
			UNSUPPORTED("pStage->flags %d", int(pStage->flags));
		}

		auto dbgctx = device->getDebuggerContext();
		// Do not optimize the shader if we have a debugger context.
		// Optimization passes are likely to damage debug information, and reorder
		// instructions.
		const bool optimize = !dbgctx;

		const ShaderModule *module = vk::Cast(pStage->module);
		const PipelineCache::SpirvBinaryKey key(module->getBinary(), pStage->pSpecializationInfo, optimize);

		sw::SpirvBinary spirv;

		if(pPipelineCache)
		{
			spirv = pPipelineCache->getOrOptimizeSpirv(key, [&] {
				return optimizeSpirv(key);
			});
		}
		else
		{
			spirv = optimizeSpirv(key);

			// If the pipeline does not have specialization constants, there's a 1-to-1 mapping between the unoptimized and optimized SPIR-V,
			// so we should use a 1-to-1 mapping of the identifiers to avoid JIT routine recompiles.
			if(!key.getSpecializationInfo())
			{
				spirv.mapOptimizedIdentifier(key.getBinary());
			}
		}

		// TODO(b/201798871): use allocator.
		auto shader = std::make_shared<sw::SpirvShader>(pStage->stage, pStage->pName, spirv,
		                                                vk::Cast(pCreateInfo->renderPass), pCreateInfo->subpass, robustBufferAccess, dbgctx);

		setShader(pStage->stage, shader);
	}
}

ComputePipeline::ComputePipeline(const VkComputePipelineCreateInfo *pCreateInfo, void *mem, Device *device)
    : Pipeline(vk::Cast(pCreateInfo->layout), device)
{
}

void ComputePipeline::destroyPipeline(const VkAllocationCallbacks *pAllocator)
{
	shader.reset();
	program.reset();
}

size_t ComputePipeline::ComputeRequiredAllocationSize(const VkComputePipelineCreateInfo *pCreateInfo)
{
	return 0;
}

void ComputePipeline::compileShaders(const VkAllocationCallbacks *pAllocator, const VkComputePipelineCreateInfo *pCreateInfo, PipelineCache *pPipelineCache)
{
	auto &stage = pCreateInfo->stage;
	const ShaderModule *module = vk::Cast(stage.module);

	ASSERT(shader.get() == nullptr);
	ASSERT(program.get() == nullptr);

	auto dbgctx = device->getDebuggerContext();
	// Do not optimize the shader if we have a debugger context.
	// Optimization passes are likely to damage debug information, and reorder
	// instructions.
	const bool optimize = !dbgctx;

	const PipelineCache::SpirvBinaryKey shaderKey(module->getBinary(), stage.pSpecializationInfo, optimize);

	sw::SpirvBinary spirv;

	if(pPipelineCache)
	{
		spirv = pPipelineCache->getOrOptimizeSpirv(shaderKey, [&] {
			return optimizeSpirv(shaderKey);
		});
	}
	else
	{
		spirv = optimizeSpirv(shaderKey);

		// If the pipeline does not have specialization constants, there's a 1-to-1 mapping between the unoptimized and optimized SPIR-V,
		// so we should use a 1-to-1 mapping of the identifiers to avoid JIT routine recompiles.
		if(!shaderKey.getSpecializationInfo())
		{
			spirv.mapOptimizedIdentifier(shaderKey.getBinary());
		}
	}

	// TODO(b/201798871): use allocator.
	shader = std::make_shared<sw::SpirvShader>(stage.stage, stage.pName, spirv,
	                                           nullptr, 0, robustBufferAccess, dbgctx);

	const PipelineCache::ComputeProgramKey programKey(shader->getIdentifier(), layout->identifier);

	if(pPipelineCache)
	{
		program = pPipelineCache->getOrCreateComputeProgram(programKey, [&] {
			return createProgram(device, shader, layout);
		});
	}
	else
	{
		program = createProgram(device, shader, layout);
	}
}

void ComputePipeline::run(uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ,
                          uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ,
                          vk::DescriptorSet::Array const &descriptorSetObjects,
                          vk::DescriptorSet::Bindings const &descriptorSets,
                          vk::DescriptorSet::DynamicOffsets const &descriptorDynamicOffsets,
                          vk::Pipeline::PushConstantStorage const &pushConstants)
{
	ASSERT_OR_RETURN(program != nullptr);
	program->run(
	    descriptorSetObjects, descriptorSets, descriptorDynamicOffsets, pushConstants,
	    baseGroupX, baseGroupY, baseGroupZ,
	    groupCountX, groupCountY, groupCountZ);
}

}  // namespace vk
