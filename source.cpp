#include ".\XY2D-LIB\xy2d_engine.hpp"
using namespace xy2d;

#define FRAG "./Shaders/default_output_frag.spv"
#define VERT "./Shaders/default_output_vert.spv"

struct camera_ubo {
	glm::mat4x4 projection;
} camera;

xy2d_device* vkdevice;
xy2d_renderer* renderer;
xy2d_pipeline* pipeline;
xy2d_shader* vert_shader;
xy2d_shader* frag_shader;
xy2d_buffer* stage_buffer;
xy2d_buffer* vertex_buffer;
xy2d_buffer* camera_buffer;
xy2d_sprite present_sprite;
glm::float32 theta = 0.0;
std::thread* renderThread;

glm::mat4 CameraTransform(glm::vec2 cameraSize, glm::vec2 cameraPosition, glm::vec2 cameraScale = glm::vec2(1.0f), glm::float32_t cameraTheta = 0.0f, glm::vec2 zNearFar = glm::vec2(1.0f, -1.0f)) {
    glm::vec2 cameraCenter = cameraSize / 2.0f;
    glm::mat4 projection = glm::ortho(0.0f, cameraSize.x, 0.0f, cameraSize.y, zNearFar.x, zNearFar.y);
    projection = glm::translate(projection, glm::vec3(cameraCenter, 0.0f));
    projection = glm::rotate(projection, cameraTheta, glm::vec3(0.0f, 0.0f, 1.0f));
    projection = glm::scale(projection, glm::vec3(cameraScale, 1.0f));
    projection = glm::translate(projection, glm::vec3(-cameraCenter - cameraPosition, 0.0f));
    return projection;
}

void PresentScene(xy2d_renderer& renderer, xy2d_cmdbuffer& cmdbuffer) {
	theta += 0.25 * 0.125;
	present_sprite.Rotate(theta);
	present_sprite.Update();
	
	glm::mat4 cameraData = CameraTransform(xy2d_window::extent, glm::vec2(0.0, 0.0), glm::vec2(1.0, 1.0));
	cmdbuffer.TransferBuffer(cameraData, 0U, stage_buffer, camera_buffer);
	cmdbuffer.TransferBuffer(present_sprite.vertices, sizeof(glm::mat4), stage_buffer, vertex_buffer);
	
	cmdbuffer.RenderBegin(*pipeline, { &cmdbuffer.swapChainIamge }, { 0U, 0U, cmdbuffer.swapChainIamge.width, cmdbuffer.swapChainIamge.height });
	cmdbuffer.RenderPushBuffer(*pipeline, *camera_buffer, 0);
	cmdbuffer.RenderBindVertexBuffer(*vertex_buffer);
	cmdbuffer.RenderDrawVertices(6, 0, 1);
	cmdbuffer.RenderEnd();
}

void RenderScene() {
	while(!xy2d_window::GetWindowCloseRequest()) {
		auto frameStart = std::chrono::steady_clock::now();
		renderer->RenderSwapChain();
		auto frameEnd = std::chrono::steady_clock::now();
		
		double frameTime = std::chrono::duration<double, std::milli>(frameEnd - frameStart).count();
		double fps = 1000.0 / frameTime;
		printf("Frame: %.3f ms (%.1f FPS)\n", frameTime, fps);
	}
}

void xy2d_window::WindowAppInit() {
	vkdevice = new xy2d_device();
	renderer = new xy2d_renderer(*vkdevice);
	vert_shader = new xy2d_shader(VERT, XY2D_SHADERSTAGE::VERTEX, { XY2D_DESCRIPTORS::UBO });
	frag_shader = new xy2d_shader(FRAG, XY2D_SHADERSTAGE::FRAGMENT, {});
	pipeline = new xy2d_pipeline(*vkdevice, { *vert_shader, *frag_shader, });
	stage_buffer = new xy2d_buffer(*vkdevice, XY2D_BUFFERTYPE::STAGING, static_cast<VkDeviceSize>(sizeof(glm::mat4) + present_sprite.SizeOf()));
	vertex_buffer = new xy2d_buffer(*vkdevice, XY2D_BUFFERTYPE::VERTEX, static_cast<VkDeviceSize>(present_sprite.SizeOf()));
	camera_buffer = new xy2d_buffer(*vkdevice, XY2D_BUFFERTYPE::UNIFORM, static_cast<VkDeviceSize>(sizeof(glm::mat4)));
	
	present_sprite.Size(glm::vec2(700.0, 700.0));
	glm::vec2 center = glm::vec2(present_sprite.xywh[2], present_sprite.xywh[3]) * glm::vec2(0.5);
	present_sprite.Origin(center);
	present_sprite.Position(center);
	present_sprite.Update();
	
	renderer->renderEvent.hook(xy2d_callback<xy2d_renderer&, xy2d_cmdbuffer&>(PresentScene));
	renderThread = new std::thread([](){ RenderScene(); });
	//xy2d_window::onAppIterate.hook(xy2d_callback<>(RenderScene));
	
	xy2d_window::onAppQuit.hook(xy2d_callback<>([&](){
		if (renderThread != VK_NULL_HANDLE) {
			renderThread->join();
			delete renderThread;
		}
		
		delete stage_buffer;
		delete vertex_buffer;
		delete renderer;
		delete frag_shader;
		delete vert_shader;
		delete pipeline;
		delete vkdevice;
	}));
}