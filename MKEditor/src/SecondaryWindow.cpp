#include "SecondaryWindow.h"

#include "MKEngine/Graphics.h"

namespace MKEditor {

	void SecondaryWindow::FixedUpdate()
	{
	}

	double lastElapsed = 0;

	void SecondaryWindow::Update()
	{
		double elapsed = MKEngine::Application::DeltaTime;
		if (elapsed < DBL_MIN)
			elapsed = DBL_MIN;
		const int fps = (1.0 / ((lastElapsed + elapsed) / 2.0));

		std::stringstream title;
		title << "MKEngine " << fps << "fps. " << elapsed << "ms.";

		SetTitle(title.str().c_str());

		lastElapsed = elapsed;
	}

	void SecondaryWindow::Render()
	{
		//MKEngine::Graphics::DrawTest(0);
	}

	void SecondaryWindow::OnWindowResize(MKEngine::WindowResizedEvent& event)
	{
		Window::OnWindowResize(event);
		MK_LOG_INFO("RESIZE");
	}

	void SecondaryWindow::OnWindowClose(MKEngine::WindowCloseEvent& event)
	{
		MK_LOG_INFO("CLOSE");

		MKEngine::Application::CloseWindow(this);
	}

}
