#include <mkpch.h>

#include <chrono>

#include "Application.h"
#include "MKEngine/Core/Log.h"
#include "MKEngine/Events/EventSystem.h"
#include "MKEngine/WindowsManager/WindowsManagerLayer.h"

namespace MKEngine {

	Application* Application::s_Application;

	bool Application::OnMouseMotion(MouseMovedEvent& e)
	{
		return true;
	}

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* layer)
	{
		m_LayerStack.PushOverlay(layer);
		layer->OnAttach();
	}

	void Application::CloseWindow(Window* window)
	{
		delete window;
	}

	void Application::Close()
	{
		m_Running = false;
	}

	void Application::DispatchEvents(Event& e) {
		EventDispatcher dispatcher(e);

		dispatcher.Dispatch<MouseMovedEvent>(MK_BIND_EVENT_FN(Application::OnMouseMotion));

		for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
		{
			if (e.Handled)
				break;
			(*it)->OnEvent(e);
		}

		if ((e.GetCategoryFlags() & MKEngine::EventCategoryWindow) != 0) {
		}
	}

	Application::Application()
	{
		MK_LOG_TRACE("Application created");

		PlatformBackend::Initialize();

		PlatformBackend::s_CurrentBackend->SetEventCallback(MK_BIND_EVENT_FN(Application::DispatchEvents));

		PushLayer(new WindowsManagerLayer());
	}

	Application::~Application()
	{
		MK_LOG_TRACE("EL");

		PlatformBackend::Finalize();
	}

	void Application::Run()
	{
		s_Application = this;

		MK_LOG_TRACE("Application run");

		m_Running = true;

		while (m_Running)
		{
			PlatformBackend::s_CurrentBackend->HandleEvents();
			PlatformBackend::s_CurrentBackend->Update();
			PlatformBackend::s_CurrentBackend->Render();
		}
	}
}