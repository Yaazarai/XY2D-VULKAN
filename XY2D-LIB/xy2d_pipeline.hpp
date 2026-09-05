#pragma once
#ifndef __XY2D_PIPELINE
#define __XY2D_PIPELINE
	#include "./xy2d_engine.hpp"
	
	namespace XY2D_NAMESPACE {
		class xy2d_pipeline : xy2d_disposable {
		public:
			xy2d_device& vkdevice;
			
			VkDescriptorSetLayout descriptorLayout;
			VkPipelineLayout pipelineLayout;
			std::vector<xy2d_shader> shaderSources;
			std::vector<VkShaderEXT> shaderObjects;
			std::vector<XY2D_SHADERSTAGE> shaderStages;
			VkResult initialized = VK_ERROR_INITIALIZATION_FAILED;
			
			xy2d_pipeline operator=(const xy2d_pipeline&) = delete;
			xy2d_pipeline(const xy2d_pipeline&) = delete;
			~xy2d_pipeline() { this->Dispose(); }
			
			xy2d_pipeline(xy2d_device& vkdevice, std::vector<xy2d_shader> shaderSources) : vkdevice(vkdevice), shaderSources(shaderSources), descriptorLayout(VK_NULL_HANDLE) {
				onDispose.hook(xy2d_callback<>([this]() {
					for(VkShaderEXT shaderObject : this->shaderObjects)
						vkDestroyShaderEXTXY2D(this->vkdevice.logicalDevice, shaderObject, VK_NULL_HANDLE);
					if (this->descriptorLayout != VK_NULL_HANDLE) vkDestroyDescriptorSetLayout(this->vkdevice.logicalDevice, this->descriptorLayout, VK_NULL_HANDLE);
				}));
				initialized = Initialize();
			}
			
			VkResult Initialize() {
				std::vector<VkDescriptorSetLayoutBinding> pbindings;
				for(glm::uint32_t bindings = 0, i = 0; i < static_cast<glm::uint32_t>(shaderSources.size()); i++)
					for(glm::uint32_t j = 0; j < static_cast<glm::uint32_t>(shaderSources[i].descriptors.size()); j++)
						pbindings.push_back({ bindings++, static_cast<VkDescriptorType>(shaderSources[i].descriptors[j]), 1U, static_cast<VkShaderStageFlags>(shaderSources[i].shaderStage) });
				
				VkDescriptorSetLayoutCreateInfo descriptorCreateInfo = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR };
				descriptorCreateInfo.bindingCount = static_cast<uint32_t>(pbindings.size());
				descriptorCreateInfo.pBindings = pbindings.data();
				vkCreateDescriptorSetLayout(vkdevice.logicalDevice, &descriptorCreateInfo, VK_NULL_HANDLE, &descriptorLayout);
				
				VkPipelineLayoutCreateInfo pipelineLayoutInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, .pPushConstantRanges = 0U };
				pipelineLayoutInfo.setLayoutCount = (descriptorLayout != VK_NULL_HANDLE)? 1U : 0U;
				pipelineLayoutInfo.pSetLayouts = &descriptorLayout;
				vkCreatePipelineLayout(vkdevice.logicalDevice, &pipelineLayoutInfo, VK_NULL_HANDLE, &pipelineLayout);
				
				std::vector<VkShaderCreateInfoEXT> sinfos;
				for(size_t i = 0; i < shaderSources.size(); i++) {
					XY2D_SHADERSTAGE nextShaderStage = (i < shaderSources.size() - 1)? shaderSources[i + 1].shaderStage : XY2D_SHADERSTAGE::INVALID;
					VkShaderCreateInfoEXT shaderCreateInfo = { .sType = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT, .flags = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT, .codeType = VkShaderCodeTypeEXT::VK_SHADER_CODE_TYPE_SPIRV_EXT, .pName = "main", .setLayoutCount = 1 };
					shaderCreateInfo.stage = static_cast<VkShaderStageFlagBits>(shaderSources[i].shaderStage);
					shaderCreateInfo.nextStage = static_cast<VkShaderStageFlags>(nextShaderStage);
					shaderCreateInfo.pCode = shaderSources[i].spirvCode.data();
					shaderCreateInfo.codeSize = shaderSources[i].spirvCode.size() * sizeof(glm::uint32_t);
					shaderCreateInfo.pSetLayouts = &descriptorLayout;
					sinfos.push_back(shaderCreateInfo);
					shaderStages.push_back(shaderSources[i].shaderStage);
				}
				
				shaderObjects.resize(shaderSources.size());
				return vkCreateShadersEXTXY2D(vkdevice.logicalDevice, static_cast<uint32_t>(sinfos.size()), sinfos.data(), VK_NULL_HANDLE, shaderObjects.data());
			}
		};
	}
#endif