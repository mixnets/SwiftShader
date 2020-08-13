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

#include "VkDestroy.hpp"
#include "VkDevice.hpp"
#include "VkObject.hpp"
#include "VkStringify.hpp"

#include "System/Debug.hpp"

#include <android/hardware_buffer.h>

#include <errno.h>
#include <string.h>

class AHardwareBufferExternalMemory : public vk::DeviceMemory::ExternalBase
{
public:
	// Helper struct to parse the VkMemoryAllocateInfo.pNext chain and
	// extract relevant information related to the handle type supported
	// by this DeviceMemory::ExternalBase subclass.
	struct AllocateInfo
	{
		bool importAhb = false;
		bool exportAhb = false;
		struct AHardwareBuffer *ahb = nullptr;
		vk::Image *imageHandle = nullptr;
		vk::Buffer *bufferHandle = nullptr;

		AllocateInfo() = default;

		// Parse the VkMemoryAllocateInfo.pNext chain to initialize an AllocateInfo.
		AllocateInfo(const VkMemoryAllocateInfo *pAllocateInfo)
		{
			const auto *createInfo = reinterpret_cast<const VkBaseInStructure *>(pAllocateInfo->pNext);
			while(createInfo)
			{
				switch(createInfo->sType)
				{
					case VK_STRUCTURE_TYPE_IMPORT_ANDROID_HARDWARE_BUFFER_INFO_ANDROID:
					{
						const auto *importInfo = reinterpret_cast<const VkImportAndroidHardwareBufferInfoANDROID *>(createInfo);
						importAhb = true;
						ahb = importInfo->buffer;
					}
					break;
					case VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO:
					{
						const auto *exportInfo = reinterpret_cast<const VkExportMemoryAllocateInfo *>(createInfo);

						if(exportInfo->handleTypes != VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID)
						{
							UNSUPPORTED("VkExportMemoryAllocateInfo::handleTypes %d", int(exportInfo->handleTypes));
						}
						exportAhb = true;
					}
					break;
					case VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO:
					{
						// AHB requires dedicated allocation -- for images, the gralloc gets to decide the image layout,
						// not us.
						const auto *dedicatedAllocateInfo = reinterpret_cast<const VkMemoryDedicatedAllocateInfo *>(createInfo);
						imageHandle = vk::Cast(dedicatedAllocateInfo->image);
						bufferHandle = vk::Cast(dedicatedAllocateInfo->buffer);
					}
					break;

					default:
						WARN("VkMemoryAllocateInfo->pNext sType = %s", vk::Stringify(createInfo->sType).c_str());
				}
				createInfo = createInfo->pNext;
			}
		}
	};

	static const VkExternalMemoryHandleTypeFlagBits typeFlagBit = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;

	static bool supportsAllocateInfo(const VkMemoryAllocateInfo *pAllocateInfo)
	{
		AllocateInfo info(pAllocateInfo);
		return (info.importAhb || info.exportAhb) && (info.bufferHandle || info.imageHandle);
	}

	explicit AHardwareBufferExternalMemory(const VkMemoryAllocateInfo *pAllocateInfo)
	    : allocateInfo(pAllocateInfo)
	{
	}

	~AHardwareBufferExternalMemory()
	{
		if(ahb)
			AHardwareBuffer_release(ahb);
	}

	VkResult allocate(size_t size, void **pBuffer) override
	{
		if(allocateInfo.importAhb)
		{
			//ahb = allocateInfo.ahb;
			//AHardwareBuffer_acquire(ahb);
			// TODO: also allocate our internal shadow memory
			return VK_ERROR_INVALID_EXTERNAL_HANDLE;
		}
		else
		{
			ASSERT(allocateInfo.exportAhb);
			// TODO: create and import the AHB
			return VK_ERROR_OUT_OF_DEVICE_MEMORY;
		}

		return VK_ERROR_OUT_OF_DEVICE_MEMORY;
		/*
		void *addr = memfd.mapReadWrite(0, size);
		if(!addr)
		{
			return VK_ERROR_MEMORY_MAP_FAILED;
		}
		*pBuffer = addr;
		return VK_SUCCESS;
		 */
	}

	void deallocate(void *buffer, size_t size) override
	{
		// FIXME
	}

	VkExternalMemoryHandleTypeFlagBits getFlagBit() const override
	{
		return typeFlagBit;
	}

	VkResult exportAhb(struct AHardwareBuffer **pAhb) const override
	{
		// Each call to vkGetMemoryAndroidHardwareBufferANDROID *must* return an Android hardware buffer with a new reference
		// acquired in addition to the reference held by the VkDeviceMemory. To avoid leaking resources, the application *must*
		// release the reference by calling AHardwareBuffer_release when it is no longer needed.
		AHardwareBuffer_acquire(ahb);
		*pAhb = ahb;
		return VK_SUCCESS;
	}

	static uint32_t getAhbDescFormat(VkFormat format)
	{
		switch(format)
		{
			case VK_FORMAT_D16_UNORM:
				return AHARDWAREBUFFER_FORMAT_D16_UNORM;
			case VK_FORMAT_X8_D24_UNORM_PACK32:
				UNSUPPORTED("AHardwareBufferExternalMemory::VkFormat VK_FORMAT_X8_D24_UNORM_PACK32");
				return AHARDWAREBUFFER_FORMAT_D24_UNORM;
			case VK_FORMAT_D24_UNORM_S8_UINT:
				UNSUPPORTED("AHardwareBufferExternalMemory::VkFormat VK_FORMAT_D24_UNORM_S8_UINT");
				return AHARDWAREBUFFER_FORMAT_D24_UNORM_S8_UINT;
			case VK_FORMAT_D32_SFLOAT:
				return AHARDWAREBUFFER_FORMAT_D32_FLOAT;
			case VK_FORMAT_D32_SFLOAT_S8_UINT:
				return AHARDWAREBUFFER_FORMAT_D32_FLOAT_S8_UINT;
			case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
				return AHARDWAREBUFFER_FORMAT_R10G10B10A2_UNORM;
			case VK_FORMAT_R16G16B16A16_SFLOAT:
				return AHARDWAREBUFFER_FORMAT_R16G16B16A16_FLOAT;
			case VK_FORMAT_R5G6B5_UNORM_PACK16:
				return AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM;
			case VK_FORMAT_R8G8B8A8_UNORM:
				return AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM;
			//case VK_FORMAT_R8G8B8A8_UNORM:
			//  return AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM;
			case VK_FORMAT_R8G8B8_UNORM:
				return AHARDWAREBUFFER_FORMAT_R8G8B8_UNORM;
			case VK_FORMAT_S8_UINT:
				return AHARDWAREBUFFER_FORMAT_S8_UINT;
			case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:
				return AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420;
			default:
				UNSUPPORTED("AHardwareBufferExternalMemory::VkFormat %d", int(format));
				return 0;
		}
	}

	static VkFormat getVkFormat(uint32_t ahbFormat)
	{
		switch(ahbFormat)
		{
			case AHARDWAREBUFFER_FORMAT_BLOB:
				return VK_FORMAT_UNDEFINED;
			case AHARDWAREBUFFER_FORMAT_D16_UNORM:
				return VK_FORMAT_D16_UNORM;
			case AHARDWAREBUFFER_FORMAT_D24_UNORM:
				UNSUPPORTED("AHardwareBufferExternalMemory::AndroidHardwareBuffer_Format AHARDWAREBUFFER_FORMAT_D24_UNORM");
				return VK_FORMAT_X8_D24_UNORM_PACK32;
			case AHARDWAREBUFFER_FORMAT_D24_UNORM_S8_UINT:
				UNSUPPORTED("AHardwareBufferExternalMemory::AndroidHardwareBuffer_Format AHARDWAREBUFFER_FORMAT_D24_UNORM_S8_UINT");
				return VK_FORMAT_X8_D24_UNORM_PACK32;
			case AHARDWAREBUFFER_FORMAT_D32_FLOAT:
				return VK_FORMAT_D32_SFLOAT;
			case AHARDWAREBUFFER_FORMAT_D32_FLOAT_S8_UINT:
				return VK_FORMAT_D32_SFLOAT_S8_UINT;
			case AHARDWAREBUFFER_FORMAT_R10G10B10A2_UNORM:
				return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
			case AHARDWAREBUFFER_FORMAT_R16G16B16A16_FLOAT:
				return VK_FORMAT_R16G16B16A16_SFLOAT;
			case AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM:
				return VK_FORMAT_R5G6B5_UNORM_PACK16;
			case AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM:
				return VK_FORMAT_R8G8B8A8_UNORM;
			case AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM:
				return VK_FORMAT_R8G8B8A8_UNORM;
			case AHARDWAREBUFFER_FORMAT_R8G8B8_UNORM:
				return VK_FORMAT_R8G8B8_UNORM;
			case AHARDWAREBUFFER_FORMAT_S8_UINT:
				return VK_FORMAT_S8_UINT;
			case AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420:
				return VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM;
			default:
				UNSUPPORTED("AHardwareBufferExternalMemory::AHardwareBuffer_Format %d", int(ahbFormat));
				return VK_FORMAT_UNDEFINED;
		}
	}

	static VkFormatFeatureFlags getVkFormatFeatures(VkFormat format)
	{
		VkFormatFeatureFlags linearTilingFeatures = 0;
		VkFormatFeatureFlags optimalTilingFeatures = 0;
		VkFormatFeatureFlags bufferFeatures = 0;
		switch(format)
		{
			case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
			case VK_FORMAT_R16G16B16A16_SFLOAT:
			case VK_FORMAT_R5G6B5_UNORM_PACK16:
			case VK_FORMAT_R8G8B8A8_UNORM:
			case VK_FORMAT_D16_UNORM:
			case VK_FORMAT_D32_SFLOAT:
			case VK_FORMAT_D32_SFLOAT_S8_UINT:
				optimalTilingFeatures |=
				    VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;
				// [[fallthrough]]
			case VK_FORMAT_S8_UINT:
				optimalTilingFeatures |=
				    VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT |
				    VK_FORMAT_FEATURE_BLIT_SRC_BIT |
				    VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
				    VK_FORMAT_FEATURE_TRANSFER_DST_BIT;
				break;
			case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:
				optimalTilingFeatures |=
				    VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT |
				    VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT |
				    VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
				    VK_FORMAT_FEATURE_TRANSFER_DST_BIT |
				    VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT;
				break;
			default:
				break;
		}
		switch(format)
		{
			case VK_FORMAT_R16G16B16A16_SFLOAT:
			case VK_FORMAT_R8G8B8A8_UNORM:
				optimalTilingFeatures |=
				    VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT;
				bufferFeatures |=
				    VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT;
			default:
				break;
		}
		switch(format)
		{
			case VK_FORMAT_S8_UINT:
			case VK_FORMAT_D16_UNORM:
			case VK_FORMAT_D32_SFLOAT:
			case VK_FORMAT_D32_SFLOAT_S8_UINT:
				optimalTilingFeatures |=
				    VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
				break;
			case VK_FORMAT_R16G16B16A16_SFLOAT:
			case VK_FORMAT_R5G6B5_UNORM_PACK16:
			case VK_FORMAT_R8G8B8A8_UNORM:
				optimalTilingFeatures |=
				    VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT;
			case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
				optimalTilingFeatures |=
				    VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT |
				    VK_FORMAT_FEATURE_BLIT_DST_BIT;
				break;
			default:
				break;
		}
		switch(format)
		{
			case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
			case VK_FORMAT_R16G16B16A16_SFLOAT:
			case VK_FORMAT_R8G8B8A8_UNORM:
				bufferFeatures |=
				    VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT;
				break;
			default:
				break;
		}
		switch(format)
		{
			case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
			case VK_FORMAT_R16G16B16A16_SFLOAT:
			case VK_FORMAT_R8G8B8A8_UNORM:
				bufferFeatures |=
				    VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT;
			default:
				break;
		}
		if(optimalTilingFeatures)
		{
			linearTilingFeatures = VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
			                       VK_FORMAT_FEATURE_TRANSFER_DST_BIT;
		}
		optimalTilingFeatures |= VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT;

		return optimalTilingFeatures | linearTilingFeatures | bufferFeatures;
	}

	static VkResult getAhbFormatProperties(const AHardwareBuffer_Desc &ahbDesc, VkAndroidHardwareBufferFormatPropertiesANDROID *pFormat)
	{

		pFormat->sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID;
		pFormat->pNext = nullptr;

		pFormat->format = getVkFormat(ahbDesc.format);
		pFormat->externalFormat = ahbDesc.format;
		pFormat->formatFeatures = getVkFormatFeatures(pFormat->format);

		pFormat->samplerYcbcrConversionComponents = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
		pFormat->suggestedYcbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY;
		pFormat->suggestedYcbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_FULL;
		pFormat->suggestedXChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
		pFormat->suggestedYChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;

		return VK_SUCCESS;
	}

	static VkResult getAndroidHardwareBufferProperties(VkDevice &device, const struct AHardwareBuffer *buffer, VkAndroidHardwareBufferPropertiesANDROID *pProperties)
	{
		AHardwareBuffer_Desc ahbDesc;
		AHardwareBuffer_describe(buffer, &ahbDesc);

		getAhbFormatProperties(ahbDesc, (VkAndroidHardwareBufferFormatPropertiesANDROID *)pProperties->pNext);
                // TODO(b/141698760)
		//   It's not clear if setting memoryTypeBits is enough
		pProperties->memoryTypeBits = ahbDesc.format;

		if(ahbDesc.format == AHARDWAREBUFFER_FORMAT_BLOB)
		{
			pProperties->allocationSize = ahbDesc.width;
		}
		else
		{
			VkImageCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO;
			info.pNext = nullptr;
			info.flags = 0;
			info.imageType = VK_IMAGE_TYPE_2D;
			info.format = getVkFormat(ahbDesc.format);
			info.extent.width = ahbDesc.width;
			info.extent.height = ahbDesc.height;
			info.extent.depth = 1;
			info.mipLevels = 1;
			info.arrayLayers = 1;
			info.samples = VK_SAMPLE_COUNT_1_BIT;
			info.tiling = VK_IMAGE_TILING_OPTIMAL;
			info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

			VkImage Image;
			VkResult result = vk::Image::Create(vk::DEVICE_MEMORY, &info, &Image, vk::Cast(device));

			pProperties->allocationSize = vk::Cast(Image)->getMemoryRequirements().size;
			vk::destroy(Image, vk::DEVICE_MEMORY);
		}

		return VK_SUCCESS;
	}

private:
	struct AHardwareBuffer *ahb = nullptr;
	AllocateInfo allocateInfo;
};
