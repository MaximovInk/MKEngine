#pragma once
#include "../PlatformBackend.h"
#include "MKEngine/Core/Log.h"

namespace MKEngine {
	class SDLBackend : public PlatformBackend {

	public:
		SDLBackend();
		~SDLBackend();

		virtual void OnUpdate() override;
		virtual void OnEventUpdate() override;
		virtual void OnRender() override;

		virtual void SetEventCallback(const EventCallbackFn& callback) override;
		virtual void* MakeWindow(Window* window, const WindowSettings& settings) override;
		virtual void MakeCurrent(Window* window) override;
		virtual void SwapWindow(Window* window) override;

	private:
		EventCallbackFn eventCallback;

		virtual void MakeCurrent(void* nativeWindow) override;
		virtual void SwapWindow(void* nativeWindow) override;
	};

}