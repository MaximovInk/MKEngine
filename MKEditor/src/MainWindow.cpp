#include "MainWindow.h"

void MKEditor::MainWindow::FixedUpdate()
{
}

void MKEditor::MainWindow::Update()
{
}

void MKEditor::MainWindow::Render()
{
}

void MKEditor::MainWindow::OnWindowResize(MKEngine::WindowResizedEvent& event)
{
	MK_LOG_INFO("RESIZE");
}

void MKEditor::MainWindow::OnWindowClose(MKEngine::WindowCloseEvent& event)
{
	MK_LOG_INFO("CLOSE");
	MKEngine::Application::s_Application->CloseWindow(this);
}
