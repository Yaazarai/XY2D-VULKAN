#include ".\XY2D-LIB\xy2d_engine.hpp"
using namespace xy2d;

#define UVFRAG "./Shaders/uvout_frag.spv"
#define FRAG "./Shaders/texture_output_frag.spv"
#define VERT "./Shaders/default_output_vert.spv"

xy2d_device* vkdevice;
xy2d_renderer* renderer;
xy2d_pipeline* pipeline;
xy2d_pipeline* pipeline_uv;

xy2d_shader* vert_shader;
xy2d_shader* frag_shader;
xy2d_shader* uvfrag_shader;

xy2d_buffer* stage_buffer;
xy2d_buffer* vertex_buffer;
xy2d_buffer* camera_buffer1;
xy2d_buffer* camera_buffer2;

xy2d_image* sampler_image;

xy2d_sprite present_sprite, render_sprite;

std::thread* renderThread;
glm::float32 theta = 0.0;

void PresentScene(xy2d_cmdbuffer* cmdbuffer, xy2d_image* swapChainImage) {
	theta += 0.00125;
	//present_sprite.Rotate(theta);
	glm::vec2 center = glm::vec2(xy2d_window::GetWindowWidth(), xy2d_window::GetWindowHeight());
	glm::vec2 windowSize = center;
	//present_sprite.Size(center);
	present_sprite.Position((windowSize - glm::vec2(present_sprite.xywh.z, present_sprite.xywh.w)) * glm::vec2(0.5));
	//present_sprite.Origin(glm::vec2(640.0, 480.0) * glm::vec2(0.5));
	//present_sprite.Update();
	present_sprite.Update();
	/*
		NOTE: Buffer / Image memory is shared PER-RENDER-PASS due to using only one command buffer.
		So if you want separate camera/vertex transforms, etc. you need a unique STAGING buffer per render pass.
	*/
	
	xy2d_camera cameraData1 = xy2d_camera::GetCamera(glm::vec2(sampler_image->width, sampler_image->height), glm::vec2(0.0, 0.0), glm::vec2(1.0, 1.0), glm::vec2(0.0, 0.0), 0.0);
	cmdbuffer->TransferBufferEXT(&cameraData1, sizeof(xy2d_camera), 0U, stage_buffer, camera_buffer1);
	
	cmdbuffer->TransferBufferEXT(render_sprite.vertices, render_sprite.SizeOf(), sizeof(xy2d_camera) + sizeof(xy2d_camera), stage_buffer, vertex_buffer, 0U);
	cmdbuffer->TransferBufferEXT(present_sprite.vertices, present_sprite.SizeOf(), sizeof(xy2d_camera) + sizeof(xy2d_camera) + render_sprite.SizeOf(), stage_buffer, vertex_buffer, render_sprite.SizeOf());
	
	cmdbuffer->RenderBegin(pipeline_uv, { sampler_image }, { 0U, 0U, sampler_image->width, sampler_image->height });
	cmdbuffer->RenderPushBuffer(pipeline_uv, camera_buffer1, 0);
	cmdbuffer->RenderBindVertexBuffer(vertex_buffer);
	cmdbuffer->RenderDrawVertices(6, 0, 1);
	cmdbuffer->RenderEnd();
	
	xy2d_camera cameraData2 = xy2d_camera::GetCamera(windowSize, glm::vec2(0.0, 0.0), glm::vec2(1.0, 1.0), windowSize * glm::vec2(0.5), theta);
	cmdbuffer->TransferBufferEXT(&cameraData2, sizeof(xy2d_camera), sizeof(xy2d_camera), stage_buffer, camera_buffer2);
	
	cmdbuffer->RenderBegin(pipeline, { swapChainImage }, { 0U, 0U, swapChainImage->width, swapChainImage->height });
	cmdbuffer->RenderPushBuffer(pipeline, camera_buffer2, 0);
	cmdbuffer->RenderPushImageSampler(pipeline, sampler_image, 1);
	cmdbuffer->RenderBindVertexBuffer(vertex_buffer);
	cmdbuffer->RenderDrawVertices(6, 6, 1);
	cmdbuffer->RenderEnd();
}

void RenderScene() {
	uint32_t frameIndex = 0U;
	double framePrevious = 0.0;
	
	while(!xy2d_window::GetWindowCloseRequest()) {
		auto frameStart = std::chrono::steady_clock::now();
		renderer->FrameRenderAndPresent();
		auto frameEnd = std::chrono::steady_clock::now();
		
		double frameTime = std::chrono::duration<double, std::milli>(frameEnd - frameStart).count();
		double fps = 1000.0 / frameTime;
		//printf("Frame: %.3f ms (%.1f FPS)\n", frameTime, fps);
		
		double frameSpan = 1000.0 / 240.0;
		//std::cout << "FRAME: [" << frameIndex << "] : " << ((renderer->frameTimeStamps[1] - framePrevious) - frameSpan) << std::endl;
		framePrevious = renderer->frameTimeStamps[0];
		
		frameIndex ++;
	}
}

void xy2d_window::WindowAppInit() {
	std::cout << "SIZEOF VERTEX: " << sizeof(xy2d_vertex) << std::endl;
	vkdevice = new xy2d_device();
	renderer = new xy2d_renderer(*vkdevice);
	vert_shader = new xy2d_shader(VERT, XY2D_SHADERSTAGE::VERTEX, { XY2D_DESCRIPTORS::UBO });
	frag_shader = new xy2d_shader(FRAG, XY2D_SHADERSTAGE::FRAGMENT, { XY2D_DESCRIPTORS::SAMPLER });
	uvfrag_shader = new xy2d_shader(UVFRAG, XY2D_SHADERSTAGE::FRAGMENT, {});
	
	pipeline = new xy2d_pipeline(*vkdevice, { *vert_shader, *frag_shader, });
	pipeline_uv = new xy2d_pipeline(*vkdevice, { *vert_shader, *uvfrag_shader, });
	
	stage_buffer = new xy2d_buffer(*vkdevice, XY2D_BUFFERTYPE::STAGING, static_cast<VkDeviceSize>(sizeof(xy2d_camera) + sizeof(xy2d_camera) + render_sprite.SizeOf() + present_sprite.SizeOf()));
	vertex_buffer = new xy2d_buffer(*vkdevice, XY2D_BUFFERTYPE::VERTEX, static_cast<VkDeviceSize>(present_sprite.SizeOf() * 4));
	camera_buffer1 = new xy2d_buffer(*vkdevice, XY2D_BUFFERTYPE::UNIFORM, static_cast<VkDeviceSize>(sizeof(xy2d_camera)));
	camera_buffer2 = new xy2d_buffer(*vkdevice, XY2D_BUFFERTYPE::UNIFORM, static_cast<VkDeviceSize>(sizeof(xy2d_camera)));
	
	sampler_image = new xy2d_image(*vkdevice, XY2D_IMAGETYPE::ATTACHEMENT, 640, 480);
	
	render_sprite = xy2d_sprite();
	present_sprite = xy2d_sprite();
	
	render_sprite.Color(0xFFFFFFFFU);
	present_sprite.Color(0xFFFFFFFFU);
	render_sprite.Size(glm::vec2(640.0, 480.0));
	render_sprite.Update();
	present_sprite.Size(glm::vec2(640.0, 480.0));
	present_sprite.Update();
	
	renderer->renderEvent.hook(xy2d_callback<xy2d_cmdbuffer*, xy2d_image*>(PresentScene));
	renderThread = new std::thread([](){ RenderScene(); });
	//xy2d_window::onAppIterate.hook(xy2d_callback<>(RenderScene));
	
	xy2d_window::onAppQuit.hook(xy2d_callback<>([&](){
		if (renderThread != VK_NULL_HANDLE) {
			renderThread->join();
			delete renderThread;
		}
		
		vkdevice->WaitDeviceIdle();
		delete camera_buffer1;
		delete camera_buffer2;
		delete sampler_image;
		delete stage_buffer;
		delete vertex_buffer;
		delete renderer;
		delete frag_shader;
		delete uvfrag_shader;
		delete vert_shader;
		delete pipeline;
		delete pipeline_uv;
		delete vkdevice;
	}));
}