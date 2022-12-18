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

		void SetEventCallback(const EventCallbackFn& callback);

		void* MakeWindow(Window* window, const WindowSettings& settings);
		void DestroyWindow(Window* window);

		static void Initialize();

		static void Finalize();

	};
}