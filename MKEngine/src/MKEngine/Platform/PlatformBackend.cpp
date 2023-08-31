#include "mkpch.h"
#include <SDL.h>
#include <SDL_syswm.h>
#include <SDL_vulkan.h>

#include "PlatformBackend.h"
#include "MKEngine/Core/Log.h"
#include "MKEngine/Input/input.h"

namespace MKEngine {

    struct SDLWindowData {
    public:
        Window* MKWindow;
        SDL_Window* NativeWindow;

        SDLWindowData()
        {
            MKWindow = nullptr;
            NativeWindow = nullptr;
        }

        SDLWindowData(Window* window, SDL_Window* nativeWindow)
        {
            this->MKWindow = window;
            this->NativeWindow = nativeWindow;
        }
    };

    static std::map<std::int16_t, SDLWindowData> windows;

    PlatformBackend* PlatformBackend::CurrentBackend;

    void PlatformBackend::Initialize()
    {
	    CurrentBackend = new PlatformBackend();
    }

    void PlatformBackend::Finalize()
    {
        delete CurrentBackend;
    }

    PlatformBackend::PlatformBackend()
    {
        MK_LOG_TRACE("Initializing SDL..");
        const int sdlSuccess = SDL_Init(SDL_INIT_EVERYTHING);

        MK_ASSERT(sdlSuccess == 0, "Could not initialize SDL!");

        if (sdlSuccess != 0)
            return;

        MK_LOG_INFO("Initialization successfully");
    }

    PlatformBackend::~PlatformBackend()
    {
        MK_LOG_TRACE("MKEngine finalizing..");

        SDL_Quit();
    }

    void PlatformBackend::HandleEvents() const
    {
        Input::update();
        SDL_Event event;

        while (SDL_PollEvent(&event)) {

            switch (event.type) {
                case SDL_MOUSEMOTION:
                {
                    auto motionEvent
                        = MouseMovedEvent(event.motion.x, event.motion.y);
                    EventCallback(motionEvent);
                    break;
                }
                case SDL_WINDOWEVENT:
                {
	                const auto wndData = windows[event.window.windowID];
                    switch (event.window.event)
                    {
                        case SDL_WINDOWEVENT_SIZE_CHANGED:
                        {
                            auto resizeEvent
                                = WindowResizedEvent(event.window.data1,
                                event.window.data2, wndData.MKWindow);
                            EventCallback(resizeEvent);
                            break;
                        }
                        case SDL_WINDOWEVENT_CLOSE:
                        {
                            auto closeEvent = WindowCloseEvent(wndData.MKWindow);
                            EventCallback(closeEvent);
                            break;
                        }
                    default: ;
                    }
                    break;
                }
            default: ;
            }

            Input::EventUpdate(&event);
        }
    }

    void PlatformBackend::Update()
    {
        


        for (const auto& [key, value] : windows) {
            if(value.MKWindow == nullptr)
            {
	            if (const auto iterator = windows.find(key); iterator != windows.end())
                    windows.erase(iterator);
                return;
            }
            value.MKWindow->Update();
        }

    }

    void PlatformBackend::Render()
    {
        for (const auto& [key, value] : windows) {

            RendererAPI::CurrentAPI->OnWindowRender(value.MKWindow);

            value.MKWindow->Render();
        }
    }

    uint64_t PlatformBackend::GetTicks()
    {
        return SDL_GetPerformanceCounter();
    }

    uint64_t PlatformBackend::GetPerformanceFrequency() {
        return SDL_GetPerformanceFrequency();
    }

    void PlatformBackend::SetEventCallback(const EventCallbackFn& callback)
    {
        this->EventCallback = callback;
    }

    void* PlatformBackend::MakeWindow(Window* window, const WindowSettings& settings)
    {
        Uint32 flags = SDL_WINDOW_SHOWN;
        if (settings.Resizable)
            flags |= SDL_WINDOW_RESIZABLE;

        flags |= SDL_WINDOW_VULKAN;

        const auto nativeWindow = SDL_CreateWindow(
            settings.Title.c_str(),
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            settings.Width,
            settings.Height,
            flags);

        windows[SDL_GetWindowID(nativeWindow)] = SDLWindowData(window, nativeWindow);

        return nativeWindow;
    }

    void PlatformBackend::DestroyWindow(const Window* window)
    {
        //TODO: not work
        auto iter = windows.find(SDL_GetWindowID(static_cast<SDL_Window*>(window->GetNativeWindow())));
        if (iter != windows.end())
            windows.erase(iter);
        SDL_DestroyWindow(static_cast<SDL_Window*>(window->GetNativeWindow()));

        MK_LOG_INFO("REMOVE {0}", windows.size());
    }

    void PlatformBackend::GetWindowSize(const Window* window, int *w,int *h)
    {
        SDL_Vulkan_GetDrawableSize(static_cast<SDL_Window*>(window->GetNativeWindow()),w,h);
    }


}