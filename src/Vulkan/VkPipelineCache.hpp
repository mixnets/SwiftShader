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
#include <memory.h>

namespace vk
{

class PipelineCache;

static inline PipelineCache* Cast(VkPipelineCache object)
{
	return reinterpret_cast<PipelineCache*>(object);
}

class PipelineCache : public Object<PipelineCache, VkPipelineCache>
{
public:
	PipelineCache(const VkPipelineCacheCreateInfo* pCreateInfo, void* mem) :
		dataSize(pCreateInfo->initialDataSize), data(mem)
	{
		memcpy(data, pCreateInfo->pInitialData, dataSize);
	}

	~PipelineCache() = delete;

	static size_t ComputeRequiredAllocationSize(const VkPipelineCacheCreateInfo* pCreateInfo)
	{
		return pCreateInfo->initialDataSize;
	}

	size_t getDataSize() const
	{
		return dataSize;
	}

	void copyData(size_t size, void* pData) const
	{
		memcpy(pData, data, size);
	}

	void merge(uint32_t srcCacheCount, const VkPipelineCache* pSrcCaches)
	{
		for(uint32_t i = 0; i < srcCacheCount; i++)
		{
			if(Cast(pSrcCaches[i])->dataSize > 0)
			{
				UNIMPLEMENTED();
			}
		}
	}

private:
	size_t dataSize = 0;
	void* data = nullptr;
};

} // namespace vk

#endif // VK_PIPELINE_CACHE_HPP_
