#pragma once

#include "mkengine.h"
//#include "glm/glm.hpp"

namespace MKEngine {

	struct Color {
		float r;
		float g;
		float b;
		float a;
	};

	struct RenderData {
		Color ClearColor;
	};

	enum class RenderBackendType {
		GL_RENDERER,
		VK_RENDERER	
	};

	class RendererAPI {
	public:

		virtual ~RendererAPI() = default;

		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
		virtual void Clear() = 0;

		virtual void* GetContext() { return s_Context; }
		virtual void SetContext(void* context) { s_Context = context;  }
		virtual void InitFunctions(void* procAddress) = 0;

		virtual void OnWindowCreated(Window* window) = 0;
		virtual void OnWindowDestroyed(Window* window) = 0;

		static void Make(RenderBackendType backend = RenderBackendType::VK_RENDERER);
		static void Destroy();

		static RendererAPI* s_API;
		static RenderBackendType s_RenderBackend;

	private:
		static void* s_Context;
	};
}