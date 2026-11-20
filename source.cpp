#include ".\XY2D-LIB\xy2d_engine.hpp"

#define LHRC "./Shaders/linespace_output_frag.spv"
#define SCNE "./Shaders/scene_output_frag.spv"
#define FRAG "./Shaders/texture_output_frag.spv"
#define VERT "./Shaders/default_output_vert.spv"
#define PHANTOM "./Images/phantom.png"
#define CASCADE_WIDTH 1024
#define CASCADE_COUNT 11

uint32_t frameIndex = 0U;
SDL_Surface* phantom_surface = VK_NULL_HANDLE;
xy2d_device vkdevice = {};
xy2d_renderer renderer = {};
xy2d_shader vertex_shader = {};
xy2d_shader fragment_shader = {};
xy2d_shaderpipe pipeline_pres = {};
//xy2d_gpualloc vertex_buffer = {};
//xy2d_gpualloc camera_buffer = {};
xy2d_gpualloc sampler_image = {};

xy2d_sprite render_sprite = {};
xy2d_sprite present_sprite = {};
std::thread* renderThread;

std::atomic<int> keyCheck;
std::atomic<int> cascadeIndex;

struct cascade {
	int index, count;
};

void xy2d_window_quit(void* appstate, SDL_AppResult result) {
	if (renderThread != VK_NULL_HANDLE) {
		renderThread->join();
		delete renderThread;
	}
	
	vkDeviceWaitIdle(vkdevice.logical);
	SDL_DestroySurface(phantom_surface);
	xy2d_renderer_destroy(renderer);
	xy2d_shaderpipe_destroy(pipeline_pres);
	//xy2d_gpualloc_destroy(vertex_buffer);
	//xy2d_gpualloc_destroy(camera_buffer);
	xy2d_gpualloc_destroy(sampler_image);
	xy2d_device_destroy(vkdevice);
}

void PresentScene(xy2d_cmdbuffer& cmdbuffer, xy2d_gpualloc& swapChainImage) {
	glm::vec2 windowSize = glm::vec2(window.width.load(), window.height.load());
	xy2d_camera cameraData = xy2d_camera_orthographic(windowSize, glm::vec2(0.0, 0.0), glm::vec2(1.0, 1.0), glm::vec2(0.0, 0.0));
	
	size_t staging_offset = 0U;
	xy2d_transfer_buffer(cmdbuffer, pipeline_pres.stagingBuffer, staging_offset, pipeline_pres.uniformBuffer, &cameraData, sizeof(xy2d_camera));
	
	xy2d_transfer_buffer(cmdbuffer, pipeline_pres.stagingBuffer, staging_offset, pipeline_pres.vertexBuffer, present_sprite.vertices, xy2d_sprite_sizeof());
	xy2d_transfer_image(cmdbuffer, pipeline_pres.stagingBuffer, staging_offset, sampler_image, phantom_surface->pixels, phantom_surface->w * phantom_surface->h * sizeof(uint32_t), 0U, 0U, 1024U, 1024U);
	
	xy2d_cmdbuffer_barrier(cmdbuffer, XY2D_PIPELINESTAGES::REMDER, XY2D_ACCESSSTAGES::RENDER, {});
	xy2d_cmdbuffer_barrier(cmdbuffer, XY2D_PIPELINESTAGES::REMDER, XY2D_ACCESSSTAGES::RENDER, { &sampler_image });
	xy2d_render_begin(cmdbuffer, pipeline_pres, { &swapChainImage });
	xy2d_shader_bind_uniforms(cmdbuffer, pipeline_pres, { &pipeline_pres.uniformBuffer, &sampler_image });
	xy2d_render_bind_vertex_buffer(cmdbuffer, pipeline_pres.vertexBuffer);
	xy2d_render_draw_vertex_buffer(cmdbuffer, 6, 0, 1);
	xy2d_render_end(cmdbuffer);
}

void render_scene() {
	double framePrevious = 0.0;
	
	while(window.openState.load(std::memory_order_relaxed)) {
		VkResult result = xy2d_renderer_frame_present(renderer);
		glm::float64_t frame_start = renderer.frameTimeStamps[0];
		glm::float64_t frame_end = renderer.frameTimeStamps[1];
		std::cout << "FRAME: [" << frameIndex << "] : " << (frame_end - frame_start) << std::endl;
		frameIndex ++;
	}
}

void xy2d_window_init() {
	SDL_SetWindowSize(window.handle, 1024, 1024);
	vertex_shader = xy2d_shader_create(VERT, XY2D_SHADER_STAGE::VERTEX, { XY2D_SHADER_UNIFORM::UBO });
	fragment_shader = xy2d_shader_create(FRAG, XY2D_SHADER_STAGE::FRAGMENT, { XY2D_SHADER_UNIFORM::SAMPLER });
	
	vkdevice = xy2d_device_create();
	renderer = xy2d_renderer_create(vkdevice);
	
	uint32_t stageBufferSize = 1024U*1024U*128U;
	uint32_t vertexBufferSize = static_cast<uint32_t>(xy2d_sprite_sizeof()) * 4U;
	uint32_t uniformBufferSize = sizeof(xy2d_camera);
	pipeline_pres = xy2d_shaderpipe_create(vkdevice, { vertex_shader, fragment_shader }, stageBufferSize, vertexBufferSize, uniformBufferSize);
	
	//vertex_buffer = xy2d_gpualloc_create(vkdevice, XY2D_GPUALLOC_TYPE::VERTEX, { static_cast<uint32_t>(xy2d_sprite_sizeof()) * 4U, 0U });
	//camera_buffer = xy2d_gpualloc_create(vkdevice, XY2D_GPUALLOC_TYPE::UNIFORM, { sizeof(xy2d_camera), 0U });
	
	SDL_Surface* surface = SDL_LoadSurface(PHANTOM);
	phantom_surface = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_BGRA32);
	SDL_DestroySurface(surface);
	
	present_sprite = xy2d_sprite_create({0.0, 0.0, 1024.0, 1024.0});
	xy2d_sprite_size(present_sprite, {1024.0f, 1024.0f});
	xy2d_sprite_update(present_sprite);
	
	sampler_image = xy2d_gpualloc_create(vkdevice, XY2D_GPUALLOC_TYPE::ATTACHEMENT, {CASCADE_WIDTH, CASCADE_WIDTH}, true, VK_FORMAT_B8G8R8A8_UNORM);
	
	auto presentCallback = xy2d_callback_create(PresentScene);
	xy2d_invoker_hook(renderer.renderEvent, presentCallback);
	renderThread = new std::thread([](){ render_scene(); });
}

SDL_AppResult xy2d_window_loop(void* appstate, SDL_AppResult result) { return result; }

SDL_AppResult xy2d_window_event(void*, SDL_Event* event, SDL_AppResult result, const bool* keystates) {
	if (event->type == SDL_EVENT_MOUSE_BUTTON_UP) {
		const bool left = event->button.button == SDL_BUTTON_LEFT && !event->button.down;
		const bool right = event->button.button == SDL_BUTTON_RIGHT && !event->button.down;
		keyCheck = (int)right - (int)left;
		cascadeIndex += (int)keyCheck;
		cascadeIndex = std::clamp((int)cascadeIndex, 0, CASCADE_COUNT - 1);
	}
	
	return result;
}