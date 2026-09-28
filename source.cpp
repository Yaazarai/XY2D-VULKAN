#include ".\XY2D-LIB\xy2d_engine.hpp"

#define LHRC "./Shaders/linespace_output_frag.spv"
#define SCNE "./Shaders/scene_output_frag.spv"
#define FRAG "./Shaders/texture_output_frag.spv"
#define VERT "./Shaders/default_output_vert.spv"
#define CASCADE_WIDTH 256
#define CASCADE_COUNT 9

xy2d_device vkdevice = {};
xy2d_renderer renderer = {};
xy2d_shader vertex_shader = {};
xy2d_shader scene_shader = {};
xy2d_shader linespace_shader = {};
xy2d_shader fragment_shader = {};
xy2d_shaderpipe pipeline_scne = {};
xy2d_shaderpipe pipeline_lhrc = {};
xy2d_shaderpipe pipeline_pres = {};
xy2d_gpualloc vertex_buffer = {};
xy2d_gpualloc camera_buffer = {};
xy2d_gpualloc sampler_image = {};
xy2d_gpualloc cascade_images[CASCADE_COUNT];

xy2d_sprite render_sprite = {};
xy2d_sprite present_sprite = {};

std::thread* renderThread;

void xy2d_window_quit(void* appstate, SDL_AppResult result) {
	if (renderThread != VK_NULL_HANDLE) {
		renderThread->join();
		delete renderThread;
	}
	
	vkDeviceWaitIdle(vkdevice.logical);
	xy2d_renderer_destroy(renderer);
	xy2d_shaderpipe_destroy(pipeline_scne);
	xy2d_shaderpipe_destroy(pipeline_lhrc);
	xy2d_shaderpipe_destroy(pipeline_pres);
	xy2d_gpualloc_destroy(vertex_buffer);
	xy2d_gpualloc_destroy(camera_buffer);
	xy2d_gpualloc_destroy(sampler_image);
	for(int i = 0; i < CASCADE_COUNT; i++)
		xy2d_gpualloc_destroy(cascade_images[i]);
	xy2d_device_destroy(vkdevice);
}

void PresentScene(xy2d_cmdbuffer& cmdbuffer, xy2d_gpualloc& swapChainImage) {
	glm::vec2 windowSize = glm::vec2(window.width.load(), window.height.load());
	
	xy2d_camera cameraData = xy2d_camera_orthographic(glm::vec2(sampler_image.length.width, sampler_image.length.height), glm::vec2(0.0, 0.0), glm::vec2(1.0, 1.0), glm::vec2(0.0, 0.0));
	xy2d_transfer_buffer(cmdbuffer, &cameraData, sizeof(xy2d_camera), 0U, &pipeline_scne.stagingBuffer, &camera_buffer);
	xy2d_transfer_buffer(cmdbuffer, render_sprite.vertices, xy2d_sprite_sizeof(), sizeof(xy2d_camera), &pipeline_scne.stagingBuffer, &vertex_buffer, 0U);
	
	xy2d_render_begin(cmdbuffer, pipeline_scne, { &sampler_image }, { 0U, 0U, sampler_image.length.width, sampler_image.length.height });
	xy2d_shader_bind_uniforms(cmdbuffer, pipeline_scne, { &camera_buffer, &sampler_image });
	xy2d_render_bind_vertex_buffer(cmdbuffer, &vertex_buffer);
	xy2d_render_draw_vertex_buffer(cmdbuffer, 6, 0, 1);
	xy2d_render_end(cmdbuffer);
	
	cameraData = xy2d_camera_orthographic(glm::vec2(cascade_images[0].length.width, cascade_images[0].length.height), glm::vec2(0.0, 0.0), glm::vec2(1.0, 1.0), glm::vec2(0.0, 0.0));
	xy2d_transfer_buffer(cmdbuffer, &cameraData, sizeof(xy2d_camera), 0U, &pipeline_lhrc.stagingBuffer, &camera_buffer);
	xy2d_transfer_buffer(cmdbuffer, render_sprite.vertices, xy2d_sprite_sizeof(), sizeof(xy2d_camera), &pipeline_lhrc.stagingBuffer, &vertex_buffer, 0U);
	xy2d_cmdbuffer_barrier(cmdbuffer, XY2D_PIPELINESTAGES::REMDER, XY2D_ACCESSSTAGES::RENDER, { &cascade_images[0] });
	
	for(int i = (CASCADE_COUNT - 1); i >= 0; i--) {
		int n = (i + 1) % CASCADE_COUNT;
		xy2d_render_begin(cmdbuffer, pipeline_lhrc, { &cascade_images[i] }, { 0U, 0U, cascade_images[i].length.width, cascade_images[i].length.height });
		xy2d_shader_bind_uniforms(cmdbuffer, pipeline_lhrc, { &camera_buffer, &cascade_images[i], &cascade_images[n] });
		xy2d_render_bind_vertex_buffer(cmdbuffer, &vertex_buffer);
		xy2d_render_draw_vertex_buffer(cmdbuffer, 6, 0, 1);
		xy2d_render_end(cmdbuffer);
	}
	
	cameraData = xy2d_camera_orthographic(windowSize, glm::vec2(0.0, 0.0), glm::vec2(1.0, 1.0), glm::vec2(0.0, 0.0));
	xy2d_transfer_buffer(cmdbuffer, &cameraData, sizeof(xy2d_camera), 0U, &pipeline_pres.stagingBuffer, &camera_buffer);
	xy2d_transfer_buffer(cmdbuffer, present_sprite.vertices, xy2d_sprite_sizeof(), sizeof(xy2d_camera), &pipeline_pres.stagingBuffer, &vertex_buffer, 0U);
	
	xy2d_render_begin(cmdbuffer, pipeline_pres, { &swapChainImage }, { 0U, 0U, swapChainImage.length.width, swapChainImage.length.height });
	xy2d_shader_bind_uniforms(cmdbuffer, pipeline_pres, { &camera_buffer, &cascade_images[0] });
	xy2d_render_bind_vertex_buffer(cmdbuffer, &vertex_buffer);
	xy2d_render_draw_vertex_buffer(cmdbuffer, 6, 0, 1);
	xy2d_render_end(cmdbuffer);
}

void render_scene() {
	uint32_t frameIndex = 0U;
	double framePrevious = 0.0;
	
	while(window.openState.load(std::memory_order_relaxed)) {
		VkResult result = xy2d_renderer_frame_present(renderer);
		std::cout << "FRAME: [" << frameIndex << "] : " << (renderer.frameTimeStamps[0]- framePrevious) << " : " << result << std::endl;
		framePrevious = renderer.frameTimeStamps.back();
		frameIndex ++;
	}
}

void xy2d_window_init() {
	SDL_SetWindowSize(window.handle, 1024, 1024);
	vertex_shader = xy2d_shader_create(VERT, XY2D_SHADER_STAGE::VERTEX, { XY2D_SHADER_UNIFORM::UBO });
	fragment_shader = xy2d_shader_create(FRAG, XY2D_SHADER_STAGE::FRAGMENT, { XY2D_SHADER_UNIFORM::SAMPLER });
	linespace_shader = xy2d_shader_create(LHRC, XY2D_SHADER_STAGE::FRAGMENT, { XY2D_SHADER_UNIFORM::SAMPLER, XY2D_SHADER_UNIFORM::SAMPLER });
	scene_shader = xy2d_shader_create(SCNE, XY2D_SHADER_STAGE::FRAGMENT, { XY2D_SHADER_UNIFORM::SAMPLER });
	
	vkdevice = xy2d_device_create();
	renderer = xy2d_renderer_create(vkdevice);
	pipeline_scne = xy2d_shaderpipe_create(vkdevice, { vertex_shader, scene_shader }, 1024U);
	pipeline_lhrc = xy2d_shaderpipe_create(vkdevice, { vertex_shader, linespace_shader }, 1024U);
	pipeline_pres = xy2d_shaderpipe_create(vkdevice, { vertex_shader, fragment_shader }, 1024U);
	
	vertex_buffer = xy2d_gpualloc_create(vkdevice, XY2D_GPUALLOC_TYPE::VERTEX, { static_cast<uint32_t>(xy2d_sprite_sizeof()) * 4U, 0U });
	camera_buffer = xy2d_gpualloc_create(vkdevice, XY2D_GPUALLOC_TYPE::UNIFORM, { sizeof(xy2d_camera), 0U });
	
	render_sprite = xy2d_sprite_create({0.0, 0.0, (float)CASCADE_WIDTH, (float)CASCADE_WIDTH}, {0.0, 0.0, 1.0, 1.0}, {1.0, 1.0}, {0.0, 0.0});
	present_sprite = xy2d_sprite_create({0.0, 0.0, 1024.0, 1024.0}, {0.0, 0.0, 1.0, 1.0}, {1.0, 1.0}, {0.0, 0.0});
	
	xy2d_sprite_size(render_sprite, {256.0f, 256.0f});
	xy2d_sprite_update(render_sprite);
	xy2d_sprite_size(present_sprite, {1024.0f, 1024.0f});
	xy2d_sprite_update(present_sprite);
	
	sampler_image = xy2d_gpualloc_create(vkdevice, XY2D_GPUALLOC_TYPE::ATTACHEMENT, {CASCADE_WIDTH, CASCADE_WIDTH});
	for(int i = 0; i < CASCADE_COUNT; i++)
		cascade_images[i] = xy2d_gpualloc_create(vkdevice, XY2D_GPUALLOC_TYPE::ATTACHEMENT, {CASCADE_WIDTH, CASCADE_WIDTH});
	
	auto presentCallback = xy2d_callback_create(PresentScene);
	xy2d_invoker_hook(renderer.renderEvent, presentCallback);
	renderThread = new std::thread([](){ render_scene(); });
}

SDL_AppResult xy2d_window_loop(void* appstate, SDL_AppResult result) { return result; }
SDL_AppResult xy2d_window_event(void*, SDL_Event* event, SDL_AppResult result) { return result; }