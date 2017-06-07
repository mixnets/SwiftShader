#include <string.h>
#include <map>
#include "utils.h"
#include "libVulkan.hpp"
#include "Context.h"

namespace vkutils
{
	void *findEntry(const char *name)
	{
		return vulkan::func_ptrs.find(std::string(name))->second;
	}
}

