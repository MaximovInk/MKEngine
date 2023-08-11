#pragma once
#include <mkengine.h>
#include "MKEngine/Core/core.h"
#include "mkpch.h"
#include "MKEngine/Events/WindowEvent.h"
#include "PlatformBackend.h"
#include "WindowSettings.h"

namespace MKEngine {


	struct WindowData {
		std::string Title;
		uint32_t Width, Height;
		bool VSync;
	};

	class Window {
	public:
		
		Window(const WindowSettings& settings);

		virtual ~Window();

		virtual void OnWindowResize(MKEngine::WindowResizedEvent& event);
		virtual void OnWindowClose(MKEngine::WindowCloseEvent& event) = 0;
		

		//OnWindowResize
		//OnWindowClose
		//Update
		//FixedUpdate
		//Render
		
		virtual void FixedUpdate() = 0;
		virtual void Update() = 0;
		virtual void Render() = 0;

		void SetTitle(const char* title);
		void* GetNativeWindow();
		int GetID();

		WindowData GetData();

		protected:
			WindowData _data;

		private:
			int _id;
			void* _nativeWindow;
	};
}