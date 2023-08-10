#pragma once
#include "MKEngine/Core/core.h"
#include "MKEngine/Platform/PlatformBackend.h"
#include "MKEngine/Events/MouseEvent.h"
#include <MKEngine/Core/LayerStack.h>

namespace MKEngine {

	class Application {
	public:
		Application();
		~Application();

		void Run();

		bool OnMouseMotion(MKEngine::MouseMovedEvent& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		void CloseWindow(Window* window);

		void Close();


		static double DeltaTime;
		static Application* s_Application;

	private:
		bool m_Running = false;

		LayerStack m_LayerStack;

		void DispatchEvents(Event& e);
	};

	Application* CreateApplication();

}