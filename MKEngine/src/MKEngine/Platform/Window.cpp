#include "mkpch.h"
#include "Window.h"
#include "SDL.h"

namespace MKEngine {
	
	Window::Window(const WindowSettings& settings)
	{
		MK_LOG_INFO("base");
		_nativeWindow = PlatformBackend::s_CurrentBackend->MakeWindow(this, settings);
		_id = SDL_GetWindowID((SDL_Window*)_nativeWindow);

		_data.Width = settings.Width;
		_data.Height = settings.Height;

		RendererAPI::s_API->OnWindowCreated(this);
	}

	Window::~Window()
	{
		RendererAPI::s_API->OnWindowDestroyed(this);
		PlatformBackend::s_CurrentBackend->DestroyWindow(this);
	}

	void Window::OnWindowResize(MKEngine::WindowResizedEvent& event)
	{
		MK_LOG_INFO("resize base");

		_data.Width = event.GetWidth();
		_data.Height = event.GetHeight();

		RendererAPI::s_API->OnWindowResized(this);
	}

	void Window::SetTitle(const char* title)
	{
		SDL_SetWindowTitle((SDL_Window*)_nativeWindow, title);
	}

	void* Window::GetNativeWindow() {
		return _nativeWindow;
	}
	int Window::GetID()
	{
		return _id;
	}
	WindowData Window::GetData()
	{
		return _data;
	}
}
