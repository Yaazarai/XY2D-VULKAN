#pragma once
#ifndef __XY2D_ENGINE
#define __XY2D_ENGINE

    #define GLM_FORCE_RADIANS
    #define GLM_FORCE_LEFT_HANDED
    #define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
	#include <glm/glm.hpp>
	#include <glm/ext.hpp>
	
	#define SDL_MAIN_USE_CALLBACKS
	#include <SDL3/SDL.h>
	#include <SDL3/SDL_main.h>
	#include <SDL3/SDL_vulkan.h>
	
	#define VMA_IMPLEMENTATION
	#define VMA_DEBUG_GLOBAL_MUTEX VK_TRUE
	#define VMA_USE_STL_CONTAINERS VK_TRUE
	#define VMA_RECORDING_ENABLED XYVK_VALIDATION
	#include <vma/vk_mem_alloc.h>
	
	#include <vulkan/vulkan.h>
	#include <vulkan/vulkan.hpp>
	#include <vulkan/utility/vk_format_utils.h>
	
	#ifdef _DEBUG
		#define XY2D_VALIDATION true
	#else
		#define XY2D_VALIDATION false
	#endif
	
	#ifndef XY2D_NAMESPACE
		#define XY2D_NAMESPACE xy2d
		namespace XY2D_NAMESPACE {}
	#endif
	
	#define VKERROR(resvar, result) ((resvar == VK_SUCCESS)? result : resvar)
	#define VK_LAYER_KHRONOS_EXTENSION_NAME "VK_LAYER_KHRONOS_validation"
	#define XY2D_ENGINE_VERSION VK_API_VERSION_1_4
	#define XY2D_ENGINE_NAME "XY2D_ENGINE"
	
	#ifndef XY2D_BUFFERED_IMAGES
		#define XY2D_BUFFERED_IMAGES 3U
	#endif
	#ifndef XY2D_CMDBUFFER_COUNT
		#define XY2D_CMDBUFFER_COUNT 64U
	#endif
	#ifndef XY2D_TIMESTAMPS_COUNT
		#define XY2D_TIMESTAMPS_COUNT 256U
	#endif
	
	#include <thread>
	#include <filesystem>
	#include <fstream>
	#include <iostream>
	#include <sstream>
	#include <string>
	#include <vector>
	#include <functional>
	#include <utility>
	#include <mutex>
	
	#include "./xy2d_wrappers.hpp"
	#include "./xy2d_callback.hpp"
	#include "./xy2d_disposable.hpp"
	#include "./xy2d_window.hpp"
	#include "./xy2d_device.hpp"
	#include "./xy2d_buffer.hpp"
	#include "./xy2d_image.hpp"
	#include "./xy2d_shader.hpp"
	#include "./xy2d_pipeline.hpp"
	#include "./xy2d_cmdbuffer.hpp"
	#include "./xy2d_renderer.hpp"
	#include "./xy2d_sprite.hpp"
#endif

/*
	ARCHITECTURE: SBFIF (Single-Buffered Frame-In-Flight Rendering)
		A simplifed CPU-GPU parallel rendering model.
	
	The CPU builds frame N+1 while waiting on frame N to complete rendering. This is done by staggering two
	VkFences for CPU side rendering synchronization:
		1. Fence A - Wait to acquire a presentation image from the swap chain.
		2. Fence B - Wait for the previous GPU submitted frame to complete.
	
	CPU work is executed and Command Buffers are recorded and then we hold that command buffer for the current
	frame until frame N has completed its rendering. Fence B waits for frame N to complete and then we immediately
	submit the next command buffer to keep the GPU busy. In practice this gives us one Frame-In-Flight (FIF) while
	preparing the next frame for execution while avoiding Multi-Buffered resources.
	
	Swapchain Resize Logic:
		When resizing the swapchain we wait for only the rendering fence (Fence B) to finish executing before recreating
		the swapchain with the new sizes. This is because we have to reset the fences back to their initial states
		at application start for the synchronization logic to work--which requires destorying/recreating fences.
		
		However we don't have to wait on Fence A because it doesn't representing any pending GPU rendering operations,
		it's only waiting to be signaled by the presentation engine to signal that a swap image has been acquired.
		
		When the swapchain is resized simply return and try re-rendering the frame.
*/

/*
	How xy2d_cmdbuffer works:
		Typically with Vulkan pipeline barriers you synchronize the current memory access/stages for your desired resources
		with the destination access/stages. For example if the current pass is COMPUTE and the next pass is RENDER then our
		appropriate access are current (COMPUTE) and destination (RENDER).
		
		However instead xy2d_cmdbuffer pipeline barriers synchronize execution of command groups within a command buffer.
		We can place barriers before the current stage and treat it as the "next stage" for synchronization. This produces
		the same result but we're forward tracking that state for simplicity since we may not otherwise know what the "next"
		pass defined by the user is without storing state and deffering execution of commands.
*/