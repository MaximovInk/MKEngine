#include "mkpch.h"
#include "WindowsManagerLayer.h"

#include "MKEngine/Events/EventSystem.h"

void MKEngine::WindowsManagerLayer::OnEvent(Event& e)
{
	EventDispatcher dispatcher(e);

	dispatcher.Dispatch<WindowResizedEvent>(MK_BIND_EVENT_FN(WindowsManagerLayer::WindowResize));
	dispatcher.Dispatch<WindowCloseEvent>(MK_BIND_EVENT_FN(WindowsManagerLayer::WindowClose));
}

bool MKEngine::WindowsManagerLayer::WindowResize(WindowResizedEvent& e)
{
	e.GetWindow()->OnWindowResize(e);
	return true;
}

bool MKEngine::WindowsManagerLayer::WindowClose(WindowCloseEvent& e)
{
	e.GetWindow()->OnWindowClose(e);
	return true;
}