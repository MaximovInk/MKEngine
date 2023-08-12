#pragma once
#include "mkpch.h"
#include "MKEngine/Events/Event.h"
#include "WindowSettings.h"
#include "RendererAPI.h"

namespace MKEngine {
	class Window;

	using EventCallbackFn = std::function<void(MKEngine::Event&)>;

	class PlatformBackend {
	public:
		static PlatformBackend* CurrentBackend;

		EventCallbackFn EventCallback;

		PlatformBackend();
		~PlatformBackend();

		void HandleEvents() const;

		static void Update();

		static void Render();

		static uint64_t GetTicks();
		static uint64_t GetPerformanceFrequency();

		void SetEventCallback(const EventCallbackFn& callback);

		static void* MakeWindow(Window* window, const WindowSettings& settings);
		static void DestroyWindow(const Window* window);
		static void GetWindowSize(const Window* window, int* w, int* h);

		static void Initialize();

		static void Finalize();

	};
}