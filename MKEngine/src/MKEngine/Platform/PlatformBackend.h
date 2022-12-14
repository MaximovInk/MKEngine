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

		virtual ~PlatformBackend() = default;
		
		virtual void OnEventUpdate() = 0;

		virtual void OnUpdate() = 0;

		virtual void OnRender() = 0;

		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;

		virtual void* MakeWindow(Window* window, const WindowSettings& settings) = 0;

		virtual void MakeCurrent(Window* window) = 0;

		virtual void SwapWindow(Window* window) = 0;

		static void Initialize();

		static void Finalize();

		virtual void MakeCurrent(void* nativeWindow) = 0;
		virtual void SwapWindow(void* nativeWindow) = 0;

	//protected:
	//	virtual void InitBackend()=0;
	//	virtual void FinalizeBackend() = 0;
	};
}