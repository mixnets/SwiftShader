// Copyright 2021 The SwiftShader Authors. All Rights Reserved.
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

#include "vulkan_core.h"

// THIS FILE SHOULD BE DELETED IF VK_GOOGLE_import_io_surface IS EVER ADDED TO THE VULKAN HEADERS
#ifdef VK_GOOGLE_import_io_surface
#	error "VK_GOOGLE_import_io_surface is already defined in the Vulkan headers, you can delete this file"
#endif

static constexpr VkStructureType VK_STRUCTURE_TYPE_IMPORT_IO_SURFACE_GOOGLE = static_cast<VkStructureType>(1000178099);

#define VK_GOOGLE_import_io_surface 1
#define VK_GOOGLE_IMPORT_IO_SURFACE_SPEC_VERSION 1
#define VK_GOOGLE_IMPORT_IO_SURFACE_EXTENSION_NAME "VK_GOOGLE_import_io_surface"

typedef struct VkImportIOSurfaceGOOGLE
{
	VkStructureType sType;
	const void *pNext;
	void *pIOSurface;
	uint32_t plane;
} VkImportIOSurfaceGOOGLE;
