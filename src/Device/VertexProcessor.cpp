// Copyright 2016 The SwiftShader Authors. All Rights Reserved.
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

#include "VertexProcessor.hpp"

#include "Pipeline/Constants.hpp"
#include "Pipeline/VertexProgram.hpp"
#include "System/Debug.hpp"
#include "System/Math.hpp"
#include "Vulkan/VkPipelineLayout.hpp"

#include <Windows.h>

#include <cstring>

bool IsExecutable(void *ptr)
{
	MEMORY_BASIC_INFORMATION info{};
	VirtualQuery(ptr, &info, sizeof(info));
#if 1
	printf("\n");
	printf("-------------------------------\n");
	printf("BaseAddress:       %p\n", info.BaseAddress);
	printf("AllocationBase:    %p\n", info.AllocationBase);
	printf("AllocationProtect: 0x%lx\n", info.AllocationProtect);
	printf("PartitionId:       0x%x\n", info.PartitionId);
	printf("RegionSize:        0x%llx\n", info.RegionSize);
	printf("State:             %s\n", [&]{
		switch(info.State) {
			case MEM_COMMIT: return "MEM_COMMIT";
			case MEM_FREE: return "MEM_FREE";
			case MEM_RESERVE: return "MEM_RESERVE";
			default: return "unknown";
		}
	}());
	printf("Protect:          %s\n",  [&]{
		std::string out;
		if (info.Protect & PAGE_EXECUTE) out += " PAGE_EXECUTE";
		if (info.Protect & PAGE_EXECUTE_READ) out += " PAGE_EXECUTE_READ";
		if (info.Protect & PAGE_EXECUTE_READWRITE) out += " PAGE_EXECUTE_READWRITE";
		if (info.Protect & PAGE_EXECUTE_WRITECOPY) out += " PAGE_EXECUTE_WRITECOPY";
		if (info.Protect & PAGE_GUARD) out += " PAGE_GUARD";
		if (info.Protect & PAGE_NOACCESS) out += " PAGE_NOACCESS";
		if (info.Protect & PAGE_NOCACHE) out += " PAGE_NOCACHE";
		if (info.Protect & PAGE_READONLY) out += " PAGE_READONLY";
		if (info.Protect & PAGE_READWRITE) out += " PAGE_READWRITE";
		if (info.Protect & PAGE_TARGETS_INVALID) out += " PAGE_TARGETS_INVALID";
		if (info.Protect & PAGE_TARGETS_NO_UPDATE) out += " PAGE_TARGETS_NO_UPDATE";
		if (info.Protect & PAGE_WRITECOMBINE) out += " PAGE_WRITECOMBINE";
		if (info.Protect & PAGE_WRITECOPY) out += " PAGE_WRITECOPY";
		return out;
	}().c_str());
	printf("Type:             %s\n",  [&]{
		std::string out;
		if (info.Type & MEM_IMAGE) out += " MEM_IMAGE";
		if (info.Type & MEM_MAPPED) out += " MEM_MAPPED";
		if (info.Type & MEM_PRIVATE) out += " MEM_PRIVATE";
		return out;
	}().c_str());
	printf("-------------------------------\n");
	printf("\n");
#endif
	return info.Protect & (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY);
}

static void *P = nullptr;
bool DoCheck()
{
	return !P || IsExecutable(P);
}

namespace sw {

void VertexCache::clear()
{
	for(uint32_t i = 0; i < SIZE; i++)
	{
		tag[i] = 0xFFFFFFFF;
	}
}

uint32_t VertexProcessor::States::computeHash()
{
	uint32_t *state = reinterpret_cast<uint32_t *>(this);
	uint32_t hash = 0;

	for(unsigned int i = 0; i < sizeof(States) / sizeof(uint32_t); i++)
	{
		hash ^= state[i];
	}

	return hash;
}

bool VertexProcessor::State::operator==(const State &state) const
{
	if(hash != state.hash)
	{
		return false;
	}

	return *static_cast<const States *>(this) == static_cast<const States &>(state);
}

VertexProcessor::VertexProcessor()
{
	setRoutineCacheSize(1024);
}

void VertexProcessor::setRoutineCacheSize(int cacheSize)
{
	routineCache = std::make_unique<RoutineCacheType>(clamp(cacheSize, 1, 65536));
}

const VertexProcessor::State VertexProcessor::update(const vk::GraphicsState &pipelineState, const sw::SpirvShader *vertexShader, const vk::Inputs &inputs)
{
	const vk::VertexInputInterfaceState &vertexInputInterfaceState = pipelineState.getVertexInputInterfaceState();
	const vk::PreRasterizationState &preRasterizationState = pipelineState.getPreRasterizationState();

	State state;

	state.shaderID = vertexShader->getIdentifier();
	state.pipelineLayoutIdentifier = preRasterizationState.getPipelineLayout()->identifier;
	state.robustBufferAccess = vertexShader->getRobustBufferAccess();
	state.isPoint = vertexInputInterfaceState.getTopology() == VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
	state.depthClipEnable = preRasterizationState.getDepthClipEnable();
	state.depthClipNegativeOneToOne = preRasterizationState.getDepthClipNegativeOneToOne();

	for(size_t i = 0; i < MAX_INTERFACE_COMPONENTS / 4; i++)
	{
		state.input[i].format = inputs.getStream(i).format;
		// TODO: get rid of attribType -- just keep the VK format all the way through, this fully determines
		// how to handle the attribute.
		state.input[i].attribType = vertexShader->inputs[i * 4].Type;
	}

	state.hash = state.computeHash();

	return state;
}

VertexProcessor::RoutineType VertexProcessor::routine(const State &state,
                                                      const vk::PipelineLayout *pipelineLayout,
                                                      const SpirvShader *vertexShader,
                                                      const vk::DescriptorSet::Bindings &descriptorSets)
{
	auto routine = routineCache->lookup(state);

	if(!routine)  // Create one
	{
		static int i = 0;
		i++;
		printf("\n");
		printf("=======================================================\n");
		printf("= ROUTINE: %d\n", i);
		VertexRoutine *generator = new VertexProgram(state, pipelineLayout, vertexShader, descriptorSets);
		generator->generate();
		routine = (*generator)("VertexRoutine_%0.8X", state.shaderID);
		delete generator;

		printf("VertexProcessor::routine(): id: %0.8llX, ptr: %p\n", state.shaderID, routine.function);

		routineCache->add(state, routine);

		if(i == 18)
		{
			P = reinterpret_cast<void *>(routine.function);
			printf(" ROUTINE: 0x%p\n", routine.function);
		}
		printf("=======================================================\n");
		printf("\n");
	}

	return routine;
}

}  // namespace sw
