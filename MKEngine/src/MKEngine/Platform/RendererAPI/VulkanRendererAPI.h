#pragma once

#include "MKEngine/Platform/RendererAPI.h"

namespace MKEngine {
	class VulkanRendererAPI : public RendererAPI {
	public:
		VulkanRendererAPI();
		~VulkanRendererAPI();

		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
		//virtual void SetClearColor(const glm::vec4& color) override;
		virtual void Clear() override;
		virtual void InitFunctions(void* procAddress) override;

	private:


		// Унаследовано через RendererAPI
		virtual void OnWindowCreated(Window* window) override;

		virtual void OnWindowDestroyed(Window* window) override;

	};
}