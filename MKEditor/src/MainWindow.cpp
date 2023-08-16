#include "MainWindow.h"
#include "MKEngine/Graphics.h"

void MKEditor::MainWindow::FixedUpdate()
{
}

double lastElapsed = 0;

void MKEditor::MainWindow::Update()
{
	double elapsed = MKEngine::Application::DeltaTime;
	if (elapsed < DBL_MIN)
		elapsed = DBL_MIN;
	const int fps = (1.0/((lastElapsed + elapsed)/2.0));

	std::stringstream title;
	title << "MKEngine " << fps << "fps. " << elapsed << "ms.";

	SetTitle(title.str().c_str());

	lastElapsed = elapsed;
}

void MKEditor::MainWindow::Render()
{
	MKEngine::Graphics::DrawTest(1);
}

void MKEditor::MainWindow::OnWindowResize(MKEngine::WindowResizedEvent& event)
{
	Window::OnWindowResize(event);
	MK_LOG_INFO("RESIZE");
}

void MKEditor::MainWindow::OnWindowClose(MKEngine::WindowCloseEvent& event)
{
	MK_LOG_INFO("CLOSE");
	MKEngine::Application::Instance->Close();
}
