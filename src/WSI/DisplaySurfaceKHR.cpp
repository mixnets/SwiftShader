// Copyright 2020 The SwiftShader Authors. All Rights Reserved.
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

#include "DisplaySurfaceKHR.hpp"

#include "Vulkan/VkDeviceMemory.hpp"
#include "Vulkan/VkImage.hpp"

#include <string.h>
#include <sys/mman.h>
#include <xf86drm.h>

namespace vk {

DisplaySurfaceKHR::DisplaySurfaceKHR(const VkDisplaySurfaceCreateInfoKHR *pCreateInfo, void *mem)
{
	fd = open("/dev/dri/card0", O_RDWR);
	drmModeRes *res = drmModeGetResources(fd);
	connector_id = res->connectors[0];
	drmModeFreeResources(res);
	drmModeConnector *connector = drmModeGetConnector(fd, connector_id);
	encoder_id = connector->encoder_id;
	memcpy(&mode_info, &connector->modes[0], sizeof(drmModeModeInfo));
	drmModeFreeConnector(connector);
	drmModeEncoder *encoder = drmModeGetEncoder(fd, encoder_id);
	crtc_id = encoder->crtc_id;
	drmModeFreeEncoder(encoder);

	crtc = drmModeGetCrtc(fd, crtc_id);

	struct drm_mode_create_dumb creq;
	memset(&creq, 0, sizeof(struct drm_mode_create_dumb));
	creq.width = mode_info.hdisplay;
	creq.height = mode_info.vdisplay;
	creq.bpp = 32;
	drmIoctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &creq);

	handle = creq.handle;
	width = creq.width;
	height = creq.height;
	pitch = creq.pitch;
	size = creq.size;

	drmModeAddFB(fd, width, height, 24, 32, pitch, handle, &fb_id);

	struct drm_mode_map_dumb mreq;
	memset(&mreq, 0, sizeof(struct drm_mode_map_dumb));
	mreq.handle = handle;
	drmIoctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &mreq);

	fb_buffer = static_cast<uint8_t *>(mmap(NULL, size, PROT_WRITE, MAP_SHARED, fd, mreq.offset));
}

void DisplaySurfaceKHR::destroySurface(const VkAllocationCallbacks *pAllocator)
{
	munmap(fb_buffer, size);

	drmModeRmFB(fd, fb_id);

	struct drm_mode_destroy_dumb dreq;
	memset(&dreq, 0, sizeof(struct drm_mode_destroy_dumb));
	dreq.handle = handle;
	drmIoctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq);

	drmModeSetCrtc(fd, crtc->crtc_id, crtc->buffer_id, crtc->x, crtc->y, &connector_id, 1, &crtc->mode);
	drmModeFreeCrtc(crtc);

	close(fd);
}

size_t DisplaySurfaceKHR::ComputeRequiredAllocationSize(const VkDisplaySurfaceCreateInfoKHR *pCreateInfo)
{
	return 0;
}

void DisplaySurfaceKHR::getSurfaceCapabilities(VkSurfaceCapabilitiesKHR *pSurfaceCapabilities) const
{
	SurfaceKHR::getSurfaceCapabilities(pSurfaceCapabilities);

	VkExtent2D extent = { width, height };

	pSurfaceCapabilities->currentExtent = extent;
	pSurfaceCapabilities->minImageExtent = extent;
	pSurfaceCapabilities->maxImageExtent = extent;
}

void DisplaySurfaceKHR::attachImage(PresentImage *image)
{
}

void DisplaySurfaceKHR::detachImage(PresentImage *image)
{
}

VkResult DisplaySurfaceKHR::present(PresentImage *image)
{
	image->getImage()->copyTo(fb_buffer, pitch);
	drmModeSetCrtc(fd, crtc_id, fb_id, 0, 0, &connector_id, 1, &mode_info);

	return VK_SUCCESS;
}

}  // namespace vk
