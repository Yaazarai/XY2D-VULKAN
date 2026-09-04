#pragma once
#ifndef __XY2D_RENDERER
#define __XY2D_RENDERER
	#include ".\xy2d_engine.hpp"
	
	namespace XY2D_NAMESPACE {
		class xy2d_renderer : xy2d_disposable {
		public:
			xy2d_device& vkdevice;
			xy2d_invoker<xy2d_renderer&, xy2d_cmdbuffer&> renderEvent;
			
			std::vector<xy2d_image*> swapChainImages = std::vector<xy2d_image*>(XY2D_BUFFERED_IMAGES);
			VkSwapchainKHR swapChain = VK_NULL_HANDLE;
			uint32_t swapChainAcquiredIndex = 0U;
			
			std::vector<VkCommandBuffer> commandBuffers = std::vector<VkCommandBuffer>(XY2D_CMDBUFFER_COUNT);
			VkCommandPool commandPool = VK_NULL_HANDLE;
			VkQueryPool timestampQueryPool = VK_FALSE;
			
			std::vector<VkSemaphore> swapChainPresented = std::vector<VkSemaphore>(XY2D_BUFFERED_IMAGES);
			VkFence swapChainAcquired = VK_NULL_HANDLE;
			VkFence swapChainFinished = VK_NULL_HANDLE;
			
			VkSurfaceFormatKHR presentFormat = { VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
			VkSurfaceCapabilitiesKHR capabilities = {};
			VkResult initialized = VK_ERROR_INITIALIZATION_FAILED;
			
			xy2d_renderer operator=(const xy2d_renderer&) = delete;
			xy2d_renderer(const xy2d_renderer&) = delete;
			~xy2d_renderer() { this->Dispose(); }
			
			xy2d_renderer(xy2d_device& vkdevice) : vkdevice(vkdevice) {
				onDispose.hook(xy2d_callback<>([this]() {
					std::for_each(swapChainImages.begin(), swapChainImages.end(), [](xy2d_image* swapImage) { delete swapImage; });
					std::for_each(swapChainPresented.begin(), swapChainPresented.end(), [this](VkSemaphore semaphore) { vkDestroySemaphore(this->vkdevice.logicalDevice, semaphore, VK_NULL_HANDLE); });
					if (this->swapChainAcquired != VK_NULL_HANDLE) vkDestroyFence(this->vkdevice.logicalDevice, this->swapChainAcquired, VK_NULL_HANDLE);
					if (this->swapChainFinished != VK_NULL_HANDLE) vkDestroyFence(this->vkdevice.logicalDevice, this->swapChainFinished, VK_NULL_HANDLE);
					if (this->timestampQueryPool != VK_NULL_HANDLE) vkDestroyQueryPool(this->vkdevice.logicalDevice, this->timestampQueryPool, VK_NULL_HANDLE);
					if (this->commandPool != VK_NULL_HANDLE) vkDestroyCommandPool(this->vkdevice.logicalDevice, this->commandPool, VK_NULL_HANDLE);
					if (this->swapChain != VK_NULL_HANDLE) vkDestroySwapchainKHR(this->vkdevice.logicalDevice, this->swapChain, VK_NULL_HANDLE);
				}));
				
				initialized = Initialize();
			}
			
			VkResult ReCreateSwapChainImages() {
				std::vector<VkSurfaceFormatKHR> surfaceFormats;
				xy2d_wrappers::Enumerate(surfaceFormats, vkGetPhysicalDeviceSurfaceFormatsKHR, vkdevice.physicalDevice, vkdevice.presentSurface);
				vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkdevice.physicalDevice, vkdevice.presentSurface, &capabilities);
				
				auto findFormat = std::find_if(surfaceFormats.begin(), surfaceFormats.end(), [this](const VkSurfaceFormatKHR& format) { return this->presentFormat.colorSpace == format.colorSpace && this->presentFormat.format == format.format; });
				if (findFormat == surfaceFormats.end())
					findFormat = std::find_if(surfaceFormats.begin(), surfaceFormats.end(), [this](const VkSurfaceFormatKHR& format) { return this->presentFormat.colorSpace == format.colorSpace; });
				presentFormat = *findFormat;
				
				uint32_t swapWidth = xy2d_window::width.load(std::memory_order_relaxed);
				uint32_t swapHeight = xy2d_window::height.load(std::memory_order_relaxed);
				uint32_t width = std::clamp(swapWidth, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
				uint32_t height = std::clamp(swapHeight, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
				VkExtent2D swapChainExtent = { width, height };
				
				VkSwapchainCreateInfoKHR swapChainCreateInfo = xy2d_wrappers::defaultSwapchainCreateInfo;
				swapChainCreateInfo.surface = vkdevice.presentSurface;
				swapChainCreateInfo.minImageCount = XY2D_BUFFERED_IMAGES;
				swapChainCreateInfo.imageExtent = swapChainExtent;
				swapChainCreateInfo.imageFormat = presentFormat.format;
				swapChainCreateInfo.preTransform = capabilities.currentTransform;
				swapChainCreateInfo.imageColorSpace = presentFormat.colorSpace;
				swapChainCreateInfo.oldSwapchain = swapChain;
				VkResult result = vkCreateSwapchainKHR(vkdevice.logicalDevice, &swapChainCreateInfo, VK_NULL_HANDLE, &swapChain);
				if (result != VK_SUCCESS) return result;
				
				std::for_each(swapChainImages.begin(), swapChainImages.end(), [](xy2d_image* swapImage){ delete swapImage; });
				vkDestroySwapchainKHR(this->vkdevice.logicalDevice, swapChainCreateInfo.oldSwapchain, VK_NULL_HANDLE);
				
				std::vector<VkImage> newSwapImages = std::vector<VkImage>(XY2D_BUFFERED_IMAGES);
				xy2d_wrappers::Enumerate(newSwapImages, vkGetSwapchainImagesKHR, vkdevice.logicalDevice, swapChain);
				
				for(uint32_t i = 0; i < swapChainImages.size(); i++)
					swapChainImages[i] = new xy2d_image(vkdevice, XY2D_IMAGETYPE::SWAPCHAIN, swapChainExtent.width, swapChainExtent.height, presentFormat.format, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, VK_FALSE, newSwapImages[i]);
				
				if (this->swapChainAcquired != VK_NULL_HANDLE)
					vkDestroyFence(this->vkdevice.logicalDevice, this->swapChainAcquired, VK_NULL_HANDLE);
				vkCreateFence(vkdevice.logicalDevice, &xy2d_wrappers::unsignaledFenceCreateInfo, VK_NULL_HANDLE, &swapChainAcquired);
				
				if (this->swapChainFinished != VK_NULL_HANDLE)
					vkDestroyFence(this->vkdevice.logicalDevice, this->swapChainFinished, VK_NULL_HANDLE);
				vkCreateFence(vkdevice.logicalDevice, &xy2d_wrappers::signaledFenceCreateInfo, VK_NULL_HANDLE, &swapChainFinished);
				return result;
			}
			
			VkResult FrameRenderAndPresent() {
				vkWaitForFences(vkdevice.logicalDevice, 1U, &swapChainAcquired, VK_TRUE, UINT64_MAX);
				vkResetFences(vkdevice.logicalDevice, 1U, &swapChainAcquired);
				
				vkResetCommandBuffer(commandBuffers[swapChainAcquiredIndex], 0U);
				vkBeginCommandBuffer(commandBuffers[swapChainAcquiredIndex], &xy2d_wrappers::defaultCommandBufferBeginInfo);
					xy2d_cmdbuffer cmdbuffer (vkdevice, *swapChainImages[swapChainAcquiredIndex], commandBuffers[swapChainAcquiredIndex], timestampQueryPool);
					cmdbuffer.InjectTimestamp();
						renderEvent.invoke(*this, cmdbuffer);
						
						cmdbuffer.TransitionImageLayouts(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, XY2D_PIPELINESTAGES::PRESENT, XY2D_ACCESSSTAGES::PRESENT, { swapChainImages[swapChainAcquiredIndex] });
					cmdbuffer.InjectTimestamp();
				vkEndCommandBuffer(commandBuffers[swapChainAcquiredIndex]);
				
				vkWaitForFences(vkdevice.logicalDevice, 1U, &swapChainFinished, VK_TRUE, UINT64_MAX);
				vkResetFences(vkdevice.logicalDevice, 1U, &swapChainFinished);
				
				VkCommandBufferSubmitInfo cmdBufferSubmitInfo = xy2d_wrappers::defaultCmdBufferSubmitInfo;
				cmdBufferSubmitInfo.commandBuffer = commandBuffers[swapChainAcquiredIndex];
				
				VkSemaphoreSubmitInfo signalSemaphoreInfo = xy2d_wrappers::defaultSignalSemaphoreInfo;
				signalSemaphoreInfo.semaphore = swapChainPresented[swapChainAcquiredIndex];
				
				VkSubmitInfo2 queueSubmitInfo = xy2d_wrappers::defaultQueueSubmitInfo;
				queueSubmitInfo.pCommandBufferInfos = &cmdBufferSubmitInfo;
				queueSubmitInfo.pSignalSemaphoreInfos = &signalSemaphoreInfo;
				VkResult result = vkQueueSubmit2(vkdevice.deviceRenderQueue, 1U, &queueSubmitInfo, swapChainFinished);
				if (result != VK_SUCCESS) return result;
				
				VkPresentInfoKHR presentInfo = xy2d_wrappers::defaultPresentInfo;
				presentInfo.pImageIndices = &swapChainAcquiredIndex;
				presentInfo.pSwapchains = &swapChain;
				presentInfo.pWaitSemaphores = &swapChainPresented[swapChainAcquiredIndex];
				return vkQueuePresentKHR(vkdevice.deviceRenderQueue, &presentInfo);
			}
			
			VkResult RenderSwapChain() {
				VkResult result = vkAcquireNextImageKHR(vkdevice.logicalDevice, swapChain, UINT64_MAX, VK_NULL_HANDLE, swapChainAcquired, &swapChainAcquiredIndex);
				if (result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR)
					return FrameRenderAndPresent();
				
				vkWaitForFences(vkdevice.logicalDevice, 1U, &swapChainFinished, VK_TRUE, UINT64_MAX);
				vkResetFences(vkdevice.logicalDevice, 1U, &swapChainFinished);
				return ReCreateSwapChainImages();
			}
			
			VkResult Initialize() {
				for(uint32_t i = 0U; i < XY2D_BUFFERED_IMAGES; i++)
					vkCreateSemaphore(vkdevice.logicalDevice, &xy2d_wrappers::defaultSemaphoreCreateInfo, VK_NULL_HANDLE, &swapChainPresented[i]);
				
				VkCommandPoolCreateInfo commandPoolCreateInfo = xy2d_wrappers::defaultCommandPoolCreateInfo;
				commandPoolCreateInfo.queueFamilyIndex = vkdevice.queueFamily.renderFamily;
				vkCreateCommandPool(vkdevice.logicalDevice, &commandPoolCreateInfo, VK_NULL_HANDLE, &commandPool);
				
				VkCommandBufferAllocateInfo commandBufferAllocateInfo = xy2d_wrappers::defaultCommandBufferAllocateInfo;
				commandBufferAllocateInfo.commandPool = commandPool;
				vkAllocateCommandBuffers(vkdevice.logicalDevice, &commandBufferAllocateInfo, commandBuffers.data());
				
				vkCreateQueryPool(vkdevice.logicalDevice, &xy2d_wrappers::defaultQueryCreateInfo, VK_NULL_HANDLE, &timestampQueryPool);
				return ReCreateSwapChainImages();
			}
		};
	}
#endif