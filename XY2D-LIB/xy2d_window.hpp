#pragma once
#ifndef __XY2D_appstate
#define __XY2D_appstate
	#include ".\xy2d_engine.hpp"
	
	#ifndef XY2D_WINDOW_MINWIDTH
		#define XY2D_WINDOW_MINWIDTH 640U
	#endif
	#ifndef XY2D_WINDOW_MINHEIGHT
		#define XY2D_WINDOW_MINHEIGHT 480U
	#endif
	
	struct xy2d_window {
		SDL_Window* handle = VK_NULL_HANDLE;
		std::atomic_uint32_t width = 0;
		std::atomic_uint32_t height = 0;
		std::atomic_uint32_t minWidth = XY2D_WINDOW_MINWIDTH;
		std::atomic_uint32_t minHeight = XY2D_WINDOW_MINHEIGHT;
		std::atomic_uint32_t openState = VK_TRUE;
	} window;
	
	void xy2d_window_quit(void*, SDL_AppResult);
	void xy2d_window_init();
	SDL_AppResult xy2d_window_loop(void*, SDL_AppResult);
	SDL_AppResult xy2d_window_event(void*, SDL_Event*, SDL_AppResult);
	
	uint32_t xy2d_window_get_isopen() { return window.openState.load(std::memory_order_relaxed); }
	uint32_t xy2d_window_get_width() { return window.width.load(std::memory_order_relaxed); }
	uint32_t xy2d_window_get_height() { return window.height.load(std::memory_order_relaxed); }
	
	static void SDL_AppQuit(void* appstate, SDL_AppResult result) {
		xy2d_window_quit(appstate, result);
		SDL_DestroyWindow(window.handle);
	}
	
	static SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
		SDL_SetAppMetadata("xy2d engine (SDL3)", "1.0", VK_NULL_HANDLE);
		if (!SDL_Init(SDL_INIT_VIDEO)) return SDL_APP_FAILURE;
		
		window.width.store((xy2d_window_get_width() > 0U)? xy2d_window_get_width() : XY2D_WINDOW_MINWIDTH, std::memory_order_relaxed);
		window.height.store((xy2d_window_get_height() > 0U)? xy2d_window_get_height() : XY2D_WINDOW_MINHEIGHT, std::memory_order_relaxed);
		
		window.handle = SDL_CreateWindow("xy2d engine", window.width, window.height, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
		SDL_SetWindowMinimumSize(window.handle, window.minWidth, window.minHeight);
		xy2d_window_init();
		return (window.handle != VK_NULL_HANDLE)? SDL_APP_CONTINUE : SDL_APP_FAILURE;
	}
	
	static SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
		SDL_AppResult result = (event->type == SDL_EVENT_QUIT || event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)? SDL_APP_SUCCESS : SDL_APP_CONTINUE;
		
		if (result == SDL_APP_SUCCESS)
			window.openState.store(VK_FALSE, std::memory_order_relaxed);
		
		int width, height;
		SDL_GetWindowSizeInPixels(window.handle, &width, &height);
		window.width.store(static_cast<uint32_t>(width), std::memory_order_relaxed);
		window.height.store(static_cast<uint32_t>(height), std::memory_order_relaxed);
		return xy2d_window_event(appstate, event, result);
	}
	
	static SDL_AppResult SDL_AppIterate(void* appstate) {
		return xy2d_window_loop(appstate, SDL_APP_CONTINUE);
	}
#endif