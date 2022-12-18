#pragma once
#include <mkengine.h>
#include "MKEngine/Core/core.h"
#include "mkpch.h"
#include "MKEngine/Events/WindowEvent.h"
#include "PlatformBackend.h"
#include "WindowSettings.h"

namespace MKEngine {
	class Window {
	public:
		
		Window(const WindowSettings& settings);

		virtual ~Window();

		virtual void OnWindowResize(MKEngine::WindowResizedEvent& event) = 0;
		virtual void OnWindowClose(MKEngine::WindowCloseEvent& event) = 0;

		//OnWindowResize
		//OnWindowClose
		//Update
		//FixedUpdate
		//Render
		
		virtual void FixedUpdate() = 0;
		virtual void Update() = 0;
		virtual void Render() = 0;

		void* GetNativeWindow();

		protected:

			struct WindowData {
				std::string Title; 
				uint32_t Width, Height;
			};

			WindowData _data;

		private:

			void* _nativeWindow;
	};
}