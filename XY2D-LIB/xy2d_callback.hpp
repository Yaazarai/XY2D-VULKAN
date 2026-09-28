/// @brief Source: https://stackoverflow.com/questions/9568150/what-is-a-c-delegate/9568485#9568485
#pragma once
#ifndef __XY2D_CALLBACK
#define __XY2D_CALLBACK
	#include "./xy2d_engine.hpp"
	
	template<typename... A>
	struct xy2d_callback {
		size_t hash;
		std::function<void(A...)> bound;
	};
	
	template<typename... A>
	struct xy2d_invoker {
		std::vector<xy2d_callback<A...>> callbacks;
	};
	
	template<typename... A>
	xy2d_callback<A...> xy2d_callback_create(void (*func)(A...)) {
    	return { .hash = std::function<void(A...)>(func).target_type().hash_code(), .bound = func };
	}
	
	template<typename... A>
	void xy2d_callback_invoke(xy2d_callback<A...>& callback, A... args) {
		callback.bound(static_cast<A&&>(args)...);
	}
	
	template<typename... A>
	void xy2d_invoker_hook(xy2d_invoker<A...>& invoker, const xy2d_callback<A...> cb) {
		invoker.callbacks.push_back(cb);
	}
	
	template<typename... A>
	void xy2d_invoker_invoke(xy2d_invoker<A...>& invk, A... args) {
		for (auto& cb : invk.callbacks)
			xy2d_callback_invoke<A...>(cb, static_cast<A&&>(args)...);
	}
#endif