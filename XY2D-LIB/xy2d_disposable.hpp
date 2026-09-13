#pragma once
#ifndef __XY2D_DISPOSABLE
#define __XY2D_DISPOSABLE
	#include "./xy2d_engine.hpp"

	namespace XY2D_NAMESPACE {
		class xy2d_disposable {
		private:
			std::atomic_bool disposed = ATOMIC_VAR_INIT(false);
			
		public:
			xy2d_invoker<> onDispose;
			
			void Dispose() {
				if (disposed.load(std::memory_order_relaxed)) return;
				onDispose.invoke();
				disposed.store(true, std::memory_order_relaxed);
			}
		};
	}
#endif