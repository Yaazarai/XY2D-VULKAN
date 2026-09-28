#pragma once
#ifndef __XYVK_GPUALLOC
#define __XYVK_GPUALLOC
	#include "./xy2d_engine.hpp"
	
	enum class XY2D_GPUALLOC_LAYOUT {
		UNINITIALIZED = VK_IMAGE_LAYOUT_UNDEFINED,
		GENERAL = VK_IMAGE_LAYOUT_GENERAL,
		PRESENT_SRCKHR = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
	};
	
	enum class XY2D_GPUALLOC_TYPE {
		VERTEX, UNIFORM, STAGING, SWAPCHAIN, ATTACHEMENT, STORAGE
	};
	
	struct xy2d_gpualloc {
		xy2d_device* vkdevice = VK_NULL_HANDLE;
		VmaAllocation memory = VK_NULL_HANDLE;
		VkBuffer buffer = VK_NULL_HANDLE;
		VkImage image = VK_NULL_HANDLE;
		VkImageView view = VK_NULL_HANDLE;
		VkSampler sampler = VK_NULL_HANDLE;
		
		VmaAllocationInfo description;
		VkExtent2D length;
		XY2D_GPUALLOC_TYPE type;
		XY2D_GPUALLOC_LAYOUT layout = XY2D_GPUALLOC_LAYOUT::UNINITIALIZED;
		
		VkFormat rgbaFormat = VK_FORMAT_B8G8R8A8_UNORM;
		VkColorComponentFlags rgbaWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		VkBool32 lerpFilter = VK_FALSE;
		
		VkResult initialized = VK_ERROR_INITIALIZATION_FAILED;
	};
	
	VkColorComponentFlags xy2d_gpualloc_get_colorflags(VkFormat format) {
		VkColorComponentFlags flags = 0;
		if (vkuFormatHasRed(format)) flags |= VK_COLOR_COMPONENT_R_BIT;
		if (vkuFormatHasGreen(format)) flags |= VK_COLOR_COMPONENT_G_BIT;
		if (vkuFormatHasBlue(format)) flags |= VK_COLOR_COMPONENT_B_BIT;
		if (vkuFormatHasAlpha(format)) flags |= VK_COLOR_COMPONENT_A_BIT;
		return flags;
	}
	
	void xy2d_gpualloc_destroy(xy2d_gpualloc& gpualloc) {
		if (gpualloc.type == XY2D_GPUALLOC_TYPE::VERTEX || gpualloc.type == XY2D_GPUALLOC_TYPE::UNIFORM || gpualloc.type == XY2D_GPUALLOC_TYPE::STAGING) {
			if (gpualloc.memory != VK_NULL_HANDLE) vmaDestroyBuffer(gpualloc.vkdevice->allocator, gpualloc.buffer, gpualloc.memory);
		} else {
			if (gpualloc.type != XY2D_GPUALLOC_TYPE::SWAPCHAIN && gpualloc.image != VK_NULL_HANDLE) vmaDestroyImage(gpualloc.vkdevice->allocator, gpualloc.image, gpualloc.memory);
			if (gpualloc.sampler != VK_NULL_HANDLE) vkDestroySampler(gpualloc.vkdevice->logical, gpualloc.sampler, VK_NULL_HANDLE);
			if (gpualloc.view != VK_NULL_HANDLE) vkDestroyImageView(gpualloc.vkdevice->logical, gpualloc.view, VK_NULL_HANDLE);
		}
	}
	
	VkResult xy2d_gpualloc_allocate(xy2d_gpualloc& gpualloc) {
		if (gpualloc.type == XY2D_GPUALLOC_TYPE::VERTEX || gpualloc.type == XY2D_GPUALLOC_TYPE::UNIFORM || gpualloc.type == XY2D_GPUALLOC_TYPE::STAGING) {
			VmaAllocationCreateFlags flags = 0x00000000U;
			VkBufferUsageFlags usage = 0x00000000U;
			
			switch (gpualloc.type) {
				case XY2D_GPUALLOC_TYPE::VERTEX: usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT; break;
				case XY2D_GPUALLOC_TYPE::UNIFORM: usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT; break;
				case XY2D_GPUALLOC_TYPE::STAGING:
					usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
					flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
				break;
			}
			
			VkBufferCreateInfo bufCreateInfo = { .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, .size = gpualloc.length.width, .usage = usage };
			VmaAllocationCreateInfo allocCreateInfo { .usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST, .flags = flags };
			gpualloc.initialized = VK_SUCCESS;
			return vmaCreateBuffer(gpualloc.vkdevice->allocator, &bufCreateInfo, &allocCreateInfo, &gpualloc.buffer, &gpualloc.memory, &gpualloc.description);
		} else {
			if (gpualloc.type != XY2D_GPUALLOC_TYPE::SWAPCHAIN) {
				VkImageCreateInfo imageCreateInfo = { .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, .extent.depth = 1U, .mipLevels = 1U, .arrayLayers = 1U, .imageType = VK_IMAGE_TYPE_2D, .tiling = VK_IMAGE_TILING_OPTIMAL, .samples = VK_SAMPLE_COUNT_1_BIT, .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT };
				imageCreateInfo.extent.width = gpualloc.length.width;
				imageCreateInfo.extent.height = gpualloc.length.height;
				imageCreateInfo.initialLayout = static_cast<VkImageLayout>(gpualloc.layout);
				imageCreateInfo.format = gpualloc.rgbaFormat;
				VmaAllocationCreateInfo imageMemoryCreateInfo = { .priority = 1.0f, .usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST, .flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT };
				VkResult result = vmaCreateImage(gpualloc.vkdevice->allocator, &imageCreateInfo, &imageMemoryCreateInfo, &gpualloc.image, &gpualloc.memory, VK_NULL_HANDLE);
				if (result != VK_SUCCESS) return result;
			}
			
			VkSamplerCreateInfo imageSamplerInfo = { .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO, .magFilter = VK_FILTER_NEAREST, .minFilter = VK_FILTER_NEAREST, .anisotropyEnable = VK_FALSE, .compareEnable = VK_FALSE, .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK, .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, .unnormalizedCoordinates = VK_FALSE, .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR, .minLod = 0.0f, .maxLod = VK_LOD_CLAMP_NONE };
			imageSamplerInfo.addressModeU = imageSamplerInfo.addressModeV = imageSamplerInfo.addressModeW = gpualloc.addressMode;
			imageSamplerInfo.mipmapMode = (gpualloc.lerpFilter)? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST;
			VkResult result = vkCreateSampler(gpualloc.vkdevice->logical, &imageSamplerInfo, VK_NULL_HANDLE, &gpualloc.sampler);
			if (result != VK_SUCCESS) return result;
			
			VkImageViewCreateInfo imageViewCreateInfo = { .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, .viewType = VK_IMAGE_VIEW_TYPE_2D, .components = { VK_COMPONENT_SWIZZLE_IDENTITY }, .subresourceRange = { .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1, .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT } };
			imageViewCreateInfo.image = gpualloc.image;
			imageViewCreateInfo.format = gpualloc.rgbaFormat;
			gpualloc.initialized = VK_SUCCESS;
			return vkCreateImageView(gpualloc.vkdevice->logical, &imageViewCreateInfo, VK_NULL_HANDLE, &gpualloc.view);
		}
	}
	
	xy2d_gpualloc xy2d_gpualloc_create(xy2d_device& vkdevice, XY2D_GPUALLOC_TYPE allocType, VkExtent2D length, VkFormat rgbaFormat = VK_FORMAT_R16G16B16A16_UNORM, VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, VkBool32 lerpFilter = false, VkImage imageSource = VK_NULL_HANDLE) {
		xy2d_gpualloc allocation = { .vkdevice = &vkdevice, .type = allocType, .length = length, .rgbaFormat = rgbaFormat, .addressMode = addressMode, .lerpFilter = lerpFilter, .image = imageSource };
		xy2d_gpualloc_allocate(allocation);
		return allocation;
	}
#endif