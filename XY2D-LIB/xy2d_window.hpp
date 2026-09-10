#pragma once
#ifndef __XY2D_appstate
#define __XY2D_appstate
	#include ".\xy2d_engine.hpp"
	
	namespace XY2D_NAMESPACE {
		class xy2d_window {
		public:
			inline static SDL_Window* handle = VK_NULL_HANDLE;
			inline static glm::vec2 minimum = glm::vec2(640.0, 480.0);
			inline static std::atomic<uint32_t> width;
			inline static std::atomic<uint32_t> height;
			inline static std::atomic<bool> closing;
			inline static std::vector<std::string> events;
			
			inline static xy2d::xy2d_invoker<> onAppQuit;
			inline static xy2d::xy2d_invoker<> onAppInit;
			inline static xy2d::xy2d_invoker<SDL_Event*> onAppEvent;
			inline static xy2d::xy2d_invoker<> onAppIterate;
			
			inline static void WindowAppInit();
			
			inline static void SetWindowSize(int32_t width, int32_t height) {
				xy2d_window::width.store(static_cast<uint32_t>(std::max(width, static_cast<int32_t>(minimum.x))), std::memory_order_relaxed);
				xy2d_window::height.store(static_cast<uint32_t>(std::max(height, static_cast<int32_t>(minimum.y))), std::memory_order_relaxed);
				
				if (handle != VK_NULL_HANDLE)
					SDL_SetWindowSize(handle, static_cast<int32_t>(xy2d_window::width), static_cast<int32_t>(xy2d_window::height));
			}
			
			inline static VkSurfaceKHR GetWindowSurface(VkInstance vkinst) {
				VkSurfaceKHR surface = VK_NULL_HANDLE;
				bool valid = SDL_Vulkan_CreateSurface(handle, vkinst, VK_NULL_HANDLE, &surface);
				return (valid)? surface : VK_NULL_HANDLE;
			}
			
			inline static bool GetWindowCloseRequest() {
				return closing.load(std::memory_order_relaxed);
			}
			
			inline static uint32_t GetWindowWidth() {
				return width.load(std::memory_order_relaxed);
			}
			
			inline static uint32_t GetWindowHeight() {
				return height.load(std::memory_order_relaxed);
			}
			
			inline static std::vector<const char*> GetInstanceExtensions() {
				uint32_t extensionCount = 0;
				const char* const* extensions = SDL_Vulkan_GetInstanceExtensions(&extensionCount);
				return std::vector<const char*>(extensions, extensions + extensionCount);
			}
			
			inline static SDL_AppResult LogEvent(SDL_AppResult result, std::vector<std::string> list) {
				#if XY2D_VALIDATION
					std::stringstream stream;
					for(std::string ss : list)
						stream << ss;
					events.push_back(stream.str());
					std::cout << "xy2d: " << stream.str() << std::endl;
				#endif
				return result;
			}
			
			inline static void AppQuit(void* appstate, SDL_AppResult result) {
				onAppQuit.invoke();
				if (handle != VK_NULL_HANDLE)
					SDL_DestroyWindow(handle);
			}
			
			inline static SDL_AppResult AppInit(void** appstate, int argc, char* argv[]) {
				SDL_SetAppMetadata("xy2d engine (SDL3)", "1.0", VK_NULL_HANDLE);
				
				glm::vec2 extent = glm::vec2(std::max(extent.x, minimum.x), std::max(extent.y, minimum.y));
				width.store(static_cast<uint32_t>(extent.x));
				height.store(static_cast<uint32_t>(extent.y));
				closing.store(false);
				
				if (!SDL_Init(SDL_INIT_VIDEO))
					return LogEvent(SDL_APP_FAILURE, { "Couldn't initialize SDL: ", SDL_GetError() });
				
				if ((handle = SDL_CreateWindow("xy2d engine", xy2d_window::width, xy2d_window::height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN)) == VK_NULL_HANDLE)
					return LogEvent(SDL_APP_FAILURE, { "Couldn't create window: ", SDL_GetError() });
				
				SDL_SetWindowMinimumSize(handle, minimum.x, minimum.y);
				WindowAppInit();
				return SDL_APP_CONTINUE;
			}
			
			inline static SDL_AppResult AppEvent(void* appstate, SDL_Event* event) {
				SDL_AppResult result = (event->type == SDL_EVENT_QUIT || event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)? SDL_APP_SUCCESS : SDL_APP_CONTINUE;
				
				if (event->type == SDL_EVENT_QUIT || event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
					closing.store(true);
				
				int width, height;
				SDL_GetWindowSizeInPixels(handle, &width, &height);
				xy2d_window::width.store(static_cast<uint32_t>(width), std::memory_order_relaxed);
				xy2d_window::height.store(static_cast<uint32_t>(height), std::memory_order_relaxed);
				onAppEvent.invoke(event);
				return result;
			}
			
			inline static SDL_AppResult AppIterate(void* appstate) {
				onAppIterate.invoke();
				return SDL_APP_CONTINUE;
			}
		};
	}
	
	static void SDL_AppQuit(void* appstate, SDL_AppResult result) { xy2d::xy2d_window::AppQuit(appstate, result); }
	static SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) { return xy2d::xy2d_window::AppInit(appstate, argc, argv); }
	static SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) { return xy2d::xy2d_window::AppEvent(appstate, event); }
	static SDL_AppResult SDL_AppIterate(void* appstate) { return xy2d::xy2d_window::AppIterate(appstate); }
#endif