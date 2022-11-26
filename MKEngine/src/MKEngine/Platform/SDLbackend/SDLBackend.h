#pragma once
#include "../PlatformBackend.h"
#include "MKEngine/Core/Log.h"

namespace MKEngine {

	

	class SDLBackend : public PlatformBackend {

	public:
		SDLBackend();
		~SDLBackend();

		virtual void OnUpdate() override;
		virtual void SetEventCallback(const EventCallbackFn& callback) override;
		virtual void MakeWindow(Window* window, const WindowSettings& settings) override;

	private:
		EventCallbackFn eventCallback;
	};

}