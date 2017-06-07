#include <string.h>
#include <map>
#include "utils.h"
#include "libVulkan.hpp"
#include "Context.h"

namespace vkutils
{
  void *GetNext(struct OutArray *pArray, size_t element_size)
  {
    void *p = NULL;

    pArray->desiredLength += 1;

    if (*pArray->pFilledLength >= pArray->capacity)
    {
      return NULL;
    }

    if (pArray->pData != NULL)
    {
      p = &pArray->pData + (*pArray->pFilledLength) * element_size;
    }

    *pArray->pFilledLength += 1;

    return p;
  }

	void *findEntry(const char *name)
	{
		return vulkan::func_ptrs.find(std::string(name))->second;
	}

	void * Allocate(const VkAllocationCallbacks * pAlloc, size_t size, size_t align, VkSystemAllocationScope scope)
	{
		return pAlloc->pfnAllocation(pAlloc->pUserData, size, align, scope);
	}

	void * Alloc(const VkAllocationCallbacks * pParent, const VkAllocationCallbacks * pAlloc, size_t size, size_t align, VkSystemAllocationScope scope)
	{
		if (pAlloc)
		{
			return Allocate(pAlloc, size, align, scope);
		}
		else
		{
			return Allocate(pParent, size, align, scope);
		}
	}

	void Free(VkAllocationCallbacks * pAlloc, void * pData)
	{
		if (pData == NULL)
		{
			return;
		}

		pAlloc->pfnFree(pAlloc->pUserData, pData);
	}

}

