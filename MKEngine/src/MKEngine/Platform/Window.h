#pragma once
#include <mkengine.h>
#include "mkpch.h"
#include "MKEngine/Events/WindowEvent.h"
#include "WindowSettings.h"

namespace MKEngine {

	struct WindowData {
		std::string Title;
		uint32_t Width{ 0 };
		uint32_t Height{ 0 };
		bool VSync{ false };
	};

	class Window {
	public:
		explicit Window(const WindowSettings& settings);

		virtual ~Window();

		virtual void OnWindowResize(MKEngine::WindowResizedEvent& event);
		virtual void OnWindowClose(MKEngine::WindowCloseEvent& event) = 0;
		
		virtual void FixedUpdate() = 0;
		virtual void Update() = 0;
		virtual void Render() = 0;

		void SetTitle(const char* title) const;
		void* GetNativeWindow() const;
		int GetID() const;

		WindowData GetData();

		protected:
			WindowData m_data;

		private:
			int m_id;
			void* m_nativeWindow;
	};
}