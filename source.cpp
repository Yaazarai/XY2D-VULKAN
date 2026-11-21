#include ".\XY2D-LIB\xy2d_engine.hpp"

#define VERT "./Shaders/default_output_vert.spv"
#define FRAG "./Shaders/texture_output_frag.spv"
#define PHANTOM "./Images/phantom.png"
#define IMAGE_LENGTH 1024
#define CASCADE_COUNT 11

xy2d_device vkdevice = {};
xy2d_renderer renderer = {};
xy2d_shader vertex_shader = {}, fragment_shader = {};
xy2d_shaderpipe pipeline_present = {};
xy2d_gpualloc sampler_image = {};
xy2d_sprite present_sprite = {};
SDL_Surface* phantom_surface = VK_NULL_HANDLE;
std::thread* renderThread;

void xy2d_window_quit(void* appstate, SDL_AppResult result) {
	if (renderThread != VK_NULL_HANDLE) {
		renderThread->join();
		delete renderThread;
	}
	
	xy2d_device_waitidle(vkdevice);
	SDL_DestroySurface(phantom_surface);
	xy2d_renderer_destroy(renderer);
	xy2d_shaderpipe_destroy(pipeline_present);
	xy2d_gpualloc_destroy(sampler_image);
	xy2d_device_destroy(vkdevice);
}


void PresentScene(xy2d_cmdbuffer& cmdbuffer, xy2d_gpualloc& swapChainImage) {
	glm::vec2 windowSize = glm::vec2(window.width.load(), window.height.load());
	xy2d_camera cameraData = xy2d_camera_orthographic(windowSize, glm::vec2(0.0, 0.0), glm::vec2(1.0, 1.0), glm::vec2(0.0, 0.0));
	
	std::vector<xy2d_transfer> transferCalls = {
		{ &pipeline_present.uniformBuffer, &cameraData, sizeof(xy2d_camera), 0, 0, 0, 0 },
		{ &pipeline_present.vertexBuffer, present_sprite.vertices, xy2d_sprite_sizeof(), 0, 0, 0, 0 },
		{ &sampler_image, phantom_surface->pixels, phantom_surface->w * phantom_surface->h * sizeof(uint32_t), 0, 0, IMAGE_LENGTH, IMAGE_LENGTH }
	};
	xy2d_transferpass(cmdbuffer, pipeline_present, transferCalls);
	
	std::vector<xy2d_gpualloc*> imageTargets = { &swapChainImage };
	std::vector<xy2d_gpualloc*> uniforms = { &pipeline_present.uniformBuffer, &sampler_image };
	glm::ivec4 renderArea = { 0, 0, swapChainImage.length.width, swapChainImage.length.height };
	std::vector<glm::ivec3> drawCalls = { {6, 0, 1} };
	xy2d_renderpass(cmdbuffer, pipeline_present, renderArea, imageTargets, uniforms, drawCalls);
}

void render_scene() {
	size_t frameIndex = 0;
	while(window.openState.load(std::memory_order_relaxed)) {
		xy2d_renderer_frame_present(renderer);
		
		#if XY2D_VALIDATION
			glm::float64_t frameStart = renderer.frameTimeStamps.front();
			glm::float64_t frameEnd = renderer.frameTimeStamps.back();
			std::cout << "FRAME[" << frameIndex++ << "] - " << renderer.frameTimeStamps.size() << " - " << (frameEnd - frameStart) << "ms" << std::endl;
		#endif
	}
}

void xy2d_window_init() {
	SDL_SetWindowSize(window.handle, 1024, 1024);
	vkdevice = xy2d_device_create();
	renderer = xy2d_renderer_create(vkdevice);
	
	uint32_t stageBufferSize = IMAGE_LENGTH*IMAGE_LENGTH*128U;
	uint32_t vertexBufferSize = static_cast<uint32_t>(xy2d_sprite_sizeof()) * 4U;
	uint32_t uniformBufferSize = sizeof(xy2d_camera);
	vertex_shader = xy2d_shader_create(VERT, XY2D_SHADER_STAGE::VERTEX, { XY2D_SHADER_UNIFORM::UBO });
	fragment_shader = xy2d_shader_create(FRAG, XY2D_SHADER_STAGE::FRAGMENT, { XY2D_SHADER_UNIFORM::SAMPLER });
	pipeline_present = xy2d_shaderpipe_create(vkdevice, { vertex_shader, fragment_shader }, stageBufferSize, vertexBufferSize, uniformBufferSize);
	
	sampler_image = xy2d_gpualloc_create(vkdevice, XY2D_GPUALLOC_TYPE::ATTACHEMENT, {IMAGE_LENGTH, IMAGE_LENGTH}, true, VK_FORMAT_B8G8R8A8_UNORM);
	SDL_Surface* surface = SDL_LoadSurface(PHANTOM);
	phantom_surface = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_BGRA32);
	SDL_DestroySurface(surface);
	
	present_sprite = xy2d_sprite_create({0.0, 0.0, IMAGE_LENGTH, IMAGE_LENGTH});
	xy2d_sprite_size(present_sprite, {IMAGE_LENGTH, IMAGE_LENGTH});
	xy2d_sprite_origin(present_sprite, glm::vec2(IMAGE_LENGTH*0.5, IMAGE_LENGTH*0.5));
	xy2d_sprite_rotate(present_sprite, glm::radians(45.0));
	xy2d_sprite_position(present_sprite, glm::vec2(IMAGE_LENGTH*0.5, IMAGE_LENGTH*0.5));
	xy2d_sprite_color(present_sprite, 0xFF0000FF);
	xy2d_sprite_update(present_sprite);
	
	auto presentCallback = xy2d_callback_create(PresentScene);
	xy2d_invoker_hook(renderer.renderEvent, presentCallback);
	renderThread = new std::thread([](){ render_scene(); });
}

SDL_AppResult xy2d_window_loop(void* appstate, SDL_AppResult result) { return result; }

SDL_AppResult xy2d_window_event(void*, SDL_Event* event, SDL_AppResult result, const bool* keystates) { return result; }