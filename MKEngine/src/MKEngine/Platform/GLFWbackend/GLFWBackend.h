#pragma once
#include "../PlatformBackend.h"
#include "MKEngine/Core/Log.h"

namespace MKEngine {
	class GLFWBackend : public PlatformBackend {

	public:
		GLFWBackend();
		~GLFWBackend();

		virtual void OnUpdate() override;
		virtual void SetEventCallback(const EventCallbackFn& callback) override;
		virtual void MakeWindow(Window* window, const WindowSettings& settings) override;
	};

}