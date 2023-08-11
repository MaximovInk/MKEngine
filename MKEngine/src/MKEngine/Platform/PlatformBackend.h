#pragma once
#include "mkpch.h"
#include "MKEngine/Events/Event.h"
#include "MKEngine/Core/core.h"
#include "WindowSettings.h"
#include "RendererAPI.h"

namespace MKEngine {
	class Window;

	using EventCallbackFn = std::function<void(MKEngine::Event&)>;

	class PlatformBackend {
	public:
		static PlatformBackend* s_CurrentBackend;

		EventCallbackFn eventCallback;

		PlatformBackend();
		~PlatformBackend();

		void HandleEvents();

		void Update();

		void Render();

		uint64_t GetTicks();
		uint64_t GetPerfomanceFrequency();

		void SetEventCallback(const EventCallbackFn& callback);

		void* MakeWindow(Window* window, const WindowSettings& settings);
		void DestroyWindow(Window* window);
		void GetWindowSize(Window* window, int* w, int* h);

		static void Initialize();

		static void Finalize();

	};
}