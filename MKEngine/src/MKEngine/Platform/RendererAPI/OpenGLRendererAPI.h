#pragma once
#include "MKEngine/Platform/RendererAPI.h"

namespace MKEngine {
	class OpenGLRendererAPI : public RendererAPI {
	public:
		OpenGLRendererAPI();
		~OpenGLRendererAPI();

		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
		//virtual void SetClearColor(const glm::vec4& color) override;
		virtual void Clear() override;
		virtual void InitFunctions(void* procAddress) override;
	};
}