#include "MainWindow.h"

void MKEditor::MainWindow::FixedUpdate()
{
}

double lastElapsed = 0;

void MKEditor::MainWindow::Update()
{
	double elapsed = MKEngine::Application::DeltaTime;
	if (elapsed < DBL_MIN)
		elapsed = DBL_MIN;
	int FPS = (1.0/((lastElapsed + elapsed)/2.0));

	//MK_LOG_INFO("FPS: {0} MS: {1}", FPS, elapsed);

	std::stringstream title;
	title << "MKEngine " << FPS << "fps. " << elapsed << "ms.";

	SetTitle(title.str().c_str());

	lastElapsed = elapsed;
}

void MKEditor::MainWindow::Render()
{
}

void MKEditor::MainWindow::OnWindowResize(MKEngine::WindowResizedEvent& event)
{
	Window::OnWindowResize(event);
	MK_LOG_INFO("RESIZE");
}

void MKEditor::MainWindow::OnWindowClose(MKEngine::WindowCloseEvent& event)
{
	MK_LOG_INFO("CLOSE");
	//MKEngine::Application::s_Application->CloseWindow(this);
	MKEngine::Application::s_Application->Close();
}
