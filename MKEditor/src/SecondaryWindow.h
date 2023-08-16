#pragma once
#include "mkengine.h"

namespace MKEditor {
	class SecondaryWindow final : public MKEngine::Window {

	public:
		using Window::Window;

		explicit SecondaryWindow(const MKEngine::WindowSettings& settings) : Window(settings)
		{
			MK_LOG_INFO("secondary");
		}

		void FixedUpdate() override;
		void Update() override;
		void Render() override;
		void OnWindowResize(MKEngine::WindowResizedEvent& event) override;
		void OnWindowClose(MKEngine::WindowCloseEvent& event) override;

	};

}