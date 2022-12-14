#include "mkpch.h"
#include "Window.h"

namespace MKEngine {
	Window::Window(const WindowSettings& settings)
	{
		MK_LOG_INFO("base");
		_nativeWindow = PlatformBackend::s_CurrentBackend->MakeWindow(this, settings);
	}

	void* Window::GetNativeWindow() {
		return _nativeWindow;
	}
}