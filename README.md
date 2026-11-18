# XY2D-VULKAN
Serialized 2D specific Vulkan Renderer. This rendering model is simple and doesn't focus on multi-threaded command buffer rendering and just provides a basic interface for multi-pass rendering to a single managed command buffer with automated synchronization. The goal of this project is to implement a simple and complete serialized but Frame-in-Flight compliant 2D renderer with the ability to query the GPU for rendering timestamps--for accurate in-app GPU profiling (which isn't provided by most simple game engines, Raylib, SDL, GameMaker, etc.).

This model provides 1 frame-in-flight on the GPU and one processing on the CPU using a staggered wait model, where we render a command buffer and then wait for the previous frame to finish rendering and immediately submit the new command buffer to keep the GPU and CPU occupied in parallel. The major benefit of this is that we avoid upkeep and memory usage issues of traditional Frame-In-Flight models which require ping-ponging multi-buffered GPU resources in order to keep rendering in parallel.

## How does it work?
Most of the behavior logic lies in the `xy2d_renderer.hpp` header under the `xy2d_renderer_frame_present` function:
1. Acquire the next available buffered SwapChain (screen) image.
2. If the SwapChain is invalid we're waiting on a resize event (manual, minimize, maximize, etc.) and early exit--rendering is deferred until the next frame.
3. Wait for that acquired SwapChain image to become available for rendering.
4. Reset the command buffer allocated to this frame.
5. Inject a starting timestamp for GPU profiling.
6. Invoke the list of user hooked render functions (provided by the user--e.g. `xy2d_invoker_hook(renderer.renderEvent, presentCallback);`).
7. Inject a barrier and transition the SwapChain to present image layout. (rendering functions auto-transition back to general layout).
8. Inject an ending timestamp for GPU profiling.
9. Wait for the GPU to finish rendering the previous frame.
10. Immediately submit the current frame after the previous one ended.
11. Submit a queue present command for the current frame.

Note that the renderer doesn't specify what image is actually being rendered to. The user must past the SwapChain image provided by the renderer into their final present renderpass. The `renderer.renderEvent` event takes in a function that will be executing during rendering and passes the current SwapChain image to that function. This way the user's render function always have access to the current renderable SwapChain image.

The user first hooks their present/render function to the event, e.g. in their main function:
```C++
	auto presentCallback = xy2d_callback_create(PresentScene);
	xy2d_invoker_hook(renderer.renderEvent, presentCallback);
```
Then provide the implementation for that function which is compliant to the render event's function call type:
```C++
void PresentScene(xy2d_cmdbuffer& cmdbuffer, xy2d_gpualloc& swapChainImage) {
    // PROVIDED RENDER / PRESENT COMMANDS
}
```
Only the LAST subscribed function must render out to the SwapChain so you have to hook your events in execution order:
```C++
xy2d_invoker_hook(renderer.renderEvent, renderSceneCallback);
xy2d_invoker_hook(renderer.renderEvent, renderLightingCallback);
xy2d_invoker_hook(renderer.renderEvent, presentCallback);
```
The command buffer is also provided and must be passed to rendering commands. This example shows how to setup a simple 2D camera (orthographic proejction) and then transfer that camera data into a GPU buffer using a pre-defined share pipeline (rendering shaders, e.g. vertex/fragment shader pair) which pushes everything into the provided command buffer:
```C++
void PresentScene(xy2d_cmdbuffer& cmdbuffer, xy2d_gpualloc& swapChainImage) {
    glm::vec2 windowSize = glm::vec2(window.width.load(), window.height.load());
    xy2d_camera cameraData = xy2d_camera_orthographic(windowSize, glm::vec2(0.0, 0.0), glm::vec2(1.0, 1.0), glm::vec2(0.0, 0.0));

    size_t staging_offset = 0U;
    xy2d_transfer_buffer(cmdbuffer, pipeline_pres.stagingBuffer, staging_offset, camera_buffer, &cameraData, sizeof(xy2d_camera));
    ...
    ...
}
```
Renderpasses are sequential and can either be provided in a single render event or multiple render events. The example below performs a transfer pass and renderpass in a single event to send a sprite (quad vertices) and SDL image in CPU memory to the GPU before rendering them directly to the swapchain:
```C++
void PresentScene(xy2d_cmdbuffer& cmdbuffer, xy2d_gpualloc& swapChainImage) {
    // 1. Setup a 2D camera via orthographic projection:
    glm::vec2 windowSize = glm::vec2(window.width.load(), window.height.load());
    xy2d_camera cameraData = xy2d_camera_orthographic(windowSize, glm::vec2(0.0, 0.0), glm::vec2(1.0, 1.0), glm::vec2(0.0, 0.0));

    // 2. Using the Present Pipeline's staging buffer send the camera to the camera uniform buffer on the GPU:
    size_t staging_offset = 0U;
    xy2d_transfer_buffer(cmdbuffer, pipeline_pres.stagingBuffer, staging_offset, camera_buffer, &cameraData, sizeof(xy2d_camera));

    // 3. Using the Present Pipeline's staging buffer send the vertex/quad/sprite data to the vertex buffer:
    xy2d_transfer_buffer(cmdbuffer, pipeline_pres.stagingBuffer, staging_offset, vertex_buffer, present_sprite.vertices, xy2d_sprite_sizeof());
    
    // 4. Using the Present Pipeline's staging buffer send the CPU image (loaded via SDL) to an allocated GPU image:
    xy2d_transfer_image(cmdbuffer, pipeline_pres.stagingBuffer, staging_offset, sampler_image, phantom_surface->pixels, phantom_surface->w * phantom_surface->h * sizeof(uint32_t), 0U, 0U, 512U, 512U);

    // 5. This renderpass uses the loaded image, so insert a synchronization barrier for that image--or multiple images/buffers:
    xy2d_cmdbuffer_barrier(cmdbuffer, XY2D_PIPELINESTAGES::REMDER, XY2D_ACCESSSTAGES::RENDER, { &sampler_image });
    // 6. Begin the renderpass and provide the image attachments, in this case the SwapChain/Present image (one or multiple if doing MRT):
    xy2d_render_begin(cmdbuffer, pipeline_pres, { &swapChainImage });
    // 7. Bind the camera buffer and loaded image to the Present Pipeline:
    xy2d_shader_bind_uniforms(cmdbuffer, pipeline_pres, { &camera_buffer, &sampler_image });
    // 8. Bind the vertex buffer our vertex/quad/sprite data is in:
    xy2d_render_bind_vertex_buffer(cmdbuffer, vertex_buffer);
    // 9. Render the sprite/quad transfered to the GPU in our vertex buffer:
    xy2d_render_draw_vertex_buffer(cmdbuffer, 6, 0, 1);
    // 10. End the current render pass and work for this frame:
    xy2d_render_end(cmdbuffer);
}
```
Once all of your events are setup you run the `xy2d_renderer_frame_present(renderer);` function for the renderer in the SDL window's render loop OR your dedicated thread loop for rendering.

Here I've got a dedicate function in a separate render thread which handles the render loop (keeps the app from hanging during resize events). Then I pull the profiling info (timestamps) for the current frame and write them to the console:
```C++
void render_scene() {
    double framePrevious = 0.0;
    
    while(window.openState.load(std::memory_order_relaxed)) {
        VkResult result = xy2d_renderer_frame_present(renderer);
        glm::float64_t frame_start = renderer.frameTimeStamps[1];
        glm::float64_t frame_end = renderer.frameTimeStamps[1];
        std::cout << "FRAME: [" << frameIndex << "] : " << (frame_end - frame_start) << std::endl;
        frameIndex ++;
    }
}
```
The full example is provided in [source.cpp](https://github.com/Yaazarai/XY2D-VULKAN/blob/main/source.cpp).
