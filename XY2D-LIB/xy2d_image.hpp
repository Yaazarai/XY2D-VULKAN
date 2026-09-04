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
			
			VkDescriptorImageInfo GetDescriptorInfo() {
				return { imageSampler, imageView, (VkImageLayout) imageLayout };
			}
			
			inline static VkWriteDescriptorSet GetWriteDescriptor(uint32_t binding, uint32_t descriptorCount, const VkDescriptorImageInfo* imageInfo) {
				return { .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, .pImageInfo = imageInfo, .dstSet = 0, .dstBinding = binding, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = descriptorCount };
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
					VkImageCreateInfo imgCreateInfo = xy2d_wrappers::defaultImageCreateInfo;
					imgCreateInfo.extent.width = static_cast<uint32_t>(width);
					imgCreateInfo.extent.height = static_cast<uint32_t>(height);
					imgCreateInfo.initialLayout = static_cast<VkImageLayout>(imageLayout);
					imgCreateInfo.format = rgbaFormat;
					VkResult result = vmaCreateImage(vkdevice.memoryAllocator, &imgCreateInfo, &xy2d_wrappers::defaultImageMemoryCreateInfo, &imageSource, &imageMemory, VK_NULL_HANDLE);
					if (result != VK_SUCCESS) return result;
				}
				
				VkSamplerCreateInfo imageSamplerInfo = xy2d_wrappers::defaultSamplerCreateInfo;
				imageSamplerInfo.addressModeU = imageSamplerInfo.addressModeV = imageSamplerInfo.addressModeW = addressMode;
				imageSamplerInfo.mipmapMode = (lerpFilter)? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST;
				VkResult result = vkCreateSampler(vkdevice.logicalDevice, &imageSamplerInfo, VK_NULL_HANDLE, &imageSampler);
				if (result != VK_SUCCESS) return result;
				
				VkImageViewCreateInfo imageViewCreateInfo = xy2d_wrappers::defaultImageViewCreateInfo;
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