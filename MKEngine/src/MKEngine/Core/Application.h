#pragma once
#include "MKEngine/Events/MouseEvent.h"
#include <MKEngine/Core/LayerStack.h>

namespace MKEngine {

	class Application {
	public:
		Application();
		~Application();

		void Run();

		static bool OnMouseMotion(MouseMovedEvent& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		static void CloseWindow(const Window* window);

		void Close();


		static double DeltaTime;
		static Application* Instance;

	private:
		bool m_running = false;

		LayerStack m_layerStack;

		void DispatchEvents(Event& e);
	};

	Application* CreateApplication();

}