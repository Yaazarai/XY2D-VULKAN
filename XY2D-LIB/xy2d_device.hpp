#pragma once
#ifndef __XY2D_DEVICE
#define __XY2D_DEVICE
	#include "./xy2d_engine.hpp"
	
	namespace XY2D_NAMESPACE {		
		struct xy2d_queuefamily {
			uint32_t renderFamily = 0U;
			VkBool32 hasRenderFamily = VK_FALSE;
			float_t queuePriority = 1.0f;
		};
		
		class xy2d_device : public xy2d_disposable {
		public:
			inline static std::vector<const char*> deviceExtensions = {
				VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
				VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME,
				VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,
				VK_EXT_SHADER_OBJECT_EXTENSION_NAME,
				VK_EXT_COLOR_WRITE_ENABLE_EXTENSION_NAME,
				VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME,
				VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME,
				VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME,
				VK_KHR_SWAPCHAIN_EXTENSION_NAME,
				VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME,
				VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME,
				VK_KHR_UNIFIED_IMAGE_LAYOUTS_EXTENSION_NAME,
				VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME,
			};
			
			VkInstance vulkanInstance = VK_NULL_HANDLE;
			VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
			VmaAllocator memoryAllocator = VK_NULL_HANDLE;
			VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
			VkQueue deviceRenderQueue = VK_NULL_HANDLE;
			VkDevice logicalDevice = VK_NULL_HANDLE;
			VkSurfaceKHR presentSurface = VK_NULL_HANDLE;
			
			VkPhysicalDeviceProperties2 deviceProperties = {};
			xy2d_queuefamily queueFamily = {};
			VkResult initialized = VK_SUCCESS;
			
			xy2d_device(const xy2d_device&) = delete;
			xy2d_device operator=(const xy2d_device&) = delete;
			~xy2d_device() { this->Dispose(); }
			
			xy2d_device() {
				onDispose.hook(xy2d_callback<>([this]() {
					if (this->memoryAllocator != VK_NULL_HANDLE) vmaDestroyAllocator(this->memoryAllocator);
					if (this->logicalDevice != VK_NULL_HANDLE) vkDestroyDevice(this->logicalDevice, VK_NULL_HANDLE);
					if (this->presentSurface != VK_NULL_HANDLE) vkDestroySurfaceKHR(this->vulkanInstance, this->presentSurface, VK_NULL_HANDLE);
					if (this->debugMessenger != VK_NULL_HANDLE) DestroyDebugUtilsMessengerEXT(this->vulkanInstance, this->debugMessenger, VK_NULL_HANDLE);
					if (this->vulkanInstance != VK_NULL_HANDLE) vkDestroyInstance(this->vulkanInstance, VK_NULL_HANDLE);
				}));
				initialized = Initialize();
			}
			
			VkResult WaitDeviceIdle() {
				return vkDeviceWaitIdle(logicalDevice);
			}
			
			VkDeviceSize GetDeviceHeapRank(VkPhysicalDevice device) {
				VkPhysicalDeviceMemoryProperties2 memoryProperties = vk::PhysicalDeviceMemoryProperties2();
				vkGetPhysicalDeviceMemoryProperties2(device, &memoryProperties);
				
				VkDeviceSize deviceLocalSize = 0;
				for(const VkMemoryHeap& heap : memoryProperties.memoryProperties.memoryHeaps)
					deviceLocalSize += (heap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)? heap.size : 0;
				
				return deviceLocalSize;
			}
			
			xy2d_queuefamily GetDeviceQueueFamily(VkPhysicalDevice device, VkSurfaceKHR presentSurface) {
				std::vector<VkQueueFamilyProperties> queueFamilies;
				xy2d_wrappers::Enumerate(queueFamilies, vkGetPhysicalDeviceQueueFamilyProperties, device);
				
				VkQueueFlags requiredFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
				for (uint32_t qp = 0, i = 0; i < queueFamilies.size(); i++) {
					vkGetPhysicalDeviceSurfaceSupportKHR(device, i, presentSurface, &qp);
					if (qp && queueFamilies[i].timestampValidBits && ((queueFamilies[i].queueFlags & requiredFlags) == requiredFlags))
						return { i , VK_TRUE };
				}
				return { 0, VK_FALSE };
			}
			
			VkResult CreateInstance() {
				std::vector<const char*> instanceExtensions = xy2d_window::GetInstanceExtensions();
				#if XY2D_VALIDATION
					instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
				#endif
				
				std::vector<const char*> validationLayers = { VK_LAYER_KHRONOS_EXTENSION_NAME };
				VkApplicationInfo applicationCreateInfo = { .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO, .pEngineName = XY2D_ENGINE_NAME, .engineVersion = XY2D_ENGINE_VERSION, .pApplicationName = XY2D_ENGINE_NAME, .applicationVersion = XY2D_ENGINE_VERSION, .apiVersion = XY2D_ENGINE_VERSION };
				VkInstanceCreateInfo createInfo = { .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, .pApplicationInfo = &applicationCreateInfo, .enabledLayerCount = static_cast<uint32_t>(validationLayers.size()), .ppEnabledLayerNames = validationLayers.data(), .enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size()), .ppEnabledExtensionNames = instanceExtensions.data() };
				VkResult result = vkCreateInstance(&createInfo, VK_NULL_HANDLE, &vulkanInstance);
				presentSurface = xy2d_window::GetWindowSurface(vulkanInstance);
				if (result != VK_SUCCESS) return result;
				
				VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = { .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT, .pfnUserCallback = DebugCallback, .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT, .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT };
				return CreateDebugUtilsMessengerEXT(vulkanInstance, &debugCreateInfo, VK_NULL_HANDLE, &debugMessenger);
			}
			
			VkResult CreateLogicalDevice() {
				std::vector<VkPhysicalDevice> devices;
				xy2d_wrappers::Enumerate(devices, vkEnumeratePhysicalDevices, vulkanInstance);
				std::sort(devices.begin(), devices.end(), [this](auto A, auto B) { return this->GetDeviceHeapRank(A) >= this->GetDeviceHeapRank(B); });
				
				physicalDevice = (devices.size() > 0)? devices.front() : VK_NULL_HANDLE;
				if (physicalDevice == VK_NULL_HANDLE)
					return VK_ERROR_DEVICE_LOST;
				
				queueFamily = GetDeviceQueueFamily(physicalDevice, presentSurface);
                if (queueFamily.hasRenderFamily != VK_TRUE)
					return VK_ERROR_INITIALIZATION_FAILED;
				
				deviceProperties = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
				vkGetPhysicalDeviceProperties2(physicalDevice, &deviceProperties);
				
				VkPhysicalDeviceFeatures2 features2 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
				vkGetPhysicalDeviceFeatures2(physicalDevice, &features2);
				
				VkPhysicalDeviceFeatures physicalDeviceFeatures = {};
				VkPhysicalDeviceHostQueryResetFeatures hostQueryResetFeatures = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES, .hostQueryReset = VK_TRUE };
				VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingCreateInfo = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES, .dynamicRendering = VK_TRUE, .pNext = &hostQueryResetFeatures };
				VkPhysicalDeviceColorWriteEnableFeaturesEXT dynamicColorWriteFeatures = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COLOR_WRITE_ENABLE_FEATURES_EXT, .colorWriteEnable = VK_TRUE, .pNext = &dynamicRenderingCreateInfo };
				VkPhysicalDeviceExtendedDynamicState2FeaturesEXT dynamicFeautures2 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT, .extendedDynamicState2 = VK_TRUE, .extendedDynamicState2LogicOp = VK_TRUE, .pNext = &dynamicColorWriteFeatures };
				VkPhysicalDeviceExtendedDynamicState3FeaturesEXT dynamicFeautures3 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT, .extendedDynamicState3DepthClampEnable = VK_TRUE, .extendedDynamicState3PolygonMode = VK_TRUE, .extendedDynamicState3RasterizationSamples = VK_TRUE, .extendedDynamicState3SampleMask = VK_TRUE, .extendedDynamicState3AlphaToOneEnable = VK_TRUE, .extendedDynamicState3ColorBlendEnable = VK_TRUE, .extendedDynamicState3ColorBlendEquation = VK_TRUE, .pNext = &dynamicFeautures2 };
				VkPhysicalDeviceShaderObjectFeaturesEXT shaderObjectFeatures = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT, .shaderObject = VK_TRUE, .pNext = &dynamicFeautures3 };
				VkPhysicalDeviceSynchronization2Features enableSync2Features = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES, .synchronization2 = VK_TRUE, .pNext = &shaderObjectFeatures };
				VkDeviceQueueCreateInfo queueCreateInfo = { .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, .queueFamilyIndex = queueFamily.renderFamily, .queueCount = 1U, .pQueuePriorities = &queueFamily.queuePriority };
				VkDeviceCreateInfo deviceCreateInfo = { .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, .queueCreateInfoCount = 1U, .pQueueCreateInfos = &queueCreateInfo, .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()), .ppEnabledExtensionNames = deviceExtensions.data(), .pEnabledFeatures = &physicalDeviceFeatures, .pNext = &enableSync2Features };
				return vkCreateDevice(physicalDevice, &deviceCreateInfo, VK_NULL_HANDLE, &logicalDevice);
			}
			
			VkResult CreateMemoryAllocator() {
				VmaAllocatorCreateInfo allocatorCreateInfo { .vulkanApiVersion = XY2D_ENGINE_VERSION, .physicalDevice = physicalDevice, .device = logicalDevice, .instance = vulkanInstance };
				return vmaCreateAllocator(&allocatorCreateInfo, &memoryAllocator);
			}
			
			VkResult Initialize() {
				initialized = VKERROR(initialized, CreateInstance());
				initialized = VKERROR(initialized, QueryRenderingCallbacks(vulkanInstance));
				initialized = VKERROR(initialized, CreateLogicalDevice());
				initialized = VKERROR(initialized, CreateMemoryAllocator());
				vkGetDeviceQueue(logicalDevice, queueFamily.renderFamily, 0, &deviceRenderQueue);
				return initialized;
			}
		};
	}
#endif