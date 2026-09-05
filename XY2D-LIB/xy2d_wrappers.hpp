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
		};
	}
#endif