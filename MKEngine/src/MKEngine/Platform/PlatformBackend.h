#pragma once
#include "mkpch.h"
#include "MKEngine/Events/Event.h"
#include "MKEngine/Core/core.h"
#include "WindowSettings.h"

namespace MKEngine {
	class Window;

	using EventCallbackFn = std::function<void(MKEngine::Event&)>;

	class PlatformBackend {
	public:
		static UniquePointer<PlatformBackend> s_CurrentBackend;

		virtual ~PlatformBackend() = default;

		virtual void OnUpdate() = 0;

		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;

		virtual void MakeWindow(Window* window, const WindowSettings& settings) = 0;

		static void Initialize();

		static void Finalize();
	};
}