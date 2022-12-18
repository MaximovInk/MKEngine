#include <mkpch.h>

#include <SDL.h>
#include <SDL_syswm.h>

#include "SDLBackend.h"
#include "MKEngine/Core/Log.h"
#include "../Window.h"

namespace MKEngine {

	struct SDLWindowData {
	public:
		Window* window;
		SDL_Window* nativeWindow;

		SDLWindowData()
		{
			window = nullptr;
			nativeWindow = nullptr;
		}

		SDLWindowData(Window* window, SDL_Window* nativeWindow)
		{
			this->window = window;
			this->nativeWindow = nativeWindow;
		}
	};

	std::map<std::int16_t, SDLWindowData> windows;

	SDLBackend::SDLBackend()
	{
		MK_LOG_TRACE("Initializing SDL..");
		int sdlSuccess = SDL_Init(SDL_INIT_EVERYTHING);

		MK_ASSERT(sdlSuccess == 0, "Could not initialize SDL!");

		if (sdlSuccess != 0)
			return;

		RendererAPI::Make();

		MK_LOG_INFO("Initialization successfully");
		
	}

	SDLBackend::~SDLBackend()
	{
		MK_LOG_TRACE("breeeed1MKEngine fiffffffnalizing..");

		RendererAPI::Destroy();

		SDL_Quit();
	}

	void SDLBackend::HandleEvents()
	{
		SDL_Event event;

		while (SDL_PollEvent(&event)) {
			
			switch (event.type) {
			case SDL_MOUSEMOTION:
			{
				auto motionEvent
					= MouseMovedEvent(event.motion.x, event.motion.y);

				eventCallback(motionEvent);
			}
				break;
			case SDL_WINDOWEVENT:
			{
				auto wndData = windows[event.window.windowID];
			switch (event.window.event)
			{
				//RESIZE EVENT
			case SDL_WINDOWEVENT_SIZE_CHANGED:
			{auto resizeEvent
				= WindowResizedEvent(event.window.data1, event.window.data2, wndData.window);
			eventCallback(resizeEvent); }
				break;
				//CLOSE EVENT
			case SDL_WINDOWEVENT_CLOSE:
			{auto closeEvent = WindowCloseEvent(wndData.window);
			eventCallback(closeEvent); }
				break;
			}
			}
				break;

			}
			
		}
	}


	void SDLBackend::Render()
	{
	}

	void SDLBackend::Update()
	{
		

		/*
		for (auto const& wnd : windows)
		{
			wnd.second.window->Update();
		}

		for (auto const& wnd : windows)
		{
			MakeCurrent(wnd.second.nativeWindow);
			wnd.second.window->Render();

		}
		*/
	}

	void SDLBackend::SetEventCallback(const EventCallbackFn& callback)
	{
		this->eventCallback = callback;
	}

	void* SDLBackend::MakeWindow(Window* window, const WindowSettings& settings)
	{
		Uint32 flags = SDL_WINDOW_SHOWN;
		if (settings.resizable)
			flags |= SDL_WINDOW_RESIZABLE;

		//flags |= SDL_WINDOW_OPENGL;
		switch (RendererAPI::s_RenderBackend)
		{
		case RenderBackendType::GL_RENDERER:
			flags |= SDL_WINDOW_OPENGL;
			break;
		case RenderBackendType::VK_RENDERER:
			flags |= SDL_WINDOW_VULKAN;
			break;
		default:
			break;
		}

		auto nativeWindow = SDL_CreateWindow(
			settings.Title.c_str(),
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			settings.Width,
			settings.Height,
			flags);

		windows[SDL_GetWindowID(nativeWindow)] = SDLWindowData(window, nativeWindow);

		return nativeWindow;
	}
	void SDLBackend::MakeCurrent(Window* window)
	{
		MakeCurrent(window->GetNativeWindow());
	}

	void SDLBackend::MakeCurrent(void* nativeWindow)
	{
		//SDL_GL_MakeCurrent((SDL_Window*)nativeWindow, RendererAPI::s_API->GetContext());
	}

	void SDLBackend::SwapWindow(Window* window)
	{
		SwapWindow(window->GetNativeWindow());
	}

	void SDLBackend::SwapWindow(void* nativeWindow)
	{
		//SDL_GL_SwapWindow((SDL_Window*)nativeWindow);
	}
	void SDLBackend::DestroyWindow(Window* window)
	{
		SDL_DestroyWindow((SDL_Window*)window->GetNativeWindow());
	}
}