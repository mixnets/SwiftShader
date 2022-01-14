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

#ifndef VULKAN_GOOGLE_PIPELINE_SUBPIXEL_PRECISION_H
#define VULKAN_GOOGLE_PIPELINE_SUBPIXEL_PRECISION_H

#include "vulkan_core.h"

// THIS FILE SHOULD BE DELETED IF VK_GOOGLE_pipeline_subpixel_precision IS EVER ADDED TO THE VULKAN HEADERS
#ifdef VK_GOOGLE_pipeline_subpixel_precision
#	error "VK_GOOGLE_pipeline_subpixel_precision is already defined in the Vulkan headers, you can delete this file"
#endif

static constexpr VkStructureType VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_SUBPIXEL_PRECISION_STATE_CREATE_INFO_GOOGLE = static_cast<VkStructureType>(1000264001);

#define VK_GOOGLE_pipeline_subpixel_precision 1
#define VK_GOOGLE_SUBPIXEL_PRECISION_SPEC_VERSION 1
#define VK_GOOGLE_SUBPIXEL_PRECISION_EXTENSION_NAME "VK_GOOGLE_pipeline_subpixel_precision"

typedef struct VkPipelineRasterizationSubpixelPrecisionStateCreateInfoGOOGLE
{
	VkStructureType sType;
	const void *pNext;
	uint32_t subPixelPrecisionBits;
} VkPipelineRasterizationSubpixelPrecisionStateCreateInfoGOOGLE;

#endif  // VULKAN_GOOGLE_PIPELINE_SUBPIXEL_PRECISION_H
