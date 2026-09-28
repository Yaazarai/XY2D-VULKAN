#pragma once
#ifndef __XY2D_RENDERER
#define __XY2D_RENDERER
	#include ".\xy2d_engine.hpp"
	
	struct xy2d_renderer {
		xy2d_device* vkdevice = VK_NULL_HANDLE;
		xy2d_invoker<xy2d_cmdbuffer&, xy2d_gpualloc&> renderEvent = {};
		
		std::vector<xy2d_gpualloc> swapChainImages = std::vector<xy2d_gpualloc>(XY2D_BUFFERED_IMAGES);
		VkSwapchainKHR swapChain = VK_NULL_HANDLE;
		uint32_t swapChainAcquiredIndex = 0U;
		
		std::vector<VkCommandBuffer> commandBuffers = std::vector<VkCommandBuffer>(XY2D_CMDBUFFER_COUNT);
		std::vector<glm::float64_t> frameTimeStamps;
		VkCommandPool commandPool = VK_NULL_HANDLE;
		VkQueryPool timestampQueryPool = VK_FALSE;
		
		std::vector<VkSemaphore> swapChainPresented = std::vector<VkSemaphore>(XY2D_BUFFERED_IMAGES);
		VkFence swapChainAcquired = VK_NULL_HANDLE;
		VkFence swapChainFinished = VK_NULL_HANDLE;
		
		VkSurfaceFormatKHR presentFormat = { VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
		VkSurfaceCapabilitiesKHR capabilities = {};
		VkResult initialized = VK_ERROR_INITIALIZATION_FAILED;
	};
	
	void xy2d_renderer_destroy(xy2d_renderer& renderer) {
		std::for_each(renderer.swapChainImages.begin(), renderer.swapChainImages.end(), [&](xy2d_gpualloc swapImage) { xy2d_gpualloc_destroy(swapImage); });
		std::for_each(renderer.swapChainPresented.begin(), renderer.swapChainPresented.end(), [&](VkSemaphore semaphore) { vkDestroySemaphore(renderer.vkdevice->logical, semaphore, VK_NULL_HANDLE); });
		if (renderer.swapChainAcquired != VK_NULL_HANDLE) vkDestroyFence(renderer.vkdevice->logical, renderer.swapChainAcquired, VK_NULL_HANDLE);
		if (renderer.swapChainFinished != VK_NULL_HANDLE) vkDestroyFence(renderer.vkdevice->logical, renderer.swapChainFinished, VK_NULL_HANDLE);
		if (renderer.timestampQueryPool != VK_NULL_HANDLE) vkDestroyQueryPool(renderer.vkdevice->logical, renderer.timestampQueryPool, VK_NULL_HANDLE);
		if (renderer.commandPool != VK_NULL_HANDLE) vkDestroyCommandPool(renderer.vkdevice->logical, renderer.commandPool, VK_NULL_HANDLE);
		if (renderer.swapChain != VK_NULL_HANDLE) vkDestroySwapchainKHR(renderer.vkdevice->logical, renderer.swapChain, VK_NULL_HANDLE);
	}
	
	VkResult xy2d_renderer_swapchain(xy2d_renderer& renderer) {
		vkDeviceWaitIdle(renderer.vkdevice->logical);
		
		std::vector<VkSurfaceFormatKHR> surfaceFormats;
		xy2d_enumerate(surfaceFormats, vkGetPhysicalDeviceSurfaceFormatsKHR, renderer.vkdevice->physical, renderer.vkdevice->surface);
		auto findFormat = std::find_if(surfaceFormats.begin(), surfaceFormats.end(), [&](const VkSurfaceFormatKHR& format) { return renderer.presentFormat.colorSpace == format.colorSpace && renderer.presentFormat.format == format.format; });
		if (findFormat == surfaceFormats.end())
			findFormat = std::find_if(surfaceFormats.begin(), surfaceFormats.end(), [&](const VkSurfaceFormatKHR& format) { return renderer.presentFormat.colorSpace == format.colorSpace; });
		renderer.presentFormat = *findFormat;
		
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(renderer.vkdevice->physical, renderer.vkdevice->surface, &renderer.capabilities);
		uint32_t width = std::clamp(xy2d_window_get_width(), renderer.capabilities.minImageExtent.width, renderer.capabilities.maxImageExtent.width);
		uint32_t height = std::clamp(xy2d_window_get_height(), renderer.capabilities.minImageExtent.height, renderer.capabilities.maxImageExtent.height);
		if (width <= 0 || height <= 0) return VK_ERROR_OUT_OF_DATE_KHR;
		
		VkExtent2D swapChainExtent = { width, height };
		VkSwapchainCreateInfoKHR swapChainCreateInfo = { .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR, .imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR, .imageArrayLayers = 1, .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE, .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, .presentMode = VK_PRESENT_MODE_FIFO_KHR, .clipped = VK_TRUE };
		swapChainCreateInfo.surface = renderer.vkdevice->surface;
		swapChainCreateInfo.minImageCount = XY2D_BUFFERED_IMAGES;
		swapChainCreateInfo.imageExtent = swapChainExtent;
		swapChainCreateInfo.imageFormat = renderer.presentFormat.format;
		swapChainCreateInfo.preTransform = renderer.capabilities.currentTransform;
		swapChainCreateInfo.imageColorSpace = renderer.presentFormat.colorSpace;
		swapChainCreateInfo.oldSwapchain = renderer.swapChain;
		VkResult result = vkCreateSwapchainKHR(renderer.vkdevice->logical, &swapChainCreateInfo, VK_NULL_HANDLE, &renderer.swapChain);
		if (result != VK_SUCCESS) return result;
		
		std::for_each(renderer.swapChainImages.begin(), renderer.swapChainImages.end(), [](xy2d_gpualloc& swapImage){ xy2d_gpualloc_destroy(swapImage); });
		vkDestroySwapchainKHR(renderer.vkdevice->logical, swapChainCreateInfo.oldSwapchain, VK_NULL_HANDLE);
		
		std::vector<VkImage> newSwapImages = std::vector<VkImage>(XY2D_BUFFERED_IMAGES);
		xy2d_enumerate(newSwapImages, vkGetSwapchainImagesKHR, renderer.vkdevice->logical, renderer.swapChain);
		for(uint32_t i = 0; i < renderer.swapChainImages.size(); i++)
			renderer.swapChainImages[i] = xy2d_gpualloc_create(*renderer.vkdevice, XY2D_GPUALLOC_TYPE::SWAPCHAIN, { swapChainExtent.width, swapChainExtent.height }, renderer.presentFormat.format, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, VK_FALSE, newSwapImages[i]);
		
		if (renderer.swapChainAcquired != VK_NULL_HANDLE) vkDestroyFence(renderer.vkdevice->logical, renderer.swapChainAcquired, VK_NULL_HANDLE);
		VkFenceCreateInfo unsignaledFenceCreateInfo = { .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = static_cast<VkFenceCreateFlags>(VK_FALSE) };
		vkCreateFence(renderer.vkdevice->logical, &unsignaledFenceCreateInfo, VK_NULL_HANDLE, &renderer.swapChainAcquired);
		
		if (renderer.swapChainFinished != VK_NULL_HANDLE) vkDestroyFence(renderer.vkdevice->logical, renderer.swapChainFinished, VK_NULL_HANDLE);
		VkFenceCreateInfo signaledFenceCreateInfo = { .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = static_cast<VkFenceCreateFlags>(VK_TRUE) };
		vkCreateFence(renderer.vkdevice->logical, &signaledFenceCreateInfo, VK_NULL_HANDLE, &renderer.swapChainFinished);
		return result;
	}
	
	VkResult xy2d_renderer_frame_present(xy2d_renderer& renderer) {
		VkResult result = vkAcquireNextImageKHR(renderer.vkdevice->logical, renderer.swapChain, UINT64_MAX, VK_NULL_HANDLE, renderer.swapChainAcquired, &renderer.swapChainAcquiredIndex);
		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
			return xy2d_renderer_swapchain(renderer);
		
		vkWaitForFences(renderer.vkdevice->logical, 1U, &renderer.swapChainAcquired, VK_TRUE, UINT64_MAX);
		vkResetFences(renderer.vkdevice->logical, 1U, &renderer.swapChainAcquired);
			vkResetCommandBuffer(renderer.commandBuffers[renderer.swapChainAcquiredIndex], 0U);
			
			VkCommandBufferBeginInfo commandBufferBeginInfo = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT };
			vkBeginCommandBuffer(renderer.commandBuffers[renderer.swapChainAcquiredIndex], &commandBufferBeginInfo);
				xy2d_cmdbuffer cmdbuffer = { .vkdevice = renderer.vkdevice, .cmdbuffer = renderer.commandBuffers[renderer.swapChainAcquiredIndex], .timestampQueryPool = renderer.timestampQueryPool };
				xy2d_cmdbuffer_timestamps_inject(cmdbuffer);
				xy2d_invoker_invoke<xy2d_cmdbuffer&, xy2d_gpualloc&>(renderer.renderEvent, cmdbuffer, renderer.swapChainImages[renderer.swapChainAcquiredIndex]);
				xy2d_cmdbuffer_barrier(cmdbuffer, XY2D_PIPELINESTAGES::PRESENT, XY2D_ACCESSSTAGES::PRESENT, { &renderer.swapChainImages[renderer.swapChainAcquiredIndex] }, XY2D_GPUALLOC_LAYOUT::PRESENT_SRCKHR);
				xy2d_cmdbuffer_timestamps_inject(cmdbuffer);
			vkEndCommandBuffer(renderer.commandBuffers[renderer.swapChainAcquiredIndex]);
		vkWaitForFences(renderer.vkdevice->logical, 1U, &renderer.swapChainFinished, VK_TRUE, UINT64_MAX);
		vkResetFences(renderer.vkdevice->logical, 1U, &renderer.swapChainFinished);
		
		renderer.frameTimeStamps = xy2d_cmdbuffer_timestamps_query(cmdbuffer);
		vkResetQueryPool(renderer.vkdevice->logical, renderer.timestampQueryPool, 0, XY2D_TIMESTAMPS_COUNT);
		
		VkCommandBufferSubmitInfo cmdBufferSubmitInfo = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO, .commandBuffer = renderer.commandBuffers[renderer.swapChainAcquiredIndex] };
		VkSemaphoreSubmitInfo signalSemaphoreInfo = { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO, .stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, .semaphore = renderer.swapChainPresented[renderer.swapChainAcquiredIndex] };
		VkSubmitInfo2 queueSubmitInfo = { .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2, .commandBufferInfoCount = 1U, .signalSemaphoreInfoCount = 1U, .waitSemaphoreInfoCount = 0U, .pCommandBufferInfos = &cmdBufferSubmitInfo, .pSignalSemaphoreInfos = &signalSemaphoreInfo };
		VkPresentInfoKHR presentInfo = { .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR, .swapchainCount = 1U, .waitSemaphoreCount = 1U, .pImageIndices = &renderer.swapChainAcquiredIndex, .pSwapchains = &renderer.swapChain, .pWaitSemaphores = &renderer.swapChainPresented[renderer.swapChainAcquiredIndex] };
		vkQueueSubmit2(renderer.vkdevice->renderQueue, 1U, &queueSubmitInfo, renderer.swapChainFinished);
		return vkQueuePresentKHR(renderer.vkdevice->renderQueue, &presentInfo);
	}
	
	xy2d_renderer xy2d_renderer_create(xy2d_device& vkdevice) {
		xy2d_renderer renderer = { .vkdevice = &vkdevice };
		VkSemaphoreCreateInfo semaphoreCreateInfo = { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
		for(uint32_t i = 0U; i < XY2D_BUFFERED_IMAGES; i++)
			vkCreateSemaphore(renderer.vkdevice->logical, &semaphoreCreateInfo, VK_NULL_HANDLE, &renderer.swapChainPresented[i]);
		
		VkCommandPoolCreateInfo commandPoolCreateInfo = { .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT };
		commandPoolCreateInfo.queueFamilyIndex = renderer.vkdevice->queueFamily;
		vkCreateCommandPool(renderer.vkdevice->logical, &commandPoolCreateInfo, VK_NULL_HANDLE, &renderer.commandPool);
		
		VkCommandBufferAllocateInfo commandBufferAllocateInfo = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY, .commandBufferCount = XY2D_CMDBUFFER_COUNT };
		commandBufferAllocateInfo.commandPool = renderer.commandPool;
		vkAllocateCommandBuffers(renderer.vkdevice->logical, &commandBufferAllocateInfo, renderer.commandBuffers.data());
		
		VkQueryPoolCreateInfo queryPoolCreateInfo = { .sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO, .queryType = VK_QUERY_TYPE_TIMESTAMP, .queryCount = XY2D_TIMESTAMPS_COUNT };
		vkCreateQueryPool(vkdevice.logical, &queryPoolCreateInfo, VK_NULL_HANDLE, &renderer.timestampQueryPool);
		vkResetQueryPool(vkdevice.logical, renderer.timestampQueryPool, 0, XY2D_TIMESTAMPS_COUNT);
		
		renderer.initialized = xy2d_renderer_swapchain(renderer);
		return renderer;
	}
#endif