#pragma once
#ifndef __XY2D_CMDBUFFER
#define __XY2D_CMDBUFFER
	#include ".\xy2d_engine.hpp"
	
	namespace XY2D_NAMESPACE {
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
		public:
			glm::vec3 xyz;
			glm::vec2 txcoord;
			
			static const VkVertexInputBindingDescription2EXT GetBindingDescription() {
				return { .sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT, .binding = 0, .stride = sizeof(xy2d_vertex), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX, .divisor = 1, .pNext = VK_NULL_HANDLE };
			}
			
			static const std::vector<VkVertexInputAttributeDescription2EXT> GetAttributeDescriptions() {
				return {
					{ .sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT, .pNext = VK_NULL_HANDLE, .binding = 0, .location = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(xy2d_vertex, xyz) },
					{ .sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT, .pNext = VK_NULL_HANDLE, .binding = 0, .location = 1, .format = VK_FORMAT_R32G32_SFLOAT, .offset = offsetof(xy2d_vertex, txcoord) },
				};
			}
		};
		
		struct xy2d_renderstate {
		public:
			VkBool32 blending;
			VkBool32 clearOnLoad;
			VkClearValue clearColor;
			VkPolygonMode polygonMode;
			VkPrimitiveTopology vertexTopology;
			
			xy2d_renderstate(VkBool32 blending = VK_TRUE, VkBool32 clearOnLoad = VK_TRUE, VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f }, VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL, VkPrimitiveTopology vertexTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
			: blending(blending), clearOnLoad(clearOnLoad), clearColor(clearColor), polygonMode(polygonMode), vertexTopology(vertexTopology) {}
		};
		
		class xy2d_cmdbuffer {
		public:
			xy2d_device& vkdevice;
			xy2d_image& swapChainIamge;
			VkCommandBuffer& cmdbuffer;
			VkQueryPool& timestampQueryPool;
			uint32_t timestampQueryIndex = 0U;
			
			VkAccessFlags2 currentAccessFlags = VK_ACCESS_2_NONE;
			VkPipelineStageFlags2 currentStageFlags = VK_PIPELINE_STAGE_2_NONE;
			
			xy2d_cmdbuffer(xy2d_device& vkdevice, xy2d_image& swapChainIamge, VkCommandBuffer& cmdbuffer, VkQueryPool& timestampQueryPool)
				: vkdevice(vkdevice), swapChainIamge(swapChainIamge), cmdbuffer(cmdbuffer), timestampQueryPool(timestampQueryPool) {}
			
			void ExecutionBarrier(XY2D_PIPELINESTAGES destinationStage, XY2D_ACCESSSTAGES destinationAccessFlags, std::vector<xy2d_image*> imageTargets, XY2D_IMAGELAYOUT imageLayout = XY2D_IMAGELAYOUT::GENERAL) {
				std::vector<VkImageMemoryBarrier2> imageMemoryBarriers;
				
				for(xy2d_image* image : imageTargets) {
					VkImageMemoryBarrier2 imageBarrier = {};
					imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
					imageBarrier.srcAccessMask = static_cast<VkAccessFlags2>(currentAccessFlags);
					imageBarrier.dstAccessMask = static_cast<VkAccessFlags2>(destinationAccessFlags);
					imageBarrier.srcStageMask = static_cast<VkPipelineStageFlags2>(currentStageFlags);
					imageBarrier.dstStageMask = static_cast<VkPipelineStageFlags2>(destinationStage);
					imageBarrier.srcQueueFamilyIndex = vkdevice.queueFamily.renderFamily;
					imageBarrier.dstQueueFamilyIndex = vkdevice.queueFamily.renderFamily;
					imageBarrier.subresourceRange = { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1, };
					imageBarrier.image = image->imageSource;
					imageBarrier.oldLayout = static_cast<VkImageLayout>(image->imageLayout);
					imageBarrier.newLayout = static_cast<VkImageLayout>(imageLayout);
					
					if (image->imageLayout == XY2D_IMAGELAYOUT::PRESENT_SRCKHR) {
						imageBarrier.srcAccessMask = VK_ACCESS_2_NONE;
						imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
					}
					
					image->imageLayout = imageLayout;
					imageMemoryBarriers.push_back(imageBarrier);
				}
				
				VkMemoryBarrier2 memoryBarrier = { .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2 };
				memoryBarrier.srcStageMask = static_cast<VkPipelineStageFlags2>(currentStageFlags);
				memoryBarrier.dstStageMask = static_cast<VkPipelineStageFlags2>(destinationStage);
				memoryBarrier.srcAccessMask = static_cast<VkAccessFlags2>(currentAccessFlags);
				memoryBarrier.dstAccessMask = static_cast<VkAccessFlags2>(destinationAccessFlags);
				
				VkDependencyInfo dependencyInfo = { .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
				dependencyInfo.memoryBarrierCount = 1U;
				dependencyInfo.pMemoryBarriers = &memoryBarrier;
				dependencyInfo.imageMemoryBarrierCount = imageMemoryBarriers.size();
				dependencyInfo.pImageMemoryBarriers = imageMemoryBarriers.data();
				vkCmdPipelineBarrier2(cmdbuffer, &dependencyInfo);
				currentAccessFlags = static_cast<VkAccessFlags2>(destinationAccessFlags);
				currentStageFlags = static_cast<VkPipelineStageFlags2>(destinationStage);
			}
			
			std::vector<glm::float64_t> QueryTimeStamps() {
				std::vector<glm::float64_t> frametimes;
				#if XY2D_VALIDATION
					std::vector<VkDeviceSize> timestamps(timestampQueryIndex);
					
					vkGetQueryPoolResults(vkdevice.logicalDevice, timestampQueryPool, 0, timestamps.size(), timestamps.size() * sizeof(VkDeviceSize), timestamps.data(), sizeof(VkDeviceSize), VK_QUERY_RESULT_64_BIT);
					
					for(int i = 0; i < timestampQueryIndex; i ++) {
						double ts = double(timestamps[i]) * (double(vkdevice.deviceProperties.properties.limits.timestampPeriod) / 1000000.0);
						frametimes.push_back(ts);
					}
				#endif
				return frametimes;
			}
			
			void InjectTimestamp() {
				#if XY2D_VALIDATION
					vkCmdWriteTimestamp(cmdbuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, timestampQueryPool, timestampQueryIndex ++);
				#endif
			}
			
			void RenderBegin(xy2d_pipeline* pipeline, std::vector<xy2d_image*> imageTargets, glm::ivec4 xywh, xy2d_renderstate renderState = xy2d_renderstate()) {
				ExecutionBarrier(XY2D_PIPELINESTAGES::REMDER, XY2D_ACCESSSTAGES::RENDER, imageTargets, XY2D_IMAGELAYOUT::GENERAL);
				
				VkRect2D renderArea = { .offset = { xywh[0], xywh[1] }, .extent = { static_cast<uint32_t>(xywh[2]), static_cast<uint32_t>(xywh[3]) } };
				VkViewport dynamicViewPort = { .minDepth = 0.0f, .maxDepth = 1.0f, .width = static_cast<float>(xywh[2]), .height = static_cast<float>(xywh[3]), };
				std::vector<VkBool32> colorWrites;
				std::vector<VkBool32> colorBlending;
				std::vector<VkColorBlendEquationEXT> colorBlendEqs;
				std::vector<VkRenderingAttachmentInfoKHR> colorAttachmentInfos;
				std::vector<VkColorComponentFlags> colorWriteMasks;
				
				for(xy2d_image* image : imageTargets) {
					VkRenderingAttachmentInfoKHR colorAttachmentInfo = { .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR, .imageView = image->imageView, .imageLayout = (VkImageLayout) image->imageLayout, .clearValue = renderState.clearColor, .loadOp = ((renderState.clearOnLoad)?VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE), .storeOp = VK_ATTACHMENT_STORE_OP_STORE };
					VkColorBlendEquationEXT blendingEquation = { .colorBlendOp = VK_BLEND_OP_ADD, .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA, .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, .alphaBlendOp = VK_BLEND_OP_ADD, .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE, .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA };
					colorAttachmentInfos.push_back(colorAttachmentInfo);
					colorBlendEqs.push_back(blendingEquation);
					colorBlending.push_back(renderState.blending);
					colorWriteMasks.push_back(image->colorWriteMask);
					colorWrites.push_back(VK_TRUE);
				}
				
				VkRenderingInfoKHR dynamicRenderInfo = { .sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR, .pColorAttachments = colorAttachmentInfos.data(), .colorAttachmentCount = static_cast<uint32_t>(colorAttachmentInfos.size()), .renderArea = renderArea, .layerCount = 1, };
				vkCmdBeginRenderingKHRXY2D(cmdbuffer, &dynamicRenderInfo);
				vkCmdBindShadersEXTXY2D(cmdbuffer, pipeline->shaderStages.size(), reinterpret_cast<VkShaderStageFlagBits*>(pipeline->shaderStages.data()), pipeline->shaderObjects.data());
				vkCmdSetViewportWithCountEXTXY2D(cmdbuffer, 1U, &dynamicViewPort);
				vkCmdSetScissorWithCountEXTXY2D(cmdbuffer, 1U, &renderArea);
				vkCmdSetColorWriteEnableEXTXY2D(cmdbuffer, colorWrites.size(), colorWrites.data());
				vkCmdSetRasterizerDiscardEnableEXTXY2D(cmdbuffer, VK_FALSE);
				vkCmdSetColorBlendEnableEXTXY2D(cmdbuffer, 0, colorBlending.size(), colorBlending.data());
				vkCmdSetColorBlendEquationEXTXY2D(cmdbuffer, 0, colorBlendEqs.size(), colorBlendEqs.data());
				vkCmdSetColorWriteMaskEXTXY2D(cmdbuffer, 0, colorWriteMasks.size(), colorWriteMasks.data());
				vkCmdSetDepthWriteEnableEXTXY2D(cmdbuffer, VK_FALSE);
				vkCmdSetDepthTestEnableEXTXY2D(cmdbuffer, VK_FALSE);
				vkCmdSetDepthClampEnableEXTXY2D(cmdbuffer, VK_FALSE);
				vkCmdSetDepthBiasEnableEXTXY2D(cmdbuffer, VK_FALSE);
				vkCmdSetStencilTestEnableEXTXY2D(cmdbuffer, VK_FALSE);
				
				VkVertexInputBindingDescription2EXT vertexBindingDescr = xy2d_vertex::GetBindingDescription();
				const std::vector<VkVertexInputAttributeDescription2EXT> vertexAttributeDescr = xy2d_vertex::GetAttributeDescriptions();
				vkCmdSetVertexInputEXTXY2D(cmdbuffer, 1U, &vertexBindingDescr, vertexAttributeDescr.size(), vertexAttributeDescr.data());
				vkCmdSetPolygonModeEXTXY2D(cmdbuffer, renderState.polygonMode);
				vkCmdSetPrimitiveTopologyEXTXY2D(cmdbuffer, renderState.vertexTopology);
				
				VkSampleMask sampleMask = VK_SAMPLE_COUNT_1_BIT;
				vkCmdSetRasterizationSamplesEXTXY2D(cmdbuffer, VK_SAMPLE_COUNT_1_BIT);
				vkCmdSetSampleMaskEXTXY2D(cmdbuffer, VK_SAMPLE_COUNT_1_BIT, &sampleMask);
				vkCmdSetAlphaToCoverageEnableEXTXY2D(cmdbuffer, VK_FALSE);
				vkCmdSetPrimitiveRestartEnableEXTXY2D(cmdbuffer, VK_FALSE);
				vkCmdSetCullModeEXTXY2D(cmdbuffer, VK_CULL_MODE_NONE);
				vkCmdSetFrontFaceEXTXY2D(cmdbuffer, VK_FRONT_FACE_CLOCKWISE);
			}
			
			void RenderEnd() {
				vkCmdEndRenderingKHRXY2D(cmdbuffer);
			}
			
			void RenderPushBuffer(xy2d_pipeline* pipeline, xy2d_buffer* uniformBuffer, VkDeviceSize binding) {
				VkDescriptorBufferInfo bufferDescriptor = { .buffer = uniformBuffer->buffer, .offset = 0U, .range = VK_WHOLE_SIZE };
				VkWriteDescriptorSet bufferDescriptorSet = { .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, .pBufferInfo = &bufferDescriptor, .dstBinding = static_cast<uint32_t>(binding), .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 1U };
				vkCmdPushDescriptorSetKHRXY2D(cmdbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipelineLayout, 0U, 1U, &bufferDescriptorSet);
			}
			
			void RenderPushImageSampler(xy2d_pipeline* pipeline, xy2d_image* uniformImage, VkDeviceSize binding) {
				VkDescriptorImageInfo imageDescriptor = { .sampler = uniformImage->imageSampler, .imageView = uniformImage->imageView, .imageLayout = (VkImageLayout) uniformImage->imageLayout };
				VkWriteDescriptorSet imageDescriptorSet = { .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, .pImageInfo = &imageDescriptor, .dstBinding = static_cast<uint32_t>(binding), .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 1U };
				vkCmdPushDescriptorSetKHRXY2D(cmdbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipelineLayout, 0U, 1U, &imageDescriptorSet);
			}
			
			void RenderPushImageStorage(xy2d_pipeline* pipeline, xy2d_image* uniformImage, VkDeviceSize binding) {
				VkDescriptorImageInfo imageDescriptor = { .sampler = uniformImage->imageSampler, .imageView = uniformImage->imageView, .imageLayout = (VkImageLayout) uniformImage->imageLayout };
				VkWriteDescriptorSet imageDescriptorSet = { .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, .pImageInfo = &imageDescriptor, .dstBinding = static_cast<uint32_t>(binding), .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, .descriptorCount = 1U };
				vkCmdPushDescriptorSetKHRXY2D(cmdbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipelineLayout, 0U, 1U, &imageDescriptorSet);
			}
			
			void RenderBindVertexBuffer(xy2d_buffer* vertexBuffer, uint32_t firstBinding = 0U) {
				VkDeviceSize offsets[] = { 0 };
				vkCmdBindVertexBuffers(cmdbuffer, firstBinding, 1, &vertexBuffer->buffer, offsets);
			}
			
			void RenderDrawVertices(uint32_t vertexCount, uint32_t firstVertex, uint32_t instanceCount) {
				vkCmdDraw(cmdbuffer, vertexCount, instanceCount, firstVertex, 0);
			}
			
			void ComputeBegin(xy2d_pipeline* pipeline) {
				ExecutionBarrier(XY2D_PIPELINESTAGES::COMPUTE, XY2D_ACCESSSTAGES::COMPUTE, {});
				vkCmdBindShadersEXTXY2D(cmdbuffer, pipeline->shaderStages.size(), reinterpret_cast<VkShaderStageFlagBits*>(pipeline->shaderStages.data()), pipeline->shaderObjects.data());
			}
			
			void ComputePushBuffer(xy2d_pipeline* pipeline, xy2d_buffer* uniformBuffer, VkDeviceSize binding) {
				VkDescriptorBufferInfo bufferDescriptor = { .buffer = uniformBuffer->buffer, .offset = 0U, .range = VK_WHOLE_SIZE };
				VkWriteDescriptorSet bufferDescriptorSet = { .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, .pBufferInfo = &bufferDescriptor, .dstSet = 0, .dstBinding = static_cast<uint32_t>(binding), .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 1U };
				vkCmdPushDescriptorSetKHRXY2D(cmdbuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->pipelineLayout, 0, 1, &bufferDescriptorSet);
			}
			
			void ComputePushImageSampler(xy2d_pipeline* pipeline, xy2d_image* uniformImage, VkDeviceSize binding) {
				VkDescriptorImageInfo imageDescriptor = { .sampler = uniformImage->imageSampler, .imageView = uniformImage->imageView, .imageLayout = (VkImageLayout) uniformImage->imageLayout };
				VkWriteDescriptorSet imageDescriptorSet = { .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, .pImageInfo = &imageDescriptor, .dstSet = 0, .dstBinding = static_cast<uint32_t>(binding), .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 1U };
				vkCmdPushDescriptorSetKHRXY2D(cmdbuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->pipelineLayout, 0, 1, &imageDescriptorSet);
			}
			
			void ComputePushImageStorage(xy2d_pipeline* pipeline, xy2d_image* uniformImage, VkDeviceSize binding) {
				VkDescriptorImageInfo imageDescriptor = { .sampler = uniformImage->imageSampler, .imageView = uniformImage->imageView, .imageLayout = (VkImageLayout) uniformImage->imageLayout };
				VkWriteDescriptorSet imageDescriptorSet = { .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, .pImageInfo = &imageDescriptor, .dstSet = 0, .dstBinding = static_cast<uint32_t>(binding), .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, .descriptorCount = 1U };
				vkCmdPushDescriptorSetKHRXY2D(cmdbuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->pipelineLayout, 0, 1, &imageDescriptorSet);
			}
			
			void ComputeEnd(glm::ivec3 groupSize) {
				vkCmdDispatch(cmdbuffer, groupSize.x, groupSize.y, groupSize.z);
			}
			
			template<typename T>
			void TransferBufferList(std::vector<T> dataList, uint32_t sizeOfObject, uint32_t offsetOfData, xy2d_buffer* stageBuffer, xy2d_buffer* buffer) {
				ExecutionBarrier(XY2D_PIPELINESTAGES::TRANSFER, XY2D_ACCESSSTAGES::TRANSFER, {});
				
				for(size_t i = 0; i < dataList.size(); i++) {
					void* stagedOffset = static_cast<int8_t*>(stageBuffer->description.pMappedData) + offsetOfData + (i * sizeOfObject);
					SDL_memcpy(stagedOffset, dataList[i], sizeOfObject);
				}
				
				VkDeviceSize sizeOfData = static_cast<VkDeviceSize>(dataList.size() * sizeOfObject);
				VkBufferCopy copyRegion { .srcOffset = offsetOfData, .dstOffset = 0, .size = sizeOfData };
				vkCmdCopyBuffer(cmdbuffer, stageBuffer->buffer, buffer->buffer, 1, &copyRegion);
			}
			
			template<typename T>
			void TransferBuffer(T& dataPointer, uint32_t offsetOfData, xy2d_buffer* stageBuffer, xy2d_buffer* buffer) {
				TransferBufferEXT(&dataPointer, sizeof(dataPointer), offsetOfData, stageBuffer, buffer);
			}
			
			void TransferBufferEXT(void* dataPointer, size_t sizeOfData, size_t offsetOfData, xy2d_buffer* stageBuffer, xy2d_buffer* buffer, uint32_t dstOffset = 0U) {
				ExecutionBarrier(XY2D_PIPELINESTAGES::TRANSFER, XY2D_ACCESSSTAGES::TRANSFER, {});
				
				void* stagedOffset = static_cast<int8_t*>(stageBuffer->description.pMappedData) + offsetOfData;
				SDL_memcpy(stagedOffset, dataPointer, sizeOfData);
				
				VkBufferCopy copyRegion { .srcOffset = offsetOfData, .dstOffset = dstOffset, .size = sizeOfData };
				vkCmdCopyBuffer(cmdbuffer, stageBuffer->buffer, buffer->buffer, 1, &copyRegion);
			}
			
			template<typename T>
			void TransferImage(T& dataPointer, uint32_t offsetOfData, uint32_t xpos, uint32_t ypos, uint32_t width, uint32_t height, xy2d_buffer* stageBuffer, xy2d_image* image) {
				TransferImageEXT(&dataPointer, sizeof(dataPointer), offsetOfData, stageBuffer, image);
			}
			
			void TransferImageEXT(void* dataPointer, uint32_t sizeOfData, uint32_t offsetOfData, uint32_t xpos, uint32_t ypos, uint32_t width, uint32_t height, xy2d_buffer* stageBuffer, xy2d_image* image) {
				ExecutionBarrier(XY2D_PIPELINESTAGES::TRANSFER, XY2D_ACCESSSTAGES::TRANSFER, { image });
				
				void* stagedOffset = static_cast<int8_t*>(stageBuffer->description.pMappedData) + offsetOfData;
				SDL_memcpy(stagedOffset, dataPointer, sizeOfData);
				
				VkExtent3D copyExtent = { .width = width, .height = height, .depth = 1U };
				VkOffset3D copyOffset = { .x = static_cast<int32_t>(xpos), .y = static_cast<int32_t>(ypos), .z = 0 };
				
				VkBufferImageCopy copyRegion = {};
				copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				copyRegion.imageSubresource.layerCount = 1;
				copyRegion.imageOffset = { .x = static_cast<int32_t>(xpos), .y = static_cast<int32_t>(ypos), .z = 0 };
				copyRegion.imageExtent = { .width = width, .height = height, .depth = 1U };;
				copyRegion.bufferOffset = static_cast<VkDeviceSize>(offsetOfData);
				vkCmdCopyBufferToImage(cmdbuffer, stageBuffer->buffer, image->imageSource, (VkImageLayout) image->imageLayout, 1U, &copyRegion);
			}
		};
	}
#endif