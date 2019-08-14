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

#ifndef VK_SEMAPHORE_IMPL_WIN32_H_
#define VK_SEMAPHORE_IMPL_WIN32_H_

#include "VkDebug.hpp"

// An implementation of vk::Semaphore for Fuchsia

#include <zircon/syscalls.h>

namespace vk
{

class Semaphore::Impl {
public:
	Impl(const VkSemaphoreCreateInfo* pCreateInfo)
	{
		// TODO: external semaphore support!
		if (pCreateInfo->pNext)
		{
			UNIMPLEMENTED("pCreateInfo->pNext");
		}

		zx_status_t status = zx_event_create(0, &handle);
		if (status != ZX_OK)
		{
			ABORT("zx_event_create() returned %d", status);
		}
	}

	~Impl()
	{
		zx_handle_close(handle);
	}

	zx_handle_t exportHandle() const
	{
		zx_handle_t new_handle = ZX_HANDLE_INVALID;
		zx_status_t status = zx_handle_duplicate(handle, ZX_RIGHT_SAME_RIGHTS, &new_handle);
		if (status != ZX_OK)
		{
			ABORT("zx_handle_duplicate() returned %d", status);
		}
		return new_handle;
	}

	void importHandle(zx_handle_t new_handle)
	{
		zx_handle_close(handle);
		handle = new_handle;
	}

	void wait()
	{
		zx_signals_t observed = 0;
		zx_status_t status = zx_object_wait_one(
				handle, ZX_EVENT_SIGNALED, ZX_TIME_INFINITE, &observed);
		if (status != ZX_OK)
		{
			ABORT("zx_object_wait_one() returned %d", status);
		}
		if (observed != ZX_EVENT_SIGNALED)
		{
			ABORT("zx_object_wait_one() returned observed %x (%x expected)", observed, ZX_EVENT_SIGNALED);
		}
		// Need to unsignal the event now, as required by the Vulkan spec.
		status = zx_object_signal(handle, ZX_EVENT_SIGNALED, 0);
		if (status != ZX_OK)
		{
			ABORT("zx_object_signal() returned %d", status);
		}
	}

	void wait(const VkPipelineStageFlags& flags)
	{
		wait();
	}

	void signal()
	{
		zx_status_t status = zx_object_signal(handle, 0, ZX_EVENT_SIGNALED);
		if (status != ZX_OK)
		{
			ABORT("zx_object_signal() returned %d", status);
		}
	}
	
private:
	zx_handle_t handle = ZX_HANDLE_INVALID;
};

zx_handle_t Semaphore::exportHandle() const
{
	return impl->exportHandle();
}

VkResult Semaphore::importHandle(zx_handle_t handle)
{
	if (handle == ZX_HANDLE_INVALID)
	{
		return VK_ERROR_INVALID_EXTERNAL_HANDLE;
	}
	impl->importHandle(handle);
	return VK_SUCCESS;
}

}  // namespace vk

#endif  // VK_SEMAPHORE_IMPL_WIN32_H_
