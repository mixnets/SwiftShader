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

#ifndef VK_QUERY_POOL_HPP_
#define VK_QUERY_POOL_HPP_

#include "VkObject.hpp"

namespace vk
{

class QueryPool : public Object<QueryPool, VkQueryPool>
{
public:
	QueryPool(const VkQueryPoolCreateInfo* pCreateInfo, void* mem) :
		queryCount(pCreateInfo->queryCount)
	{
		// According to the Vulkan spec, section 34.1. Features:
		// "pipelineStatisticsQuery specifies whether the pipeline statistics
		//  queries are supported. If this feature is not enabled, queries of
		//  type VK_QUERY_TYPE_PIPELINE_STATISTICS cannot be created, and
		//  none of the VkQueryPipelineStatisticFlagBits bits can be set in the
		//  pipelineStatistics member of the VkQueryPoolCreateInfo structure."
		if(pCreateInfo->queryType == VK_QUERY_TYPE_PIPELINE_STATISTICS)
		{
			UNIMPLEMENTED();
		}
	}

	~QueryPool() = delete;

	static size_t ComputeRequiredAllocationSize(const VkQueryPoolCreateInfo* pCreateInfo)
	{
		return 0;
	}

	void getResults(uint32_t pFirstQuery, uint32_t pQueryCount, size_t pDataSize,
		            void* pData, VkDeviceSize pStride, VkQueryResultFlags pFlags) const
	{
		// dataSize must be large enough to contain the result of each query
		ASSERT(static_cast<size_t>(pStride * pQueryCount) <= pDataSize);

		// The sum of firstQuery and queryCount must be less than or equal to the number of queries
		ASSERT((pFirstQuery + pQueryCount) <= queryCount);

		char* data = static_cast<char*>(pData);
		for(uint32_t i = 0; i < pQueryCount; i++, data += pStride)
		{
			// FIXME: write actual results instead of 0 once they are available
			if(pFlags & VK_QUERY_RESULT_64_BIT)
			{
				*reinterpret_cast<uint64_t*>(data) = 0;
			}
			else
			{
				*reinterpret_cast<uint32_t*>(data) = 0;
			}
		}
	}

private:
	uint32_t                         queryCount;
};

static inline QueryPool* Cast(VkQueryPool object)
{
	return reinterpret_cast<QueryPool*>(object);
}

} // namespace vk

#endif // VK_QUERY_POOL_HPP_
