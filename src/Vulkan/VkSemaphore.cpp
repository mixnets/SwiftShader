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

#include "VkSemaphore.hpp"

#include "VkConfig.hpp"
#include "VkStringify.hpp"
#include "VkTimelineSemaphore.hpp"

#include "marl/blockingcall.h"
#include "marl/conditionvariable.h"

#include <chrono>
#include <climits>
#include <functional>
#include <memory>
#include <utility>

namespace {

std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> now()
{
	return std::chrono::time_point_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now());
}

}  // anonymous namespace

namespace vk {

// This is a base abstract class for all external semaphore implementations
// used in this source file.
class Semaphore::External
{
public:
	virtual ~External() = default;

	// Initialize new instance with a given initial state.
	virtual VkResult init(bool initialState) = 0;

	virtual bool tryWait() = 0;
	virtual void wait() = 0;
	virtual void signal() = 0;

	// For VK_KHR_external_semaphore_fd
	virtual VkResult importOpaqueFd(int fd)
	{
		return VK_ERROR_INVALID_EXTERNAL_HANDLE;
	}

	virtual VkResult exportOpaqueFd(int *pFd)
	{
		return VK_ERROR_INVALID_EXTERNAL_HANDLE;
	}

#if VK_USE_PLATFORM_FUCHSIA
	// For VK_FUCHSIA_external_semaphore
	virtual VkResult importHandle(zx_handle_t handle)
	{
		return VK_ERROR_INVALID_EXTERNAL_HANDLE;
	}
	virtual VkResult exportHandle(zx_handle_t *pHandle)
	{
		return VK_ERROR_INVALID_EXTERNAL_HANDLE;
	}
#endif
	// Pointer to previous temporary external instanc,e used for |tempExternal| only.
	External *previous = nullptr;
};

}  // namespace vk

#if SWIFTSHADER_EXTERNAL_SEMAPHORE_OPAQUE_FD
#	if defined(__linux__) || defined(__ANDROID__)
#		include "VkSemaphoreExternalLinux.hpp"
#	else
#		error "Missing VK_KHR_external_semaphore_fd implementation for this platform!"
#	endif
#elif VK_USE_PLATFORM_FUCHSIA
#	include "VkSemaphoreExternalFuchsia.hpp"
#endif

namespace vk {

// The bitmask of all external semaphore handle types supported by this source file.
static const VkExternalSemaphoreHandleTypeFlags kSupportedTypes =
#if SWIFTSHADER_EXTERNAL_SEMAPHORE_OPAQUE_FD
    VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT |
#endif
#if VK_USE_PLATFORM_FUCHSIA
    VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_TEMP_ZIRCON_EVENT_BIT_FUCHSIA |
#endif
    0;

namespace {

struct SemaphoreCreateInfo
{
	bool exportSemaphore = false;
	VkExternalSemaphoreHandleTypeFlags exportHandleTypes = 0;

	VkSemaphoreType semaphoreType = VK_SEMAPHORE_TYPE_BINARY;
	uint64_t initialPayload = 0;

	// Create a new instance. The external instance will be allocated only
	// the pCreateInfo->pNext chain indicates it needs to be exported.
	SemaphoreCreateInfo(const VkSemaphoreCreateInfo *pCreateInfo)
	{
		for(const auto *nextInfo = reinterpret_cast<const VkBaseInStructure *>(pCreateInfo->pNext);
		    nextInfo != nullptr; nextInfo = nextInfo->pNext)
		{
			switch(nextInfo->sType)
			{
				case VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO:
				{
					const auto *exportInfo = reinterpret_cast<const VkExportSemaphoreCreateInfo *>(nextInfo);
					exportSemaphore = true;
					exportHandleTypes = exportInfo->handleTypes;
					if((exportHandleTypes & ~kSupportedTypes) != 0)
					{
						UNSUPPORTED("exportInfo->handleTypes 0x%X (supports 0x%X)",
						            int(exportHandleTypes),
						            int(kSupportedTypes));
					}
				}
				break;
				case VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO:
				{
					const auto *tlsInfo = reinterpret_cast<const VkSemaphoreTypeCreateInfo *>(nextInfo);
					semaphoreType = tlsInfo->semaphoreType;
					initialPayload = tlsInfo->initialValue;
				}
				break;
				default:
					WARN("nextInfo->sType = %s", vk::Stringify(nextInfo->sType).c_str());
					break;
			}
		}
	}
};

}  // namespace

void Semaphore::wait()
{
	ASSERT(semaphoreType == VK_SEMAPHORE_TYPE_BINARY);
	marl::lock lock(mutex);
	External *ext = tempExternal ? tempExternal : external;
	if(ext)
	{
		if(!ext->tryWait())
		{
			// Dispatch the ext wait to a background thread.
			// Even if this creates a new thread on each
			// call, it is assumed that this is negligible
			// compared with the actual semaphore wait()
			// operation.
			lock.unlock_no_tsa();
			marl::blocking_call([ext]() {
				ext->wait();
			});
			lock.lock_no_tsa();
		}

		// If the import was temporary, reset the semaphore to its previous state.
		// See "6.4.5. Importing Semaphore Payloads" in Vulkan 1.1 spec.
		if(ext == tempExternal)
		{
			tempExternal = ext->previous;
			deallocateExternal(ext);
		}
	}
	else
	{
		internal.wait();
	}
}

void Semaphore::signal()
{
	ASSERT(semaphoreType == VK_SEMAPHORE_TYPE_BINARY);
	marl::lock lock(mutex);
	External *ext = tempExternal ? tempExternal : external;
	if(ext)
	{
		// Assumes that signalling an external semaphore is non-blocking,
		// so it can be performed directly either from a fiber or thread.
		ext->signal();
	}
	else
	{
		internal.signal();
	}
}

Semaphore::Semaphore(const VkSemaphoreCreateInfo *pCreateInfo, void *mem, const VkAllocationCallbacks *pAllocator)
    : allocator(pAllocator)
{
	SemaphoreCreateInfo info(pCreateInfo);
	exportableHandleTypes = info.exportHandleTypes;
	semaphoreType = info.semaphoreType;

	if(semaphoreType == VK_SEMAPHORE_TYPE_TIMELINE)
	{
		timeline = new(mem) TimelineSemaphore(info.initialPayload);
	}
}

void Semaphore::destroy(const VkAllocationCallbacks *pAllocator)
{
	marl::lock lock(mutex);
	while(tempExternal)
	{
		External *ext = tempExternal;
		tempExternal = ext->previous;
		deallocateExternal(ext);
	}
	if(external)
	{
		deallocateExternal(external);
		external = nullptr;
	}

	if(timeline != nullptr)
	{
		vk::deallocate(timeline, pAllocator);
	}
}

size_t Semaphore::ComputeRequiredAllocationSize(const VkSemaphoreCreateInfo *pCreateInfo)
{
	size_t requiredMem = 0;

	for(const auto *nextInfo = reinterpret_cast<const VkBaseInStructure *>(pCreateInfo->pNext);
	    nextInfo != nullptr; nextInfo = nextInfo->pNext)
	{
		switch(nextInfo->sType)
		{
			case VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO:
				// Semaphore::External instance is created and destroyed on demand, so don't
				// request memory for it.
				break;
			case VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO:
			{
				const auto *tlsInfo = reinterpret_cast<const VkSemaphoreTypeCreateInfo *>(nextInfo);
				if(tlsInfo->semaphoreType == VK_SEMAPHORE_TYPE_TIMELINE)
				{
					requiredMem += sizeof(TimelineSemaphore);
				}
			}
			break;
			default:
				WARN("nextInfo->sType = %s", vk::Stringify(nextInfo->sType).c_str());
				break;
		}
	}
	return requiredMem;
}

template<class EXTERNAL>
Semaphore::External *Semaphore::allocateExternal()
{
	auto *ext = reinterpret_cast<Semaphore::External *>(
	    vk::allocate(sizeof(EXTERNAL), alignof(EXTERNAL), allocator));
	new(ext) EXTERNAL();
	return ext;
}

void Semaphore::deallocateExternal(Semaphore::External *ext)
{
	ext->~External();
	vk::deallocate(ext, allocator);
}

template<typename ALLOC_FUNC, typename IMPORT_FUNC>
VkResult Semaphore::importPayload(bool temporaryImport,
                                  ALLOC_FUNC alloc_func,
                                  IMPORT_FUNC import_func)
{
	if(semaphoreType == VK_SEMAPHORE_TYPE_TIMELINE)
	{
		UNSUPPORTED("timelineSemaphore->importPayload()");
	}
	marl::lock lock(mutex);

	// Create new External instance if needed.
	External *ext = external;
	if(temporaryImport || !ext)
	{
		ext = alloc_func();
	}
	VkResult result = import_func(ext);
	if(result != VK_SUCCESS)
	{
		if(temporaryImport || !external)
		{
			deallocateExternal(ext);
		}
		return result;
	}

	if(temporaryImport)
	{
		ext->previous = tempExternal;
		tempExternal = ext;
	}
	else if(!external)
	{
		external = ext;
	}
	return VK_SUCCESS;
}

template<typename ALLOC_FUNC, typename EXPORT_FUNC>
VkResult Semaphore::exportPayload(ALLOC_FUNC alloc_func, EXPORT_FUNC export_func)
{
	if(semaphoreType == VK_SEMAPHORE_TYPE_TIMELINE)
	{
		UNSUPPORTED("timelineSemaphore->exportPayload()");
	}
	marl::lock lock(mutex);
	// Sanity check, do not try to export a semaphore that has a temporary import.
	if(tempExternal != nullptr)
	{
		TRACE("Cannot export semaphore with a temporary import!");
		return VK_ERROR_INVALID_EXTERNAL_HANDLE;
	}
	// Allocate |external| if it doesn't exist yet.
	if(!external)
	{
		External *ext = alloc_func();
		VkResult result = ext->init(internal.isSignalled());
		if(result != VK_SUCCESS)
		{
			deallocateExternal(ext);
			return result;
		}
		external = ext;
	}
	return export_func(external);
}

#if SWIFTSHADER_EXTERNAL_SEMAPHORE_OPAQUE_FD
VkResult Semaphore::importFd(int fd, bool temporaryImport)
{
	return importPayload(
	    temporaryImport,
	    [this]() {
		    return allocateExternal<OpaqueFdExternalSemaphore>();
	    },
	    [fd](External *ext) {
		    return ext->importOpaqueFd(fd);
	    });
}

VkResult Semaphore::exportFd(int *pFd)
{
	if((exportableHandleTypes & VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT) == 0)
	{
		TRACE("Cannot export semaphore as opaque FD (exportableHandleType = 0x%X, want 0x%X)",
		      exportableHandleTypes,
		      VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT);

		return VK_ERROR_INVALID_EXTERNAL_HANDLE;
	}

	return exportPayload([this]() { return allocateExternal<OpaqueFdExternalSemaphore>(); },
	                     [pFd](External *ext) {
		                     return ext->exportOpaqueFd(pFd);
	                     });
}
#endif  // SWIFTSHADER_EXTERNAL_SEMAPHORE_OPAQUE_FD

#if VK_USE_PLATFORM_FUCHSIA
VkResult Semaphore::importHandle(zx_handle_t handle, bool temporaryImport)
{
	return importPayload(
	    temporaryImport,
	    [this]() {
		    return allocateExternal<ZirconEventExternalSemaphore>();
	    },
	    [handle](External *ext) {
		    return ext->importHandle(handle);
	    });
}

VkResult Semaphore::exportHandle(zx_handle_t *pHandle)
{
	if((exportableHandleTypes & VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_TEMP_ZIRCON_EVENT_BIT_FUCHSIA) == 0)
	{
		TRACE("Cannot export semaphore as Zircon handle (exportableHandleType = 0x%X, want 0x%X)",
		      exportableHandleTypes,
		      VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_TEMP_ZIRCON_EVENT_BIT_FUCHSIA);

		return VK_ERROR_INVALID_EXTERNAL_HANDLE;
	}

	return exportPayload([this]() { return allocateExternal<ZirconEventExternalSemaphore>(); },
	                     [pHandle](External *ext) {
		                     return ext->exportHandle(pHandle);
	                     });
}
#endif  // VK_USE_PLATFORM_FUCHSIA

void Semaphore::timelineSignal(uint64_t value)
{
	ASSERT(semaphoreType == VK_SEMAPHORE_TYPE_TIMELINE);
	marl::lock lock(mutex);

	timeline->signal(value);
}

void Semaphore::timelineWait(uint64_t value)
{
	ASSERT(semaphoreType == VK_SEMAPHORE_TYPE_TIMELINE);
	timeline->wait(value);
}

template<typename CLOCK, typename DURATION>
VkResult Semaphore::timelineWait(uint64_t value, const std::chrono::time_point<CLOCK, DURATION> end_ns)
{
	ASSERT(semaphoreType == VK_SEMAPHORE_TYPE_TIMELINE);
	if(!timeline->waitUntil(value, end_ns))
	{
		return VK_TIMEOUT;
	}

	return VK_SUCCESS;
}

uint64_t Semaphore::getPayloadValue()
{
	ASSERT(semaphoreType == VK_SEMAPHORE_TYPE_TIMELINE);
	return timeline->getPayloadValue();
}

void Semaphore::addSharedDep(TimelineSemaphore other)
{
	ASSERT(semaphoreType == VK_SEMAPHORE_TYPE_TIMELINE);
	timeline->addSharedDep(other);
}

VkSemaphoreType Semaphore::getType()
{
	return semaphoreType;
}

int Semaphore::getTimelineId()
{
	ASSERT(semaphoreType == VK_SEMAPHORE_TYPE_TIMELINE);
	return timeline->getId();
}

VkResult WaitForSemaphores(const VkSemaphoreWaitInfo *pWaitInfo, uint64_t timeout)
{
	// Wait a number of nanoseconds equal to timeout for each timeline semaphore to reach the specified condition
	using time_point = std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>;
	const time_point start = now();
	const uint64_t max_timeout = (LLONG_MAX - start.time_since_epoch().count());
	bool infiniteTimeout = (timeout > max_timeout);
	const time_point end_ns = start + std::chrono::nanoseconds(std::min(max_timeout, timeout));

	if(pWaitInfo->flags & VK_SEMAPHORE_WAIT_ANY_BIT)
	{
		TimelineSemaphore any = TimelineSemaphore();

		for(uint32_t i = 0; i < pWaitInfo->semaphoreCount; i++)
		{
			Semaphore *semaphore = Cast(pWaitInfo->pSemaphores[i]);
			uint64_t waitValue = pWaitInfo->pValues[i];

			if(semaphore->getPayloadValue() == waitValue)
			{
				return VK_SUCCESS;
			}

			semaphore->addSharedDep(any);
			any.addToWaitMap(semaphore->getTimelineId(), waitValue);
		}

		if(infiniteTimeout)
		{
			any.wait(1ull);
			return VK_SUCCESS;
		}
		else
		{
			uint64_t tempValue = 1;
			if(any.waitUntil(tempValue, end_ns))
			{
				return VK_SUCCESS;
			}
		}

		return VK_TIMEOUT;
	}
	else
	{
		for(uint32_t i = 0; i < pWaitInfo->semaphoreCount; i++)
		{
			Semaphore *semaphore = Cast(pWaitInfo->pSemaphores[i]);
			uint64_t value = pWaitInfo->pValues[i];
			if(infiniteTimeout)
			{
				semaphore->timelineWait(value);
			}
			else if(semaphore->timelineWait(pWaitInfo->pValues[i], end_ns) != VK_SUCCESS)
			{
				return VK_TIMEOUT;
			}
		}
		return VK_SUCCESS;
	}
}

}  // namespace vk
