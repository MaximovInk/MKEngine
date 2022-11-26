#include "mkpch.h"
#include "SDLBackend.h"
#include "SDL.h"
#include "MKEngine/Core/Log.h"
#include "../Window.h"

namespace MKEngine {

	struct SDLWindowData {
	public:
		Window* window;
		SDL_Window* nativeWindow;

		SDLWindowData()
		{

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

		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

		MK_LOG_INFO("Initialization successfully");
	}

	SDLBackend::~SDLBackend()
	{
		MK_LOG_TRACE("MKEngine finalizing..");

		SDL_Quit();
	}

	void SDLBackend::OnUpdate()
	{
		SDL_Event event;

		while (SDL_PollEvent(&event)) {


			switch (event.type)
			{
			case SDL_MOUSEMOTION:
			{auto motionEvent
				= MouseMovedEvent(event.motion.x, event.motion.y);

			eventCallback(motionEvent);
			}
			break;
			case SDL_WINDOWEVENT:
			{
				auto wndData = windows[event.window.windowID];

				switch (event.window.event) {
				case SDL_WINDOWEVENT_SIZE_CHANGED:
					auto resizeEvent
						= WindowResizedEvent(event.window.data1, event.window.data2, wndData.window);

					eventCallback(resizeEvent);

					break;
				}
			}
			break;
			}
		}

		for (auto const& wnd : windows)
		{
			wnd.second.window->Update();
		}
	}

	void SDLBackend::SetEventCallback(const EventCallbackFn& callback)
	{
		this->eventCallback = callback;
	}

	void SDLBackend::MakeWindow(Window* window, const WindowSettings& settings)
	{
		Uint32 flags = SDL_WINDOW_SHOWN;
		if (settings.resizable)
			flags |= SDL_WINDOW_RESIZABLE;

		auto nativeWindow = SDL_CreateWindow(
			settings.Title.c_str(),
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			settings.Width,
			settings.Height,
			flags);

		windows[SDL_GetWindowID(nativeWindow)] = SDLWindowData(window, nativeWindow);
	}
}