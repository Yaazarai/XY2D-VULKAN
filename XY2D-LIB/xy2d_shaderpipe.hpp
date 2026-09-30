#pragma once
#ifndef __XY2D_SHADERPIPE
#define __XY2D_SHADERPIPE
	#include "./xy2d_engine.hpp"
	
	enum class XY2D_SHADER_STAGE {
		INVALID = 0x00000000,
		VERTEX = VK_SHADER_STAGE_VERTEX_BIT,
		FRAGMENT = VK_SHADER_STAGE_FRAGMENT_BIT,
		COMPUTE = VK_SHADER_STAGE_COMPUTE_BIT,
	};
	
	enum class XY2D_SHADER_UNIFORM {
		UBO     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		SSBO    = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		IMAGE2D = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
		SAMPLER = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
	};
	
	struct xy2d_shader {
		XY2D_SHADER_STAGE shaderStage = XY2D_SHADER_STAGE::INVALID;
		std::vector<XY2D_SHADER_UNIFORM> uniforms;
		std::vector<glm::uint32_t> spirvCode;
		std::string filePath;
	};
	
	xy2d_shader xy2d_shader_create(std::string filePath, XY2D_SHADER_STAGE shaderStage, std::vector<XY2D_SHADER_UNIFORM> uniforms = {}) {
		std::ifstream file(filePath, std::ios::ate | std::ios::binary);
		xy2d_shader shader = { .shaderStage = (file.is_open())? shaderStage : XY2D_SHADER_STAGE::INVALID, .uniforms = uniforms, .filePath = filePath };
		shader.spirvCode.resize(static_cast<size_t>(file.tellg()) / sizeof(glm::uint32_t));
		if (shader.shaderStage != XY2D_SHADER_STAGE::INVALID)
			file.seekg(0).read(reinterpret_cast<char*>(shader.spirvCode.data()), shader.spirvCode.size() * sizeof(glm::uint32_t));
		return shader;
	}
	
	struct xy2d_shaderpipe {
		xy2d_device* vkdevice = VK_NULL_HANDLE;
		xy2d_gpualloc stagingBuffer = {};
		VkDescriptorSetLayout descriptorLayout = VK_NULL_HANDLE;
		VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
		std::vector<xy2d_shader> shaderSources;
		std::vector<VkShaderEXT> shaderObjects;
		std::vector<XY2D_SHADER_STAGE> shaderStages;
		VkResult initialized = VK_ERROR_INITIALIZATION_FAILED;
	};
	
	void xy2d_shaderpipe_destroy(xy2d_shaderpipe& shaderpipe) {
		for(VkShaderEXT shaderObject : shaderpipe.shaderObjects)
			vkDestroyShaderEXTXY2D(shaderpipe.vkdevice->logical, shaderObject, VK_NULL_HANDLE);
		if (shaderpipe.descriptorLayout != VK_NULL_HANDLE) vkDestroyDescriptorSetLayout(shaderpipe.vkdevice->logical, shaderpipe.descriptorLayout, VK_NULL_HANDLE);
		if (shaderpipe.pipelineLayout != VK_NULL_HANDLE) vkDestroyPipelineLayout(shaderpipe.vkdevice->logical, shaderpipe.pipelineLayout, VK_NULL_HANDLE);
		if (shaderpipe.stagingBuffer.buffer != VK_NULL_HANDLE) xy2d_gpualloc_destroy(shaderpipe.stagingBuffer);
	}
	
	xy2d_shaderpipe xy2d_shaderpipe_create(xy2d_device& vkdevice, std::vector<xy2d_shader> shaderSources, uint32_t stageBufferSize = 0U) {
		xy2d_shaderpipe shaderpipe = { .vkdevice = &vkdevice, .shaderSources = shaderSources, .initialized = VK_SUCCESS };
		std::vector<VkDescriptorSetLayoutBinding> pbindings;
		for(glm::uint32_t bindings = 0, i = 0; i < static_cast<glm::uint32_t>(shaderpipe.shaderSources.size()); i++)
			for(glm::uint32_t j = 0; j < static_cast<glm::uint32_t>(shaderpipe.shaderSources[i].uniforms.size()); j++)
				pbindings.push_back({ bindings++, static_cast<VkDescriptorType>(shaderpipe.shaderSources[i].uniforms[j]), 1U, static_cast<VkShaderStageFlags>(shaderpipe.shaderSources[i].shaderStage) });
		
		VkDescriptorSetLayoutCreateInfo descriptorCreateInfo = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR };
		descriptorCreateInfo.bindingCount = static_cast<uint32_t>(pbindings.size());
		descriptorCreateInfo.pBindings = pbindings.data();
		vkCreateDescriptorSetLayout(shaderpipe.vkdevice->logical, &descriptorCreateInfo, VK_NULL_HANDLE, &shaderpipe.descriptorLayout);
		
		VkPipelineLayoutCreateInfo pipelineLayoutInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, .pPushConstantRanges = 0U };
		pipelineLayoutInfo.setLayoutCount = (shaderpipe.descriptorLayout != VK_NULL_HANDLE)? 1U : 0U;
		pipelineLayoutInfo.pSetLayouts = &shaderpipe.descriptorLayout;
		vkCreatePipelineLayout(shaderpipe.vkdevice->logical, &pipelineLayoutInfo, VK_NULL_HANDLE, &shaderpipe.pipelineLayout);
		
		std::vector<VkShaderCreateInfoEXT> sinfos;
		for(size_t i = 0; i < shaderpipe.shaderSources.size(); i++) {
			XY2D_SHADER_STAGE nextShaderStage = (i < shaderpipe.shaderSources.size() - 1)? shaderpipe.shaderSources[i + 1].shaderStage : XY2D_SHADER_STAGE::INVALID;
			VkShaderCreateInfoEXT shaderCreateInfo = { .sType = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT, .flags = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT, .codeType = VkShaderCodeTypeEXT::VK_SHADER_CODE_TYPE_SPIRV_EXT, .pName = "main", .setLayoutCount = 1 };
			shaderCreateInfo.stage = static_cast<VkShaderStageFlagBits>(shaderpipe.shaderSources[i].shaderStage);
			shaderCreateInfo.nextStage = static_cast<VkShaderStageFlags>(nextShaderStage);
			shaderCreateInfo.pCode = shaderpipe.shaderSources[i].spirvCode.data();
			shaderCreateInfo.codeSize = shaderpipe.shaderSources[i].spirvCode.size() * sizeof(glm::uint32_t);
			shaderCreateInfo.pSetLayouts = &shaderpipe.descriptorLayout;
			sinfos.push_back(shaderCreateInfo);
			shaderpipe.shaderStages.push_back(shaderpipe.shaderSources[i].shaderStage);
		}
		
		shaderpipe.shaderObjects.resize(shaderpipe.shaderSources.size());
		vkCreateShadersEXTXY2D(vkdevice.logical, static_cast<uint32_t>(sinfos.size()), sinfos.data(), VK_NULL_HANDLE, shaderpipe.shaderObjects.data());
		shaderpipe.stagingBuffer = xy2d_gpualloc_create(vkdevice, XY2D_GPUALLOC_TYPE::STAGING, { stageBufferSize, 0U });
		return shaderpipe;
	}
#endif