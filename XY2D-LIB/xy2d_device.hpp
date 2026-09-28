#pragma once
#ifndef __XY2D_DEVICE
#define __XY2D_DEVICE
	#include "./xy2d_engine.hpp"
	
	const char* deviceExtensions [] = {
		VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
		VK_EXT_SHADER_OBJECT_EXTENSION_NAME,
		VK_EXT_COLOR_WRITE_ENABLE_EXTENSION_NAME,
		VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME,
		VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME,
		VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME,
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_KHR_UNIFIED_IMAGE_LAYOUTS_EXTENSION_NAME,
		VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME,
	};
	
	struct xy2d_device {
		VkInstance instance = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT debugger = VK_NULL_HANDLE;
		VmaAllocator allocator = VK_NULL_HANDLE;
		VkPhysicalDevice physical = VK_NULL_HANDLE;
		VkDevice logical = VK_NULL_HANDLE;
		VkQueue renderQueue = VK_NULL_HANDLE;
		VkSurfaceKHR surface = VK_NULL_HANDLE;
		VkPhysicalDeviceProperties2 properties = {};
		uint32_t queueFamily = 0U;
		float_t queuePriority = 1.0f;
		VkResult initialized = VK_ERROR_INITIALIZATION_FAILED;
	};
	
	void xy2d_device_destroy(xy2d_device& vkdevice) {
		if (vkdevice.allocator != VK_NULL_HANDLE) vmaDestroyAllocator(vkdevice.allocator);
		if (vkdevice.logical != VK_NULL_HANDLE) vkDestroyDevice(vkdevice.logical, VK_NULL_HANDLE);
		if (vkdevice.surface != VK_NULL_HANDLE) vkDestroySurfaceKHR(vkdevice.instance, vkdevice.surface, VK_NULL_HANDLE);
		if (vkdevice.debugger != VK_NULL_HANDLE) xy2d_debugger_destroy(vkdevice.instance, vkdevice.debugger, VK_NULL_HANDLE);
		if (vkdevice.instance != VK_NULL_HANDLE) vkDestroyInstance(vkdevice.instance, VK_NULL_HANDLE);
	}
	
	VkResult xy2d_device_instance(xy2d_device& vkdevice) {
		uint32_t extensionCount = 0;
		const char* const* extensions = SDL_Vulkan_GetInstanceExtensions(&extensionCount);
		std::vector<const char*> instanceExtensions = std::vector<const char*>( extensions, extensions + extensionCount);
		
		#if XY2D_VALIDATION
			instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		#endif
		
		const char* validationLayers[] = { VK_LAYER_KHRONOS_EXTENSION_NAME };
		VkApplicationInfo applicationCreateInfo = { .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO, .pEngineName = XY2D_ENGINE_NAME, .engineVersion = XY2D_ENGINE_VERSION, .pApplicationName = XY2D_ENGINE_NAME, .applicationVersion = XY2D_ENGINE_VERSION, .apiVersion = XY2D_ENGINE_VERSION };
		VkInstanceCreateInfo createInfo = { .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, .pApplicationInfo = &applicationCreateInfo, .enabledLayerCount = 1U, .ppEnabledLayerNames = validationLayers, .enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size()), .ppEnabledExtensionNames = instanceExtensions.data() };
		VkResult result = vkCreateInstance(&createInfo, VK_NULL_HANDLE, &vkdevice.instance);
		
		#if XY2D_VALIDATION
			VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = { .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT, .pfnUserCallback = xy2d_debugger_callback, .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT, .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT };
			result = xy2d_debugger_create(vkdevice.instance, &debugCreateInfo, VK_NULL_HANDLE, &vkdevice.debugger);
		#endif
		
		return (SDL_Vulkan_CreateSurface(window.handle, vkdevice.instance, VK_NULL_HANDLE, &vkdevice.surface))? VK_SUCCESS : result;
	}
	
	VkDeviceSize xy2d_device_get_heaprank(VkPhysicalDevice physical) {
		VkPhysicalDeviceMemoryProperties memoryProperties;
		vkGetPhysicalDeviceMemoryProperties(physical, &memoryProperties);
		
		VkDeviceSize localHeapsize = 0;
		for(size_t i = 0; i < memoryProperties.memoryHeapCount; i++)
			if (memoryProperties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) localHeapsize += memoryProperties.memoryHeaps[i].size;
		return localHeapsize;
	}
	
	VkResult xy2d_device_logical(xy2d_device& vkdevice) {
		std::vector<VkPhysicalDevice> devices;
		xy2d_enumerate(devices, vkEnumeratePhysicalDevices, vkdevice.instance);
		std::sort(devices.begin(), devices.end(), [](auto A, auto B) { return xy2d_device_get_heaprank(A) >= xy2d_device_get_heaprank(B); });
		
		vkdevice.physical = (devices.size() > 0)? devices.front() : VK_NULL_HANDLE;
		if (vkdevice.physical == VK_NULL_HANDLE) return VK_ERROR_DEVICE_LOST;
		
		std::vector<VkQueueFamilyProperties> queueFamilies;
		xy2d_enumerate(queueFamilies, vkGetPhysicalDeviceQueueFamilyProperties, vkdevice.physical);
		
		VkQueueFlags requiredFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
		for (VkBool32 qp = 0, i = 0; i < queueFamilies.size(); vkdevice.queueFamily = i++)
			if (vkGetPhysicalDeviceSurfaceSupportKHR(vkdevice.physical, i, vkdevice.surface, &qp) == VK_SUCCESS)
				if (qp && queueFamilies[i].timestampValidBits && ((queueFamilies[i].queueFlags & requiredFlags) == requiredFlags)) break;
		
		vkdevice.properties = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
		vkGetPhysicalDeviceProperties2(vkdevice.physical, &vkdevice.properties);
		
		VkPhysicalDeviceFeatures2 features2 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
		vkGetPhysicalDeviceFeatures2(vkdevice.physical, &features2);
		
		VkPhysicalDeviceFeatures physicalDeviceFeatures = {};
		VkPhysicalDeviceHostQueryResetFeatures hostQueryResetFeatures = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES, .hostQueryReset = VK_TRUE };
		VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingCreateInfo = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES, .dynamicRendering = VK_TRUE, .pNext = &hostQueryResetFeatures };
		VkPhysicalDeviceColorWriteEnableFeaturesEXT dynamicColorWriteFeatures = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COLOR_WRITE_ENABLE_FEATURES_EXT, .colorWriteEnable = VK_TRUE, .pNext = &dynamicRenderingCreateInfo };
		VkPhysicalDeviceExtendedDynamicState2FeaturesEXT dynamicFeautures2 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT, .extendedDynamicState2 = VK_TRUE, .extendedDynamicState2LogicOp = VK_TRUE, .pNext = &dynamicColorWriteFeatures };
		VkPhysicalDeviceExtendedDynamicState3FeaturesEXT dynamicFeautures3 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT, .extendedDynamicState3DepthClampEnable = VK_TRUE, .extendedDynamicState3PolygonMode = VK_TRUE, .extendedDynamicState3RasterizationSamples = VK_TRUE, .extendedDynamicState3SampleMask = VK_TRUE, .extendedDynamicState3AlphaToOneEnable = VK_TRUE, .extendedDynamicState3ColorBlendEnable = VK_TRUE, .extendedDynamicState3ColorBlendEquation = VK_TRUE, .pNext = &dynamicFeautures2 };
		VkPhysicalDeviceShaderObjectFeaturesEXT shaderObjectFeatures = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT, .shaderObject = VK_TRUE, .pNext = &dynamicFeautures3 };
		VkPhysicalDeviceSynchronization2Features enableSync2Features = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES, .synchronization2 = VK_TRUE, .pNext = &shaderObjectFeatures };
		VkDeviceQueueCreateInfo queueCreateInfo = { .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, .queueFamilyIndex = vkdevice.queueFamily, .queueCount = 1U, .pQueuePriorities = &vkdevice.queuePriority };
		
		VkDeviceCreateInfo deviceCreateInfo = { .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, .queueCreateInfoCount = 1U, .pQueueCreateInfos = &queueCreateInfo, .enabledExtensionCount = static_cast<uint32_t>(sizeof(deviceExtensions) / sizeof(deviceExtensions[0])), .ppEnabledExtensionNames = deviceExtensions, .pEnabledFeatures = &physicalDeviceFeatures, .pNext = &enableSync2Features };
		VkResult result = vkCreateDevice(vkdevice.physical, &deviceCreateInfo, VK_NULL_HANDLE, &vkdevice.logical);
		if (result != VK_SUCCESS) return result;
		
		VmaAllocatorCreateInfo allocatorCreateInfo { .vulkanApiVersion = XY2D_ENGINE_VERSION, .physicalDevice = vkdevice.physical, .device = vkdevice.logical, .instance = vkdevice.instance };
		return vmaCreateAllocator(&allocatorCreateInfo, &vkdevice.allocator);
	}
	
	xy2d_device xy2d_device_create() {
		xy2d_device vkdevice = {};
		vkdevice.initialized = xy2d_device_instance(vkdevice);
		vkdevice.initialized = VKERROR(vkdevice.initialized, xy2d_device_logical(vkdevice));
		vkdevice.initialized = VKERROR(vkdevice.initialized, xy2d_device_render_callbacks(vkdevice.instance));
		vkGetDeviceQueue(vkdevice.logical, vkdevice.queueFamily, 0, &vkdevice.renderQueue);
		return vkdevice;
	}
#endif