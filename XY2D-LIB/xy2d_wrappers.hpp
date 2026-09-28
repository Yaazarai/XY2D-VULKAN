/// @brief Source: https://stackoverflow.com/questions/9568150/what-is-a-c-delegate/9568485#9568485
#pragma once
#ifndef __XY2D_WRAPPERS
#define __XY2D_WRAPPERS
	#include "./xy2d_engine.hpp"
	
	template<typename EF, typename T, typename... VKArgs>
	void xy2d_enumerate(std::vector<T>& enumData, EF enumerateFunction, VKArgs... vkargs) {
		uint32_t count = 0;
		enumerateFunction(vkargs..., &count, VK_NULL_HANDLE);
		enumData.resize(count);
		enumerateFunction(vkargs..., &count, enumData.data());
	}
	
	VkColorComponentFlags xy2d_get_color_components(VkFormat format) {
		VkColorComponentFlags flags = 0;
		if (vkuFormatHasRed(format)) flags |= VK_COLOR_COMPONENT_R_BIT;
		if (vkuFormatHasGreen(format)) flags |= VK_COLOR_COMPONENT_G_BIT;
		if (vkuFormatHasBlue(format)) flags |= VK_COLOR_COMPONENT_B_BIT;
		if (vkuFormatHasAlpha(format)) flags |= VK_COLOR_COMPONENT_A_BIT;
		return flags;
	}
	
	VkResult xy2d_debugger_create(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
		auto create = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		return (create != VK_NULL_HANDLE)? create(instance, pCreateInfo, pAllocator, pDebugMessenger) : VK_ERROR_INITIALIZATION_FAILED;
	}
	
	VkResult xy2d_debugger_destroy(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
		auto destroy = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		destroy(instance, debugMessenger, pAllocator);
		
		return (destroy != VK_NULL_HANDLE)? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED;
	}
	
	VKAPI_ATTR VkBool32 VKAPI_CALL xy2d_debugger_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
		std::cout << "xy2d-engine: Validation Layer: " << pCallbackData->pMessage << std::endl;
		
		return (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)? VK_TRUE : VK_FALSE;
	}
	
	#define VK_EXTFN_LIST(LIST) \
		LIST(vkCreateShadersEXT) \
		LIST(vkDestroyShaderEXT) \
		LIST(vkCmdBindShadersEXT) \
		LIST(vkCmdSetColorWriteEnableEXT) \
		LIST(vkCmdSetColorBlendEnableEXT) \
		LIST(vkCmdSetColorBlendEquationEXT) \
		LIST(vkCmdSetColorWriteMaskEXT) \
		LIST(vkCmdSetDepthClampEnableEXT) \
		LIST(vkCmdSetVertexInputEXT) \
		LIST(vkCmdSetPolygonModeEXT) \
		LIST(vkCmdSetRasterizationSamplesEXT) \
		LIST(vkCmdSetSampleMaskEXT) \
		LIST(vkCmdSetAlphaToCoverageEnableEXT)
	
	#define VKDECLARE_EXTFN(FN) PFN_##FN FN##XY2D = VK_NULL_HANDLE;
		VK_EXTFN_LIST(VKDECLARE_EXTFN)
	#undef VKDECLARE_EXTFN
	
	#define VKIMPORT_EXTFN(FN) FN##XY2D = (PFN_##FN)vkGetInstanceProcAddr(instance, #FN); \
		if (FN##XY2D == VK_NULL_HANDLE) return VK_ERROR_FEATURE_NOT_PRESENT;
	
	VkResult xy2d_device_render_callbacks(VkInstance instance) {
		VK_EXTFN_LIST(VKIMPORT_EXTFN)
		return VK_SUCCESS;
	}
	
	#undef VKIMPORT_EXTFN
#endif