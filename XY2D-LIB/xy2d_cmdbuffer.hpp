#pragma once
#ifndef __XY2D_CMDBUFFER
#define __XY2D_CMDBUFFER
	#include ".\xy2d_engine.hpp"
	
	enum class XY2D_ACCESSSTAGES : VkAccessFlags2 {
		TRANSFER = VK_ACCESS_2_TRANSFER_READ_BIT | VK_ACCESS_2_TRANSFER_WRITE_BIT,
		COMPUTE = VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT | VK_ACCESS_2_SHADER_SAMPLED_READ_BIT,
		RENDER = VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT | VK_ACCESS_2_SHADER_SAMPLED_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
		PRESENT = VK_ACCESS_2_NONE,
	};
	
	enum class XY2D_PIPELINESTAGES : VkPipelineStageFlags2 {
		TRANSFER = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
		COMPUTE  = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
		REMDER   = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
		PRESENT  = VK_PIPELINE_STAGE_2_NONE,
	};
	
	struct xy2d_vertex {
		glm::vec2 xy;
		glm::vec2 txcoord;
		glm::uint color;
	};
	
	const VkVertexInputBindingDescription2EXT xy2d_vertex_bindings() {
		return { .sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT, .binding = 0, .stride = sizeof(xy2d_vertex), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX, .divisor = 1, .pNext = VK_NULL_HANDLE };
	}
	
	const std::vector<VkVertexInputAttributeDescription2EXT> xy2d_vertex_attributes() {
		return {
			{ .sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT, .pNext = VK_NULL_HANDLE, .binding = 0, .location = 0, .format = VK_FORMAT_R32G32_SFLOAT, .offset = offsetof(xy2d_vertex, xy) },
			{ .sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT, .pNext = VK_NULL_HANDLE, .binding = 0, .location = 1, .format = VK_FORMAT_R32G32_SFLOAT, .offset = offsetof(xy2d_vertex, txcoord) },
			{ .sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT, .pNext = VK_NULL_HANDLE, .binding = 0, .location = 2, .format = VK_FORMAT_R32_UINT, .offset = offsetof(xy2d_vertex, color) },
		};
	}
	
	struct xy2d_camera {
		glm::mat4 projection  = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
		glm::vec4 xrotation = { 0.0f, 0.0f, 0.0f, 0.0f };
		glm::vec4 yrotation = { 0.0f, 0.0f, 0.0f, 0.0f };
		glm::vec2 position = { 0.0f, 0.0f };
		glm::vec2 scale = { 0.0f, 0.0f };
		glm::vec2 origin = { 0.0f, 0.0f };
	};
	
	xy2d_camera xy2d_camera_orthographic(glm::vec2 dimensions, glm::vec2 position, glm::vec2 scale, glm::vec2 origin, glm::float32 theta = 0.0, glm::float32 znear = 1.0, glm::float32 zfar = -1.0) {
		glm::mat4 prj = glm::ortho(0.0f, dimensions.x, 0.0f, dimensions.y, znear, zfar);
		glm::vec4 xrot = glm::vec4(glm::cos(theta), -glm::sin(theta), 0.0, 0.0);
		glm::vec4 yrot = glm::vec4(glm::sin(theta), glm::cos(theta), 0.0, 0.0);
		return { .projection = prj, .xrotation = xrot, .yrotation = yrot, .position = position, .scale = scale, .origin = origin };
	}
	
	struct xy2d_renderstate {
		VkBool32 blending = VK_TRUE;
		VkBool32 clearOnLoad = VK_TRUE;
		VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
		VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
		VkPrimitiveTopology vertexTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	};
	
	struct xy2d_cmdbuffer {
		xy2d_device* vkdevice = VK_NULL_HANDLE;
		VkQueryPool timestampQueryPool = VK_NULL_HANDLE;
		VkCommandBuffer cmdbuffer = VK_NULL_HANDLE;
		
		uint32_t timestampQueryIndex = 0U;
		VkAccessFlags2 currentAccessFlags = VK_ACCESS_2_NONE;
		VkPipelineStageFlags2 currentStageFlags = VK_PIPELINE_STAGE_2_NONE;
	};
	
	void xy2d_cmdbuffer_barrier(xy2d_cmdbuffer& cmdbuffer, XY2D_PIPELINESTAGES destinationStage, XY2D_ACCESSSTAGES destinationAccessFlags, std::vector<xy2d_gpualloc*> imageTargets, XY2D_GPUALLOC_LAYOUT imageLayout = XY2D_GPUALLOC_LAYOUT::GENERAL) {
		std::vector<VkImageMemoryBarrier2> imageMemoryBarriers;
		
		for(xy2d_gpualloc* image : imageTargets) {
			VkImageMemoryBarrier2 imageBarrier = {};
			imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
			imageBarrier.srcAccessMask = static_cast<VkAccessFlags2>(cmdbuffer.currentAccessFlags);
			imageBarrier.dstAccessMask = static_cast<VkAccessFlags2>(destinationAccessFlags);
			imageBarrier.srcStageMask = static_cast<VkPipelineStageFlags2>(cmdbuffer.currentStageFlags);
			imageBarrier.dstStageMask = static_cast<VkPipelineStageFlags2>(destinationStage);
			imageBarrier.subresourceRange = { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1, };
			imageBarrier.image = image->image;
			imageBarrier.oldLayout = static_cast<VkImageLayout>(image->layout);
			imageBarrier.newLayout = static_cast<VkImageLayout>(imageLayout);
			
			if (image->layout == XY2D_GPUALLOC_LAYOUT::PRESENT_SRCKHR) {
				imageBarrier.srcAccessMask = VK_ACCESS_2_NONE;
				imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
			}
			image->layout = imageLayout;
			imageMemoryBarriers.push_back(imageBarrier);
		}
		
		VkMemoryBarrier2 memoryBarrier = { .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2 };
		memoryBarrier.srcStageMask = static_cast<VkPipelineStageFlags2>(cmdbuffer.currentStageFlags);
		memoryBarrier.dstStageMask = static_cast<VkPipelineStageFlags2>(destinationStage);
		memoryBarrier.srcAccessMask = static_cast<VkAccessFlags2>(cmdbuffer.currentAccessFlags);
		memoryBarrier.dstAccessMask = static_cast<VkAccessFlags2>(destinationAccessFlags);
		
		VkDependencyInfo dependencyInfo = { .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
		dependencyInfo.memoryBarrierCount = 1U;
		dependencyInfo.pMemoryBarriers = &memoryBarrier;
		dependencyInfo.imageMemoryBarrierCount = imageMemoryBarriers.size();
		dependencyInfo.pImageMemoryBarriers = imageMemoryBarriers.data();
		vkCmdPipelineBarrier2(cmdbuffer.cmdbuffer, &dependencyInfo);
		
		cmdbuffer.currentAccessFlags = static_cast<VkAccessFlags2>(destinationAccessFlags);
		cmdbuffer.currentStageFlags = static_cast<VkPipelineStageFlags2>(destinationStage);
	}
	
	std::vector<glm::float64_t> xy2d_cmdbuffer_timestamps_query(xy2d_cmdbuffer& cmdbuffer) {
		std::vector<glm::float64_t> frametimes;
		#if XY2D_VALIDATION
			std::vector<VkDeviceSize> timestamps(cmdbuffer.timestampQueryIndex);
			vkGetQueryPoolResults(cmdbuffer.vkdevice->logical, cmdbuffer.timestampQueryPool, 0, timestamps.size(), timestamps.size() * sizeof(VkDeviceSize), timestamps.data(), sizeof(VkDeviceSize), VK_QUERY_RESULT_64_BIT);
			for(int i = 0; i < cmdbuffer.timestampQueryIndex; i ++)
				frametimes.push_back(double(timestamps[i]) * (double(cmdbuffer.vkdevice->properties.properties.limits.timestampPeriod) / 1000000.0));
		#endif
		return frametimes;
	}
	
	void xy2d_cmdbuffer_timestamps_inject(xy2d_cmdbuffer& cmdbuffer) {
		#if XY2D_VALIDATION
			vkCmdWriteTimestamp(cmdbuffer.cmdbuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, cmdbuffer.timestampQueryPool, cmdbuffer.timestampQueryIndex ++);
		#endif
	}
	
	void xy2d_shader_bind_uniforms(xy2d_cmdbuffer& cmdbuffer, xy2d_shaderpipe& pipeline, std::vector<xy2d_gpualloc*> uniforms) {
		for(VkDeviceSize binding = 0; binding < uniforms.size(); binding++) {
			VkDescriptorBufferInfo bufferDescriptor = { .buffer = uniforms[binding]->buffer, .offset = 0U, .range = VK_WHOLE_SIZE };
			VkDescriptorImageInfo imageDescriptor = { .sampler = uniforms[binding]->sampler, .imageView = uniforms[binding]->view, .imageLayout = static_cast<VkImageLayout>(uniforms[binding]->layout) };
			
			VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			if (uniforms[binding]->type == XY2D_GPUALLOC_TYPE::ATTACHEMENT) descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			if (uniforms[binding]->type == XY2D_GPUALLOC_TYPE::STORAGE) descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			
			VkWriteDescriptorSet writeDescriptorSet = { .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, .pBufferInfo = &bufferDescriptor, .pImageInfo = &imageDescriptor, .dstBinding = static_cast<uint32_t>(binding), .descriptorType = descriptorType, .descriptorCount = 1U };
			vkCmdPushDescriptorSet(cmdbuffer.cmdbuffer, (pipeline.shaderStages[0] == XY2D_SHADER_STAGE::COMPUTE)? VK_PIPELINE_BIND_POINT_COMPUTE : VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipelineLayout, 0U, 1U, &writeDescriptorSet);
		}
	}
	
	void xy2d_render_begin(xy2d_cmdbuffer& cmdbuffer, xy2d_shaderpipe& pipeline, std::vector<xy2d_gpualloc*> imageTargets, glm::ivec4 xywh, xy2d_renderstate renderState = xy2d_renderstate()) {
		xy2d_cmdbuffer_barrier(cmdbuffer, XY2D_PIPELINESTAGES::REMDER, XY2D_ACCESSSTAGES::RENDER, imageTargets, XY2D_GPUALLOC_LAYOUT::GENERAL);
		
		VkRect2D renderArea = { .offset = { xywh[0], xywh[1] }, .extent = { static_cast<uint32_t>(xywh[2]), static_cast<uint32_t>(xywh[3]) } };
		VkViewport dynamicViewPort = { .minDepth = 0.0f, .maxDepth = 1.0f, .width = static_cast<float>(xywh[2]), .height = static_cast<float>(xywh[3]), };
		std::vector<VkBool32> colorWrites;
		std::vector<VkBool32> colorBlending;
		std::vector<VkColorBlendEquationEXT> colorBlendEqs;
		std::vector<VkRenderingAttachmentInfoKHR> colorAttachmentInfos;
		std::vector<VkColorComponentFlags> rgbaWriteMasks;
		
		for(xy2d_gpualloc* image : imageTargets) {
			VkRenderingAttachmentInfoKHR colorAttachmentInfo = { .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR, .imageView = image->view, .imageLayout = (VkImageLayout) image->layout, .clearValue = renderState.clearColor, .loadOp = ((renderState.clearOnLoad)?VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE), .storeOp = VK_ATTACHMENT_STORE_OP_STORE };
			VkColorBlendEquationEXT blendingEquation = { .colorBlendOp = VK_BLEND_OP_ADD, .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA, .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, .alphaBlendOp = VK_BLEND_OP_ADD, .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE, .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA };
			colorAttachmentInfos.push_back(colorAttachmentInfo);
			colorBlendEqs.push_back(blendingEquation);
			colorBlending.push_back(renderState.blending);
			rgbaWriteMasks.push_back(image->rgbaWriteMask);
			colorWrites.push_back(VK_TRUE);
		}
		
		VkRenderingInfoKHR dynamicRenderInfo = { .sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR, .pColorAttachments = colorAttachmentInfos.data(), .colorAttachmentCount = static_cast<uint32_t>(colorAttachmentInfos.size()), .renderArea = renderArea, .layerCount = 1, };
		vkCmdBeginRendering(cmdbuffer.cmdbuffer, &dynamicRenderInfo);
		vkCmdBindShadersEXTXY2D(cmdbuffer.cmdbuffer, pipeline.shaderStages.size(), reinterpret_cast<VkShaderStageFlagBits*>(pipeline.shaderStages.data()), pipeline.shaderObjects.data());
		vkCmdSetViewportWithCount(cmdbuffer.cmdbuffer, 1U, &dynamicViewPort);
		vkCmdSetScissorWithCount(cmdbuffer.cmdbuffer, 1U, &renderArea);
		vkCmdSetColorWriteEnableEXTXY2D(cmdbuffer.cmdbuffer, colorWrites.size(), colorWrites.data());
		vkCmdSetColorBlendEnableEXTXY2D(cmdbuffer.cmdbuffer, 0, colorBlending.size(), colorBlending.data());
		vkCmdSetColorBlendEquationEXTXY2D(cmdbuffer.cmdbuffer, 0, colorBlendEqs.size(), colorBlendEqs.data());
		vkCmdSetColorWriteMaskEXTXY2D(cmdbuffer.cmdbuffer, 0, rgbaWriteMasks.size(), rgbaWriteMasks.data());
		vkCmdSetRasterizerDiscardEnable(cmdbuffer.cmdbuffer, VK_FALSE);
		vkCmdSetDepthWriteEnable(cmdbuffer.cmdbuffer, VK_FALSE);
		vkCmdSetDepthTestEnable(cmdbuffer.cmdbuffer, VK_FALSE);
		vkCmdSetDepthClampEnableEXTXY2D(cmdbuffer.cmdbuffer, VK_FALSE);
		vkCmdSetDepthBiasEnable(cmdbuffer.cmdbuffer, VK_FALSE);
		vkCmdSetStencilTestEnable(cmdbuffer.cmdbuffer, VK_FALSE);
		vkCmdSetAlphaToCoverageEnableEXTXY2D(cmdbuffer.cmdbuffer, VK_FALSE);
		vkCmdSetPrimitiveRestartEnable(cmdbuffer.cmdbuffer, VK_FALSE);
		
		VkVertexInputBindingDescription2EXT vertexBindingDescr = xy2d_vertex_bindings();
		const std::vector<VkVertexInputAttributeDescription2EXT> vertexAttributeDescr = xy2d_vertex_attributes();
		vkCmdSetVertexInputEXTXY2D(cmdbuffer.cmdbuffer, 1U, &vertexBindingDescr, vertexAttributeDescr.size(), vertexAttributeDescr.data());
		vkCmdSetPolygonModeEXTXY2D(cmdbuffer.cmdbuffer, renderState.polygonMode);
		vkCmdSetPrimitiveTopology(cmdbuffer.cmdbuffer, renderState.vertexTopology);
		
		VkSampleMask sampleMask = VK_SAMPLE_COUNT_1_BIT;
		vkCmdSetRasterizationSamplesEXTXY2D(cmdbuffer.cmdbuffer, VK_SAMPLE_COUNT_1_BIT);
		vkCmdSetSampleMaskEXTXY2D(cmdbuffer.cmdbuffer, VK_SAMPLE_COUNT_1_BIT, &sampleMask);
		vkCmdSetCullMode(cmdbuffer.cmdbuffer, VK_CULL_MODE_NONE);
		vkCmdSetFrontFace(cmdbuffer.cmdbuffer, VK_FRONT_FACE_CLOCKWISE);
	}
	
	void xy2d_render_end(xy2d_cmdbuffer& cmdbuffer) {
		vkCmdEndRendering(cmdbuffer.cmdbuffer);
	}
	
	void xy2d_render_bind_vertex_buffer(xy2d_cmdbuffer& cmdbuffer, xy2d_gpualloc* vertexBuffer, uint32_t firstBinding = 0U) {
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(cmdbuffer.cmdbuffer, firstBinding, 1, &vertexBuffer->buffer, offsets);
	}
	
	void xy2d_render_draw_vertex_buffer(xy2d_cmdbuffer& cmdbuffer, uint32_t vertexCount, uint32_t firstVertex, uint32_t instanceCount) {
		vkCmdDraw(cmdbuffer.cmdbuffer, vertexCount, instanceCount, firstVertex, 0);
	}
	
	void xy2d_compute_begin(xy2d_cmdbuffer& cmdbuffer, xy2d_shaderpipe& pipeline) {
		xy2d_cmdbuffer_barrier(cmdbuffer, XY2D_PIPELINESTAGES::COMPUTE, XY2D_ACCESSSTAGES::COMPUTE, {});
		
		vkCmdBindShadersEXTXY2D(cmdbuffer.cmdbuffer, pipeline.shaderStages.size(), reinterpret_cast<VkShaderStageFlagBits*>(pipeline.shaderStages.data()), pipeline.shaderObjects.data());
	}
	
	void xy2d_compute_end(xy2d_cmdbuffer& cmdbuffer, glm::ivec3 groupSize) {
		vkCmdDispatch(cmdbuffer.cmdbuffer, groupSize.x, groupSize.y, groupSize.z);
	}
	
	void xy2d_transfer_buffer(xy2d_cmdbuffer& cmdbuffer, void* dataPointer, size_t sizeOfData, size_t offsetOfData, xy2d_gpualloc* stageBuffer, xy2d_gpualloc* buffer, uint32_t dstOffset = 0U) {
		xy2d_cmdbuffer_barrier(cmdbuffer, XY2D_PIPELINESTAGES::TRANSFER, XY2D_ACCESSSTAGES::TRANSFER, {});
		void* stagedOffset = static_cast<int8_t*>(stageBuffer->description.pMappedData) + offsetOfData;
		SDL_memcpy(stagedOffset, dataPointer, sizeOfData);
		
		VkBufferCopy copyRegion { .srcOffset = offsetOfData, .dstOffset = dstOffset, .size = sizeOfData };
		vkCmdCopyBuffer(cmdbuffer.cmdbuffer, stageBuffer->buffer, buffer->buffer, 1, &copyRegion);
	}
	
	void xy2d_transfer_image(xy2d_cmdbuffer& cmdbuffer, void* dataPointer, uint32_t sizeOfData, uint32_t offsetOfData, uint32_t xpos, uint32_t ypos, uint32_t width, uint32_t height, xy2d_gpualloc* stageBuffer, xy2d_gpualloc* image) {
		xy2d_cmdbuffer_barrier(cmdbuffer, XY2D_PIPELINESTAGES::TRANSFER, XY2D_ACCESSSTAGES::TRANSFER, { image });
		void* stagedOffset = static_cast<int8_t*>(stageBuffer->description.pMappedData) + offsetOfData;
		SDL_memcpy(stagedOffset, dataPointer, sizeOfData);
		
		VkBufferImageCopy copyRegion = {};
		copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegion.imageSubresource.layerCount = 1;
		copyRegion.imageOffset = { .x = static_cast<int32_t>(xpos), .y = static_cast<int32_t>(ypos), .z = 0 };
		copyRegion.imageExtent = { .width = width, .height = height, .depth = 1U };;
		copyRegion.bufferOffset = static_cast<VkDeviceSize>(offsetOfData);
		vkCmdCopyBufferToImage(cmdbuffer.cmdbuffer, stageBuffer->buffer, image->image, (VkImageLayout) image->layout, 1U, &copyRegion);
	}
#endif

/*
	template<typename T>
	void xy2d_transfer_bufferlist(xy2d_cmdbuffer& cmdbuffer, std::vector<T> dataList, uint32_t sizeOfObject, uint32_t offsetOfData, xy2d_gpualloc* stageBuffer, xy2d_gpualloc* buffer) {
		xy2d_cmdbuffer_barrier(cmdbuffer, XY2D_PIPELINESTAGES::TRANSFER, XY2D_ACCESSSTAGES::TRANSFER, {});
		
		for(size_t i = 0; i < dataList.size(); i++) {
			void* stagedOffset = static_cast<int8_t*>(stageBuffer->description.pMappedData) + offsetOfData + (i * sizeOfObject);
			SDL_memcpy(stagedOffset, dataList[i], sizeOfObject);
		}
		
		VkDeviceSize sizeOfData = static_cast<VkDeviceSize>(dataList.size() * sizeOfObject);
		VkBufferCopy copyRegion { .srcOffset = offsetOfData, .dstOffset = 0, .size = sizeOfData };
		vkCmdCopyBuffer(cmdbuffer.cmdbuffer, stageBuffer->buffer, buffer->buffer, 1, &copyRegion);
	}
	
	template<typename T>
	void xy2d_transfer_buffer(xy2d_cmdbuffer& cmdbuffer, T& dataPointer, uint32_t offsetOfData, xy2d_gpualloc* stageBuffer, xy2d_gpualloc* buffer) {
		xy2d_transfer_buffer_ext(cmdbuffer, &dataPointer, sizeof(dataPointer), offsetOfData, stageBuffer, buffer);
	}
	
	template<typename T>
	void xy2d_transfer_image(xy2d_cmdbuffer& cmdbuffer, T& dataPointer, uint32_t offsetOfData, uint32_t xpos, uint32_t ypos, uint32_t width, uint32_t height, xy2d_gpualloc* stageBuffer, xy2d_gpualloc* image) {
		xy2d_transfer_image_ext(cmdbuffer, &dataPointer, sizeof(dataPointer), offsetOfData, xpos, ypos, width, height, stageBuffer, image);
	}
*/