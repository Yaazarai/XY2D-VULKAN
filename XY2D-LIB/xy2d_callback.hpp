/// @brief Source: https://stackoverflow.com/questions/9568150/what-is-a-c-delegate/9568485#9568485
#pragma once
#ifndef __XY2D_CALLBACK
#define __XY2D_CALLBACK
	#include "./xy2d_engine.hpp"
	
	namespace XY2D_NAMESPACE {
		template<typename... A>
		class xy2d_callback {
		public:
			size_t hash;
			std::function<void(A...)> bound;
			
			xy2d_callback(std::function<void(A...)> func)
				: hash(func.target_type().hash_code()), bound(std::move(func)) {}
			
			bool compare(const xy2d_callback<A...>& cb) {
				return hash == cb.hash;
			}
			
			constexpr size_t hash_code() const throw() {
				return hash;
			}
			
			xy2d_callback<A...>& invoke(A... args) {
				bound(static_cast<A&&>(args)...); return (*this);
			}
		};
		
		template<typename... A>
		class xy2d_invoker {
		public:
			std::mutex safety_lock;
			std::vector<xy2d_callback<A...>> callbacks;
			
			void hook(const xy2d_callback<A...> cb) {
				std::lock_guard<std::mutex> guard(safety_lock);
				
				callbacks.push_back(cb);
			}
			
			void invoke(A... args) {
				std::lock_guard<std::mutex> guard(safety_lock);
				
				for (xy2d_callback<A...> cb : callbacks)
					cb.invoke(static_cast<A&&>(args)...);
			}
		};
	}
#endif