#include <mkpch.h>
#include "Application.h"
#include "MKEngine/Core/Log.h"
#include "MKEngine/Events/EventSystem.h"
#include "MKEngine/Platform/PlatformBackend.h"
#include "MKEngine/WindowsManager/WindowsManagerLayer.h"

namespace MKEngine {

	Application* Application::Instance;

	bool Application::OnMouseMotion(MouseMovedEvent& e)
	{
		return true;
	}

	void Application::PushLayer(Layer* layer)
	{
		m_layerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* layer)
	{
		m_layerStack.PushOverlay(layer);
		layer->OnAttach();
	}

	void Application::CloseWindow(const Window* window)
	{
		PlatformBackend::CurrentBackend->DestroyWindow(window);
		delete window;
	}

	void Application::Close()
	{
		m_running = false;
	}

	void Application::DispatchEvents(Event& e) {
		EventDispatcher dispatcher(e);

		dispatcher.Dispatch<MouseMovedEvent>(MK_BIND_EVENT_FN(Application::OnMouseMotion));

		for (auto it = m_layerStack.rbegin(); it != m_layerStack.rend(); ++it)
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
		PlatformBackend::CurrentBackend->SetEventCallback(MK_BIND_EVENT_FN(Application::DispatchEvents));
		RendererAPI::Make();

		PushLayer(new WindowsManagerLayer());
	}

	Application::~Application()
	{
		RendererAPI::Destroy();
		PlatformBackend::Finalize();
	}

	double Application::DeltaTime;
	static uint32_t current;
	void Application::Run()
	{
		Instance = this;

		MK_LOG_TRACE("Application run");

		m_running = true;

		current = MKEngine::PlatformBackend::GetTicks();
		uint32_t last = current;

		while (m_running)
		{
			current = MKEngine::PlatformBackend::GetTicks();
			const double elapsed = (current - last) / static_cast<double>(
				MKEngine::PlatformBackend::GetPerformanceFrequency());

			PlatformBackend::CurrentBackend->HandleEvents();
			MKEngine::PlatformBackend::Update();
			MKEngine::PlatformBackend::Render();

			last = current;
			DeltaTime = elapsed;
		}
	}
}