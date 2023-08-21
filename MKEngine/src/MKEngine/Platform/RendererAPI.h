#pragma once

#include "mkengine.h"
//#include "glm/glm.hpp"

/*
Useful links:
VkGuide: https://github.com/KhronosGroup/Vulkan-Guide - useful features
Dynamic-state: https://github.com/KhronosGroup/Vulkan-Guide/blob/main/chapters/dynamic_state.adoc - dynamic viewports etc.


*/

namespace MKEngine {

	class RendererAPI {
	public:
		virtual ~RendererAPI() = default;

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