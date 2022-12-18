#pragma once
#include "mkengine.h"

namespace MKEditor {
	class MainWindow : public MKEngine::Window {

	public:
		using MKEngine::Window::Window;

		MainWindow(const MKEngine::WindowSettings& settings) : Window(settings)
		{
			MK_LOG_INFO("main");
		}

		virtual void FixedUpdate() override;
		virtual void Update() override;
		virtual void Render() override;
		virtual void OnWindowResize(MKEngine::WindowResizedEvent& event) override;
		virtual void OnWindowClose(MKEngine::WindowCloseEvent& event) override;

	};

}