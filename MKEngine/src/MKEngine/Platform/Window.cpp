#include "mkpch.h"
#include "Window.h"

#include "PlatformBackend.h"
#include "SDL.h"

namespace MKEngine {
	
	Window::Window(const WindowSettings& settings)
	{
		MK_LOG_INFO("base");
		m_nativeWindow = PlatformBackend::CurrentBackend->MakeWindow(this, settings);
		m_id = SDL_GetWindowID(static_cast<SDL_Window*>(m_nativeWindow));

		m_data.Width = settings.Width;
		m_data.Height = settings.Height;

		RendererAPI::CurrentAPI->OnWindowCreated(this);
	}

	Window::~Window()
	{
		RendererAPI::CurrentAPI->OnWindowDestroyed(this);
		PlatformBackend::CurrentBackend->DestroyWindow(this);
	}

	void Window::OnWindowResize(MKEngine::WindowResizedEvent& event)
	{
		MK_LOG_INFO("resize base");

		m_data.Width = event.GetWidth();
		m_data.Height = event.GetHeight();

		RendererAPI::CurrentAPI->OnWindowResized(this);
	}

	void Window::SetTitle(const char* title) const
	{
		SDL_SetWindowTitle(static_cast<SDL_Window*>(m_nativeWindow), title);
	}

	void* Window::GetNativeWindow() const
	{
		return m_nativeWindow;
	}

	int Window::GetID() const
	{
		return m_id;
	}

	WindowData Window::GetData()
	{
		return m_data;
	}
}
