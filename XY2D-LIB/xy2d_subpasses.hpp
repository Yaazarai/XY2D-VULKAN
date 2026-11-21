#pragma once
#ifndef __XY2D_subpasses
#define __XY2D_subpasses
	#include "..\XY2D-LIB\xy2d_engine.hpp"
	
	void xy2d_renderpass(xy2d_cmdbuffer& cmdbuffer, xy2d_shaderpipe& pipeline, glm::ivec4 renderArea, std::vector<xy2d_gpualloc*> imageTargets, std::vector<xy2d_gpualloc*> uniforms, std::vector<glm::ivec3> drawCalls) {
		std::vector<xy2d_gpualloc*> imageBarriers;
		for(xy2d_gpualloc* un : uniforms)
			if (un->type == XY2D_GPUALLOC_TYPE::ATTACHEMENT || un->type == XY2D_GPUALLOC_TYPE::SWAPCHAIN || un->type == XY2D_GPUALLOC_TYPE::STORAGE)
				imageBarriers.push_back(un);
		
		xy2d_cmdbuffer_timestamps_inject(cmdbuffer);
		xy2d_cmdbuffer_barrier(cmdbuffer, XY2D_PIPELINESTAGES::REMDER, XY2D_ACCESSSTAGES::RENDER, imageBarriers);
		xy2d_render_begin(cmdbuffer, pipeline, imageTargets);
		xy2d_shader_bind_uniforms(cmdbuffer, pipeline, uniforms);
		xy2d_render_bind_vertex_buffer(cmdbuffer, pipeline.vertexBuffer);
			for(glm::ivec3 draw : drawCalls)
				xy2d_render_draw_vertex_buffer(cmdbuffer, draw[0], draw[1], draw[2]);
		xy2d_render_end(cmdbuffer);
		xy2d_cmdbuffer_timestamps_inject(cmdbuffer);
	}
	
	struct xy2d_transfer { xy2d_gpualloc* allocation; void* datapointer; size_t length; uint32_t xpos, ypos, width, height; };
	void xy2d_transferpass(xy2d_cmdbuffer& cmdbuffer, xy2d_shaderpipe& pipeline, std::vector<xy2d_transfer> transferCalls) {
		size_t stagingOffset = 0;
		for(xy2d_transfer tr : transferCalls) {
			xy2d_cmdbuffer_timestamps_inject(cmdbuffer);
			if (tr.allocation->type == XY2D_GPUALLOC_TYPE::ATTACHEMENT || tr.allocation->type == XY2D_GPUALLOC_TYPE::SWAPCHAIN || tr.allocation->type == XY2D_GPUALLOC_TYPE::STORAGE) {
				xy2d_transfer_image(cmdbuffer, pipeline.stagingBuffer, stagingOffset, *tr.allocation, tr.datapointer, tr.length, tr.xpos, tr.ypos, tr.width, tr.height);
			} else {
				xy2d_transfer_buffer(cmdbuffer, pipeline.stagingBuffer, stagingOffset, *tr.allocation, tr.datapointer, tr.length);
			}
			xy2d_cmdbuffer_timestamps_inject(cmdbuffer);
		}
	}
	
	void xy2d_computepass(xy2d_cmdbuffer& cmdbuffer, xy2d_shaderpipe& pipeline, std::vector<xy2d_gpualloc*> uniforms, std::vector<glm::ivec3> dispatches) {
		std::vector<xy2d_gpualloc*> imageBarriers;
		for(xy2d_gpualloc* un : uniforms)
			if (un->type == XY2D_GPUALLOC_TYPE::ATTACHEMENT || un->type == XY2D_GPUALLOC_TYPE::SWAPCHAIN || un->type == XY2D_GPUALLOC_TYPE::STORAGE)
				imageBarriers.push_back(un);
		
		xy2d_cmdbuffer_timestamps_inject(cmdbuffer);
		xy2d_cmdbuffer_barrier(cmdbuffer, XY2D_PIPELINESTAGES::COMPUTE, XY2D_ACCESSSTAGES::COMPUTE, imageBarriers);
		vkCmdBindShadersEXTXY2D(cmdbuffer.cmdbuffer, pipeline.shaderStages.size(), reinterpret_cast<VkShaderStageFlagBits*>(pipeline.shaderStages.data()), pipeline.shaderObjects.data());
		xy2d_shader_bind_uniforms(cmdbuffer, pipeline, uniforms);
			for(glm::ivec3 groupSize : dispatches)
				vkCmdDispatch(cmdbuffer.cmdbuffer, groupSize.x, groupSize.y, groupSize.z);
		xy2d_cmdbuffer_timestamps_inject(cmdbuffer);
	}
#endif