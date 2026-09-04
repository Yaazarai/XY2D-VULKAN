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
				VkPhysicalDeviceMemoryProperties2 memoryProperties = xy2d_wrappers::defaultMemoryProperties2;
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
				instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
				
				VkInstanceCreateInfo createInfo = xy2d_wrappers::defaultInstanceCreateInfo;
				createInfo.enabledExtensionCount = (XY2D_VALIDATION)? instanceExtensions.size() : instanceExtensions.size() - 1U;
				createInfo.ppEnabledExtensionNames = instanceExtensions.data();
				
				VkResult result = vkCreateInstance(&createInfo, VK_NULL_HANDLE, &vulkanInstance);
				if (result != VK_SUCCESS) return result;
				
				presentSurface = xy2d_window::GetWindowSurface(vulkanInstance);
				return CreateDebugUtilsMessengerEXT(vulkanInstance, &xy2d_wrappers::defaultDebugCreateInfo, VK_NULL_HANDLE, &debugMessenger);
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
				
				deviceProperties = xy2d_wrappers::defaultPhysicalDeviceProperties;
				vkGetPhysicalDeviceProperties2(physicalDevice, &deviceProperties);
				
				VkPhysicalDeviceFeatures2 features2 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
				vkGetPhysicalDeviceFeatures2(physicalDevice, &features2);
				
				VkDeviceQueueCreateInfo queueCreateInfo = xy2d_wrappers::defaultQueueCreateInfo;
				queueCreateInfo.queueFamilyIndex = queueFamily.renderFamily;
				queueCreateInfo.pQueuePriorities = &queueFamily.queuePriority;
				
				VkDeviceCreateInfo deviceCreateInfo = xy2d_wrappers::defaultDeviceCreateInfo;
				deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
				deviceCreateInfo.pNext = &xy2d_wrappers::defaultEnableSync2Features; 
				
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