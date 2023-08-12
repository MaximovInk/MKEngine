#pragma once

#include "mkengine.h"
//#include "glm/glm.hpp"

/*
Useful links:
https://github.com/KhronosGroup/Vulkan-Guide/blob/main/chapters/dynamic_state.adoc - dynamic viewports etc.

*/

namespace MKEngine {

	class RendererAPI {
	public:
		virtual ~RendererAPI() = default;

		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
		virtual void Clear() = 0;

		virtual void* GetContext() { return m_context; }
		virtual void SetContext(void* context) { m_context = context;  }
		virtual void InitFunctions(void* procAddress) = 0;

		virtual void OnWindowCreated(Window* window) = 0;
		virtual void OnWindowDestroyed(Window* window) = 0;
		virtual void OnWindowResized(Window* window) = 0;
		virtual void OnWindowRender(Window* window) = 0;

		static void Make();
		static void Destroy();

		static RendererAPI* CurrentAPI;

	private:
		static void* m_context;
	};
}