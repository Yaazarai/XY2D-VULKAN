#pragma once
#ifndef __XY2D_IMAGE
#define __XY2D_IMAGE
	#include "./xy2d_engine.hpp"

	namespace XY2D_NAMESPACE {
		enum class XY2D_IMAGETYPE {
			SWAPCHAIN,
			ATTACHEMENT
		};
		
		enum class XY2D_IMAGELAYOUT : uint32_t {
			UNINITIALIZED = VK_IMAGE_LAYOUT_UNDEFINED,
			GENERAL = VK_IMAGE_LAYOUT_GENERAL,
			PRESENT_SRCKHR = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		};
		
		class xy2d_image : public xy2d_disposable {
		public:
			xy2d_device& vkdevice;
			VmaAllocation imageMemory = VK_NULL_HANDLE;
			VkImage imageSource = VK_NULL_HANDLE;
			VkImageView imageView = VK_NULL_HANDLE;
			VkSampler imageSampler = VK_NULL_HANDLE;
			
			uint32_t width, height;
			VkFormat rgbaFormat;
			VkColorComponentFlags colorWriteMask;
			VkSamplerAddressMode addressMode;
			VkBool32 lerpFilter;
			XY2D_IMAGETYPE sourceType = XY2D_IMAGETYPE::ATTACHEMENT;
			XY2D_IMAGELAYOUT imageLayout = XY2D_IMAGELAYOUT::UNINITIALIZED;
			VkResult initialized = VK_ERROR_INITIALIZATION_FAILED;
			
			xy2d_image operator=(const xy2d_image&) = delete;
			xy2d_image(const xy2d_image&) = delete;
			~xy2d_image() { this->Dispose(); }
			
			xy2d_image(xy2d_device& vkdevice, const XY2D_IMAGETYPE sourceType, uint32_t width, uint32_t height, VkFormat rgbaFormat = VK_FORMAT_B8G8R8A8_UNORM, VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, VkBool32 lerpFilter = false, VkImage imageSource = VK_NULL_HANDLE, VkImageView imageView = VK_NULL_HANDLE, VkSampler imageSampler = VK_NULL_HANDLE)
			: vkdevice(vkdevice), sourceType(sourceType), width(width), height(height), rgbaFormat(rgbaFormat), colorWriteMask(xy2d_wrappers::GetColorComponentFlags(rgbaFormat)), addressMode(addressMode), lerpFilter(lerpFilter), imageLayout(XY2D_IMAGELAYOUT::UNINITIALIZED), imageSource(imageSource), imageView(imageView), imageSampler(imageSampler) {
				onDispose.hook(xy2d_callback<>([this]() {
					if (this->sourceType != XY2D_IMAGETYPE::SWAPCHAIN) {
						if (this->imageSampler != VK_NULL_HANDLE) vkDestroySampler(this->vkdevice.logicalDevice, this->imageSampler, VK_NULL_HANDLE);
						if (this->imageView != VK_NULL_HANDLE) vkDestroyImageView(this->vkdevice.logicalDevice, this->imageView, VK_NULL_HANDLE);
						if (this->imageSource != VK_NULL_HANDLE) vmaDestroyImage(this->vkdevice.memoryAllocator, this->imageSource, imageMemory);
					} else {
						if (this->imageView != VK_NULL_HANDLE) vkDestroyImageView(this->vkdevice.logicalDevice, this->imageView, VK_NULL_HANDLE);
					}
				}));
				initialized = Initialize();
			}
			
			VkResult CreateImage(XY2D_IMAGETYPE sourceType, uint32_t width, uint32_t height, VkFormat rgbaFormat = VK_FORMAT_R16G16B16A16_UNORM, VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, VkBool32 lerpFilter = false) {
				this->sourceType = sourceType;
				this->width = width;
				this->height = height;
				this->rgbaFormat = rgbaFormat;
				this->colorWriteMask = xy2d_wrappers::GetColorComponentFlags(rgbaFormat);
				this->addressMode = addressMode;
				this->lerpFilter = lerpFilter;
				
				if (sourceType != XY2D_IMAGETYPE::SWAPCHAIN) {
					VkImageCreateInfo imageCreateInfo = { .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, .extent.depth = 1U, .mipLevels = 1U, .arrayLayers = 1U, .imageType = VK_IMAGE_TYPE_2D, .tiling = VK_IMAGE_TILING_OPTIMAL, .samples = VK_SAMPLE_COUNT_1_BIT, .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT };
					imageCreateInfo.extent.width = static_cast<uint32_t>(width);
					imageCreateInfo.extent.height = static_cast<uint32_t>(height);
					imageCreateInfo.initialLayout = static_cast<VkImageLayout>(imageLayout);
					imageCreateInfo.format = rgbaFormat;
					VmaAllocationCreateInfo imageMemoryCreateInfo = { .priority = 1.0f, .usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST, .flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT };
					VkResult result = vmaCreateImage(vkdevice.memoryAllocator, &imageCreateInfo, &imageMemoryCreateInfo, &imageSource, &imageMemory, VK_NULL_HANDLE);
					if (result != VK_SUCCESS) return result;
				}
				
				VkSamplerCreateInfo imageSamplerInfo = { .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO, .magFilter = VK_FILTER_NEAREST, .minFilter = VK_FILTER_NEAREST, .anisotropyEnable = VK_FALSE, .compareEnable = VK_FALSE, .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK, .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, .unnormalizedCoordinates = VK_FALSE, .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR, .minLod = 0.0f, .maxLod = VK_LOD_CLAMP_NONE };
				imageSamplerInfo.addressModeU = imageSamplerInfo.addressModeV = imageSamplerInfo.addressModeW = addressMode;
				imageSamplerInfo.mipmapMode = (lerpFilter)? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST;
				vkCreateSampler(vkdevice.logicalDevice, &imageSamplerInfo, VK_NULL_HANDLE, &imageSampler);
				
				VkImageViewCreateInfo imageViewCreateInfo = { .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, .viewType = VK_IMAGE_VIEW_TYPE_2D, .components = { VK_COMPONENT_SWIZZLE_IDENTITY }, .subresourceRange = { .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1, .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT } };
				imageViewCreateInfo.image = imageSource;
				imageViewCreateInfo.format = rgbaFormat;
				return vkCreateImageView(vkdevice.logicalDevice, &imageViewCreateInfo, VK_NULL_HANDLE, &imageView);
			}
			
			VkResult Initialize() {
				CreateImage(sourceType, width, height, rgbaFormat, addressMode);
				return (imageSource == VK_NULL_HANDLE)? VK_ERROR_INITIALIZATION_FAILED : VK_SUCCESS;
			}
		};
	}
#endif