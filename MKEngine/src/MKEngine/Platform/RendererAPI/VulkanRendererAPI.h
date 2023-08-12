#pragma once

#include "MKEngine/Platform/RendererAPI.h"

namespace MKEngine {
	class VulkanRendererAPI final : public RendererAPI {
	public:
		VulkanRendererAPI();
		~VulkanRendererAPI() override;

		void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
		void Clear() override;
		void InitFunctions(void* procAddress) override;

	private:
		void OnWindowCreated(Window* window) override;

		void OnWindowDestroyed(Window* window) override;

		void OnWindowResized(Window* window) override;

		void OnWindowRender(Window* window) override;

	public:
		bool operator==(const VulkanRendererAPI& other) const;
	};
}