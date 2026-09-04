/// @brief Source: https://stackoverflow.com/questions/9568150/what-is-a-c-delegate/9568485#9568485
#pragma once
#ifndef __XY2D_WRAPPERS
#define __XY2D_WRAPPERS
	#include "./xy2d_engine.hpp"
	
	namespace XY2D_NAMESPACE {
		VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
			#if XY2D_VALIDATION
				auto create = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
				if (create == VK_NULL_HANDLE) return VK_ERROR_INITIALIZATION_FAILED;
				return create(instance, pCreateInfo, pAllocator, pDebugMessenger);
			#endif
			return VK_SUCCESS;
		}
		
		VkResult DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
			#if XY2D_VALIDATION
				auto destroy = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
				destroy(instance, debugMessenger, pAllocator);
				if (destroy == VK_NULL_HANDLE) return VK_ERROR_INITIALIZATION_FAILED;
			#endif
			return VK_SUCCESS;
		}
		
		VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
			#if XY2D_VALIDATION
				std::cout << "xy2d-engine: Validation Layer: " << pCallbackData->pMessage << std::endl;
				return (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)? VK_TRUE : VK_FALSE;
			#endif
			return VK_FALSE;
		}
		
		#define VKDECLARE_EXTFN(FN,SFX) PFN_##FN FN##SFX = VK_NULL_HANDLE
		#define VKIMPORTS_EXTFN(FN,SFX) FN##SFX = (PFN_##FN) vkGetInstanceProcAddr(instance, #FN); \
			if (FN##SFX == VK_NULL_HANDLE) { std::cout << "xy2d-engine: Failed to load VK_KHR_dynamic_rendering EXT function: PFN_"#FN << std::endl; return VK_ERROR_FEATURE_NOT_PRESENT; }
		
		VKDECLARE_EXTFN(vkCmdBeginRenderingKHR,XY2D);
		VKDECLARE_EXTFN(vkCmdEndRenderingKHR,XY2D);
		VKDECLARE_EXTFN(vkCmdPushDescriptorSetKHR,XY2D);
		VKDECLARE_EXTFN(vkCreateShadersEXT, XY2D);
		VKDECLARE_EXTFN(vkDestroyShaderEXT,XY2D);
		VKDECLARE_EXTFN(vkCmdBindShadersEXT,XY2D);
		VKDECLARE_EXTFN(vkCmdSetColorWriteEnableEXT,XY2D);
		VKDECLARE_EXTFN(vkCmdSetColorBlendEnableEXT,XY2D);
		VKDECLARE_EXTFN(vkCmdSetColorBlendEquationEXT,XY2D);
		VKDECLARE_EXTFN(vkCmdSetColorWriteMaskEXT,XY2D);
		VKDECLARE_EXTFN(vkCmdSetDepthWriteEnableEXT,XY2D);
		VKDECLARE_EXTFN(vkCmdSetDepthTestEnableEXT,XY2D);
		VKDECLARE_EXTFN(vkCmdSetDepthClampEnableEXT,XY2D);
		VKDECLARE_EXTFN(vkCmdSetDepthBiasEnableEXT,XY2D);
		VKDECLARE_EXTFN(vkCmdSetVertexInputEXT,XY2D);
		VKDECLARE_EXTFN(vkCmdSetPrimitiveTopologyEXT,XY2D);
		VKDECLARE_EXTFN(vkCmdSetPolygonModeEXT,XY2D);
		VKDECLARE_EXTFN(vkCmdSetCullModeEXT,XY2D);
		VKDECLARE_EXTFN(vkCmdSetFrontFaceEXT,XY2D);
		VKDECLARE_EXTFN(vkCmdSetRasterizerDiscardEnableEXT,XY2D);
		VKDECLARE_EXTFN(vkCmdSetStencilTestEnableEXT,XY2D);
		VKDECLARE_EXTFN(vkCmdSetRasterizationSamplesEXT,XY2D);
		VKDECLARE_EXTFN(vkCmdSetSampleMaskEXT,XY2D);
		VKDECLARE_EXTFN(vkCmdSetAlphaToCoverageEnableEXT,XY2D);
		VKDECLARE_EXTFN(vkCmdSetPrimitiveRestartEnableEXT,XY2D);
		VKDECLARE_EXTFN(vkCmdSetViewportWithCountEXT,XY2D);
		VKDECLARE_EXTFN(vkCmdSetScissorWithCountEXT,XY2D);
		
		VkResult QueryRenderingCallbacks(VkInstance instance) {
			VKIMPORTS_EXTFN(vkCmdBeginRenderingKHR,XY2D);
			VKIMPORTS_EXTFN(vkCmdEndRenderingKHR,XY2D);
			VKIMPORTS_EXTFN(vkCmdPushDescriptorSetKHR,XY2D);
			VKIMPORTS_EXTFN(vkCreateShadersEXT, XY2D);
			VKIMPORTS_EXTFN(vkDestroyShaderEXT,XY2D);
			VKIMPORTS_EXTFN(vkCmdBindShadersEXT,XY2D);
			VKIMPORTS_EXTFN(vkCmdSetColorWriteEnableEXT,XY2D);
			VKIMPORTS_EXTFN(vkCmdSetColorBlendEnableEXT,XY2D);
			VKIMPORTS_EXTFN(vkCmdSetColorBlendEquationEXT,XY2D);
			VKIMPORTS_EXTFN(vkCmdSetColorWriteMaskEXT,XY2D);
			VKIMPORTS_EXTFN(vkCmdSetDepthWriteEnableEXT,XY2D);
			VKIMPORTS_EXTFN(vkCmdSetDepthTestEnableEXT,XY2D);
			VKIMPORTS_EXTFN(vkCmdSetDepthClampEnableEXT,XY2D);
			VKIMPORTS_EXTFN(vkCmdSetDepthBiasEnableEXT,XY2D);
			VKIMPORTS_EXTFN(vkCmdSetVertexInputEXT,XY2D);
			VKIMPORTS_EXTFN(vkCmdSetPrimitiveTopologyEXT,XY2D);
			VKIMPORTS_EXTFN(vkCmdSetPolygonModeEXT,XY2D);
			VKIMPORTS_EXTFN(vkCmdSetCullModeEXT,XY2D);
			VKIMPORTS_EXTFN(vkCmdSetFrontFaceEXT,XY2D);
			VKIMPORTS_EXTFN(vkCmdSetRasterizerDiscardEnableEXT,XY2D);
			VKIMPORTS_EXTFN(vkCmdSetStencilTestEnableEXT,XY2D);
			VKIMPORTS_EXTFN(vkCmdSetRasterizationSamplesEXT,XY2D);
			VKIMPORTS_EXTFN(vkCmdSetSampleMaskEXT,XY2D);
			VKIMPORTS_EXTFN(vkCmdSetAlphaToCoverageEnableEXT,XY2D);
			VKIMPORTS_EXTFN(vkCmdSetPrimitiveRestartEnableEXT,XY2D);
			VKIMPORTS_EXTFN(vkCmdSetViewportWithCountEXT,XY2D);
			VKIMPORTS_EXTFN(vkCmdSetScissorWithCountEXT,XY2D);
			return VK_SUCCESS;
		}
		
		class xy2d_wrappers {
		public:
			template<typename EF, typename T, typename... VKArgs>
			inline static void Enumerate(std::vector<T>& enumData, EF enumerateFunction, VKArgs... vkargs) {
				uint32_t count = 0;
				enumerateFunction(vkargs..., &count, VK_NULL_HANDLE);
				enumData.resize(count);
				enumerateFunction(vkargs..., &count, enumData.data());
			}
			
			inline static VkColorComponentFlags GetColorComponentFlags(VkFormat format) {
				VkColorComponentFlags flags = 0;
				if (vkuFormatHasRed(format)) flags |= VK_COLOR_COMPONENT_R_BIT;
				if (vkuFormatHasGreen(format)) flags |= VK_COLOR_COMPONENT_G_BIT;
				if (vkuFormatHasBlue(format)) flags |= VK_COLOR_COMPONENT_B_BIT;
				if (vkuFormatHasAlpha(format)) flags |= VK_COLOR_COMPONENT_A_BIT;
				return flags;
			}
			
			inline static const char* defaultValidationLayers = VK_LAYER_KHRONOS_EXTENSION_NAME;
			
			inline static std::vector<const char*> defaultDeviceExtensions = {
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
			};
			
			inline static VkPhysicalDeviceMemoryProperties2 defaultMemoryProperties2 = {
				.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2,
				.pNext = VK_NULL_HANDLE,
				.memoryProperties = {},
			};
			
			inline static VkDebugUtilsMessengerCreateInfoEXT defaultDebugCreateInfo {
				.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
				.pNext = VK_NULL_HANDLE,
				.flags = 0,
				.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT,
				.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
				.pfnUserCallback = DebugCallback,
				.pUserData = VK_NULL_HANDLE,
			};
			
			inline static VkApplicationInfo defaultApplicationCreateInfo = {
				.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
				.pNext = VK_NULL_HANDLE,
				.pApplicationName = XY2D_ENGINE_NAME,
				.applicationVersion = XY2D_ENGINE_VERSION,
				.pEngineName = XY2D_ENGINE_NAME,
				.engineVersion = XY2D_ENGINE_VERSION,
				.apiVersion = XY2D_ENGINE_VERSION,
			};
			
			inline static VkInstanceCreateInfo defaultInstanceCreateInfo = {
				.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
				.pNext = &defaultDebugCreateInfo,
				.flags = 0,
				.pApplicationInfo = &defaultApplicationCreateInfo,
				.enabledLayerCount = XY2D_VALIDATION,
				.ppEnabledLayerNames = &defaultValidationLayers,
				.enabledExtensionCount = 0,
				.ppEnabledExtensionNames = VK_NULL_HANDLE,
			};
			
			inline static VkPhysicalDeviceProperties2 defaultPhysicalDeviceProperties = {
				.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
				.pNext = VK_NULL_HANDLE,
			};
			
			inline static VkPhysicalDevicePushDescriptorPropertiesKHR defaultPushDescriptorProperties = {
				.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PUSH_DESCRIPTOR_PROPERTIES_KHR
			};
			
			inline static VkDeviceQueueCreateInfo defaultQueueCreateInfo = {
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.queueCount = 1,
				.queueFamilyIndex = 0,
				.pQueuePriorities = VK_NULL_HANDLE,
			};
			
			inline static VkPhysicalDeviceDynamicRenderingFeatures defaultDynamicRenderingCreateInfo = {
				.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR,
				.dynamicRendering = VK_TRUE,
			};
			
			inline static VkPhysicalDeviceColorWriteEnableFeaturesEXT defaultDynamicColorWriteFeatures = {
				.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COLOR_WRITE_ENABLE_FEATURES_EXT,
				.pNext = &defaultDynamicRenderingCreateInfo,
				.colorWriteEnable = VK_TRUE,
			};
			
			inline static VkPhysicalDeviceExtendedDynamicState2FeaturesEXT defaultDynamicFeautures2 = {
				.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT,
				.pNext = &defaultDynamicColorWriteFeatures,
				.extendedDynamicState2 = VK_TRUE
			};
			
			inline static VkPhysicalDeviceExtendedDynamicState3FeaturesEXT defaultDynamicFeautures3 = {
				.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT,
				.pNext = &defaultDynamicFeautures2,
				.extendedDynamicState3ColorBlendEnable = VK_TRUE,
				.extendedDynamicState3ColorBlendEquation = VK_TRUE,
				.extendedDynamicState3ColorWriteMask = VK_TRUE,
				.extendedDynamicState3DepthClampEnable = VK_TRUE,
				.extendedDynamicState3PolygonMode = VK_TRUE,
			};
			
			inline static VkPhysicalDeviceShaderObjectFeaturesEXT defaultShaderObjectFeatures = {
				.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT,
				.pNext = &defaultDynamicFeautures3,
				.shaderObject = VK_TRUE,
			};
			
			inline static VkPhysicalDeviceSynchronization2Features defaultEnableSync2Features = {
				.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES,
				.pNext = &defaultShaderObjectFeatures,
				.synchronization2 = VK_TRUE
			};
			
			inline static VkPhysicalDeviceFeatures defaultDeviceFeatures = {
				.multiDrawIndirect = VK_TRUE,
				.fillModeNonSolid = VK_TRUE,
			};
			
			inline static VkDeviceCreateInfo defaultDeviceCreateInfo {
				.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
				.pEnabledFeatures = &defaultDeviceFeatures,
				.pNext = &defaultShaderObjectFeatures,
				.queueCreateInfoCount = 1,
				.enabledExtensionCount = static_cast<uint32_t>(defaultDeviceExtensions.size()),
				.ppEnabledExtensionNames = defaultDeviceExtensions.data(),
			};
			
			inline static VkImageCreateInfo defaultImageCreateInfo = {
				.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
				.extent.depth = 1,
				.mipLevels = 1,
				.arrayLayers = 1,
				.imageType = VK_IMAGE_TYPE_2D,
				.tiling = VK_IMAGE_TILING_OPTIMAL,
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
			};
			
			inline static VmaAllocationCreateInfo defaultImageMemoryCreateInfo = {
				.priority = 1.0f,
				.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
				.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
			};
			
			inline static VkSamplerCreateInfo defaultSamplerCreateInfo = {
				.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
				.magFilter = VK_FILTER_NEAREST,
				.minFilter = VK_FILTER_NEAREST,
				.anisotropyEnable = VK_FALSE,
				.compareEnable = VK_FALSE,
				.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
				.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
				.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
				.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
				.unnormalizedCoordinates = VK_FALSE,
				.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
				.mipLodBias = 0.0f,
				.minLod = 0.0f,
				.maxLod = VK_LOD_CLAMP_NONE,
			};
			
			inline static VkImageViewCreateInfo defaultImageViewCreateInfo = {
				.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.viewType = VK_IMAGE_VIEW_TYPE_2D,
				.components = { VK_COMPONENT_SWIZZLE_IDENTITY },
				.subresourceRange = { .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1, .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT },
			};
			
			inline static VkDescriptorSetLayoutCreateInfo defaultDescriptorCreateInfo = {
				.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
				.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR,
			};
			
			inline static VkPipelineLayoutCreateInfo defaultPipelineLayoutInfo = {
				.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
				.pPushConstantRanges = 0U,
			};
			
			inline static VkShaderCreateInfoEXT defaultShaderCreateInfo = {
				.sType = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT,
				.flags = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT,
				.codeType = VkShaderCodeTypeEXT::VK_SHADER_CODE_TYPE_SPIRV_EXT,
				.pName = "main",
				.setLayoutCount = 1,
			};
			
			inline static VkSwapchainCreateInfoKHR defaultSwapchainCreateInfo = {
				.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
				.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
				.imageArrayLayers = 1,
				.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
				.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
				.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
				.presentMode = VK_PRESENT_MODE_FIFO_KHR,
				.clipped = VK_TRUE,
			};
			
			inline static VkQueryPoolCreateInfo defaultQueryCreateInfo = {
				.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
				.queryType = VK_QUERY_TYPE_TIMESTAMP,
				.queryCount = 2U * XY2D_TIMESTAMPS_COUNT,
				.flags = 0,
			};
			
			inline static VkSemaphoreCreateInfo defaultSemaphoreCreateInfo = {
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			};
			
			inline static VkFenceCreateInfo signaledFenceCreateInfo {
				.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
				.flags = VK_FENCE_CREATE_SIGNALED_BIT,
			};
			
			inline static VkFenceCreateInfo unsignaledFenceCreateInfo {
				.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			};
			
			inline static VkCommandPoolCreateInfo defaultCommandPoolCreateInfo {
				.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
				.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			};
			
			inline static VkCommandBufferAllocateInfo defaultCommandBufferAllocateInfo {
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
				.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				.commandBufferCount = XY2D_CMDBUFFER_COUNT,
			};
			
			inline static VkCommandBufferBeginInfo defaultCommandBufferBeginInfo {
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
				.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
			};
			
			inline static VkCommandBufferSubmitInfo defaultCmdBufferSubmitInfo = {
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
			};
			
			inline static VkSemaphoreSubmitInfo defaultSignalSemaphoreInfo = {
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
				.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
			};
			
			inline static VkSubmitInfo2 defaultQueueSubmitInfo = {
				.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
				.commandBufferInfoCount = 1U,
				.signalSemaphoreInfoCount = 1U,
				.waitSemaphoreInfoCount = 0U,
			};
			
			inline static VkPresentInfoKHR defaultPresentInfo = {
				.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
				.swapchainCount = 1U,
				.waitSemaphoreCount = 1U,
			};
			
			inline static VkMemoryBarrier2 defaultMemoryBarrier = {
				.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
			};
			
			inline static VkDependencyInfo defaultDependencyInfo = {
				.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
			};
			
			inline static VkColorBlendEquationEXT defaultBlendingEquation = {
				.colorBlendOp = VK_BLEND_OP_ADD,
				.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
				.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
				.alphaBlendOp = VK_BLEND_OP_ADD,
				.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
				.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
			};
			
			inline static VkRenderingAttachmentInfoKHR defaultColorAttachmentInfo {
				.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
				.clearValue = {0.0f, 0.0f, 0.0f, 1.0f},
				.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			};
			
			inline static VkRenderingInfoKHR defaultDynamicRenderInfo {
				.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
				.layerCount = 1
			};
		};
	}
#endif