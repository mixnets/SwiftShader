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

#ifndef VK_PIPELINE_CACHE_HPP_
#define VK_PIPELINE_CACHE_HPP_

#include "VkObject.hpp"
#include <functional>
#include <memory>
#include <map>
#include <mutex>
#include <vector>

namespace sw
{
	class ComputeProgram;
	class SpirvShader;
}

namespace vk
{

class PipelineLayout;
class RenderPass;

class PipelineCache : public Object<PipelineCache, VkPipelineCache>
{
public:
	PipelineCache(const VkPipelineCacheCreateInfo* pCreateInfo, void* mem);
	virtual ~PipelineCache();
	void destroy(const VkAllocationCallbacks* pAllocator);

	static size_t ComputeRequiredAllocationSize(const VkPipelineCacheCreateInfo* pCreateInfo);

	VkResult getData(size_t* pDataSize, void* pData);
	VkResult merge(uint32_t srcCacheCount, const VkPipelineCache* pSrcCaches);

	struct SpirvShaderKey
	{
		const uint32_t codeSerialID;
		const VkShaderStageFlagBits pipelineStage;
		const std::string entryPointName;
		const std::vector<uint32_t> insns;
		const vk::RenderPass *renderPass;
		const uint32_t subpassIndex;
		const VkSpecializationInfo* specializationInfo; // FIXME: needs deep copy

		bool operator<(const SpirvShaderKey &other) const
		{
			return (codeSerialID < other.codeSerialID) &&
			       (pipelineStage < other.pipelineStage) &&
			       (entryPointName < other.entryPointName) &&
			       (insns < other.insns) &&
			       (renderPass < other.renderPass) &&
			       (subpassIndex < other.subpassIndex) &&
			       (specializationInfo < other.specializationInfo); // FIXME: needs deep compare
		}
	};

	std::mutex& getSpirvShadersMutex() { return spirvShadersMutex; }
	const std::shared_ptr<sw::SpirvShader>* findSpirvShader(const PipelineCache::SpirvShaderKey& key) const;
	void storeSpirvShader(const std::shared_ptr<sw::SpirvShader> &shader, const PipelineCache::SpirvShaderKey& key);

	struct ComputeProgramKey
	{
		const sw::SpirvShader* shader;
		const vk::PipelineLayout* layout;

		bool operator<(const ComputeProgramKey &other) const
		{
			return (shader < other.shader) && (layout < other.layout);
		}
	};

	std::mutex& getComputeProgramsMutex() { return computeProgramsMutex; }
	const std::shared_ptr<sw::ComputeProgram>* findComputeProgram(const PipelineCache::ComputeProgramKey& key) const;
	void storeComputeProgram(const std::shared_ptr<sw::ComputeProgram> &computeProgram, const PipelineCache::ComputeProgramKey& key);

private:
	struct CacheHeader
	{
		uint32_t headerLength;
		uint32_t headerVersion;
		uint32_t vendorID;
		uint32_t deviceID;
		uint8_t  pipelineCacheUUID[VK_UUID_SIZE];
	};

	size_t dataSize = 0;
	uint8_t* data   = nullptr;

	std::mutex spirvShadersMutex;
	std::map<SpirvShaderKey, std::shared_ptr<sw::SpirvShader>> spirvShaders;

	std::mutex computeProgramsMutex;
	std::map<ComputeProgramKey, std::shared_ptr<sw::ComputeProgram>> computePrograms;
};

static inline PipelineCache* Cast(VkPipelineCache object)
{
	return PipelineCache::Cast(object);
}

} // namespace vk

#endif // VK_PIPELINE_CACHE_HPP_
