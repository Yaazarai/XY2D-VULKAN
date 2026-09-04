#pragma once
#ifndef __XY2D_SHADER
#define __XY2D_SHADER
	#include "./xy2d_engine.hpp"
	
	namespace XY2D_NAMESPACE {
		enum class XY2D_SHADERSTAGE {
			INVALID = 0x00000000,
			VERTEX = VK_SHADER_STAGE_VERTEX_BIT,
			FRAGMENT = VK_SHADER_STAGE_FRAGMENT_BIT,
			COMPUTE = VK_SHADER_STAGE_COMPUTE_BIT,
		};
		
		enum class XY2D_DESCRIPTORS {
			UBO     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			SSBO    = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			IMAGE2D = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
			SAMPLER = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		};
		
		class xy2d_shader {
		public:
			XY2D_SHADERSTAGE shaderStage;
			std::vector<XY2D_DESCRIPTORS> descriptors;
			std::vector<glm::uint32_t> spirvCode;
			std::string filePath;
			
			xy2d_shader() = default;
			
			xy2d_shader(std::string filePath, XY2D_SHADERSTAGE shaderStage, std::vector<XY2D_DESCRIPTORS> descriptors)
			: filePath(filePath), shaderStage(shaderStage), descriptors(descriptors) {
				std::ifstream file(filePath, std::ios::ate | std::ios::binary);
				this->shaderStage = (file.is_open())? this->shaderStage : XY2D_SHADERSTAGE::INVALID;
				if (this->shaderStage == XY2D_SHADERSTAGE::INVALID) return;
				spirvCode.resize(static_cast<size_t>(file.tellg()) / sizeof(glm::uint32_t));
				file.seekg(0).read(reinterpret_cast<char*>(spirvCode.data()), spirvCode.size() * sizeof(glm::uint32_t));
			}
		};
	}
#endif