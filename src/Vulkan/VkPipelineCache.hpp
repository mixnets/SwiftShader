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
#include <cstring>
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
		struct SpecializationInfo
		{
			SpecializationInfo(const VkSpecializationInfo* specializationInfo)
			{
				if(specializationInfo)
				{
					info = reinterpret_cast<VkSpecializationInfo*>(
						allocate(sizeof(VkSpecializationInfo), REQUIRED_MEMORY_ALIGNMENT, DEVICE_MEMORY));

					info->mapEntryCount = specializationInfo->mapEntryCount;
					if(specializationInfo->mapEntryCount > 0)
					{
						size_t entriesSize = specializationInfo->mapEntryCount * sizeof(VkSpecializationMapEntry);
						VkSpecializationMapEntry* mapEntries = reinterpret_cast<VkSpecializationMapEntry*>(
							allocate(entriesSize, REQUIRED_MEMORY_ALIGNMENT, DEVICE_MEMORY));
						memcpy(mapEntries, specializationInfo->pMapEntries, entriesSize);
						info->pMapEntries = mapEntries;
					}

					info->dataSize = specializationInfo->dataSize;
					if(specializationInfo->dataSize > 0)
					{
						void* data = allocate(specializationInfo->dataSize, REQUIRED_MEMORY_ALIGNMENT, DEVICE_MEMORY);
						memcpy(data, specializationInfo->pData, specializationInfo->dataSize);
						info->pData = data;
					}
				}
			}

			bool operator<(const SpecializationInfo& specializationInfo) const
			{
				return (info && specializationInfo.info) ?
					   ((info->mapEntryCount < specializationInfo.info->mapEntryCount) ||
			            (info->dataSize < specializationInfo.info->dataSize) ||
				        ((info->mapEntryCount > 0) &&
				         (memcmp(info->pMapEntries, specializationInfo.info->pMapEntries, info->mapEntryCount * sizeof(VkSpecializationMapEntry)) < 0)) ||
				        ((info->dataSize > 0) &&
				         (memcmp(info->pData, specializationInfo.info->pData, info->dataSize) < 0))) :
				       (info < specializationInfo.info);
			}

			VkSpecializationInfo* get() const
			{
				return info;
			}

		private:
			VkSpecializationInfo* info = nullptr;
		};

		SpirvShaderKey(const VkShaderStageFlagBits pipelineStage,
		               const std::string& entryPointName,
		               const std::vector<uint32_t>& insns,
		               const vk::RenderPass *renderPass,
		               const uint32_t subpassIndex,
		               const VkSpecializationInfo* specializationInfo) :
			pipelineStage(pipelineStage), entryPointName(entryPointName), insns(insns), renderPass(renderPass),
			subpassIndex(subpassIndex), specializationInfo(specializationInfo)
		{
		}

		bool operator<(const SpirvShaderKey &other) const
		{
			return (pipelineStage < other.pipelineStage) ||
			       (renderPass < other.renderPass) ||
			       (subpassIndex < other.subpassIndex) ||
			       (insns.size() < other.insns.size()) ||
			       (entryPointName.size() < other.entryPointName.size()) ||
				   (memcmp(entryPointName.c_str(), other.entryPointName.c_str(), entryPointName.size()) < 0) ||
				   (memcmp(insns.data(), other.insns.data(), insns.size() * sizeof(uint32_t)) < 0) ||
			       (specializationInfo < other.specializationInfo);
		}

		const VkShaderStageFlagBits& getPipelineStage() const { return pipelineStage; }
		const std::string& getEntryPointName() const { return entryPointName; }
		const std::vector<uint32_t>& getInsns() const { return insns; }
		const vk::RenderPass *getRenderPass() const { return renderPass; }
		uint32_t getSubpassIndex() const { return subpassIndex; }
		const VkSpecializationInfo *getSpecializationInfo() const { return specializationInfo.get(); }

	private:
		const VkShaderStageFlagBits pipelineStage;
		const std::string entryPointName;
		const std::vector<uint32_t> insns;
		const vk::RenderPass *renderPass;
		const uint32_t subpassIndex;
		const SpecializationInfo specializationInfo;
	};

	std::mutex& getShaderMutex() { return spirvShadersMutex; }
	const std::shared_ptr<sw::SpirvShader>* operator[](const PipelineCache::SpirvShaderKey& key) const;
	void insert(const PipelineCache::SpirvShaderKey& key, const std::shared_ptr<sw::SpirvShader> &shader);

	struct ComputeProgramKey
	{
		ComputeProgramKey(const sw::SpirvShader* shader, const vk::PipelineLayout* layout) :
			shader(shader), layout(layout)
		{}

		bool operator<(const ComputeProgramKey &other) const
		{
			return (shader < other.shader) || (layout < other.layout);
		}

		const sw::SpirvShader* getShader() const { return shader; }
		const vk::PipelineLayout* getLayout() const { return layout; }

	private:
		const sw::SpirvShader* shader;
		const vk::PipelineLayout* layout;
	};

	std::mutex& getProgramMutex() { return computeProgramsMutex; }
	const std::shared_ptr<sw::ComputeProgram>* operator[](const PipelineCache::ComputeProgramKey& key) const;
	void insert(const PipelineCache::ComputeProgramKey& key, const std::shared_ptr<sw::ComputeProgram> &computeProgram);

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
