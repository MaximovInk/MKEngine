#include "mkpch.h"
#include "Window.h"


namespace MKEngine {
	Window::Window(const WindowSettings& settings)
	{
		MK_LOG_INFO("base");
		_nativeWindow = PlatformBackend::s_CurrentBackend->MakeWindow(this, settings);
		RendererAPI::s_API->OnWindowCreated(this);
	}

	Window::~Window()
	{
		RendererAPI::s_API->OnWindowDestroyed(this);
		PlatformBackend::s_CurrentBackend->DestroyWindow(this);
	}

	void* Window::GetNativeWindow() {
		return _nativeWindow;
	}
}