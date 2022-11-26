#include "mkpch.h"
#include "WindowsManagerLayer.h"

#include "MKEngine/Events/EventSystem.h"

void MKEngine::WindowsManagerLayer::OnEvent(Event& e)
{
	//MK_LOG_INFO("WINDOWS MANAGER LAYER");

	EventDispatcher dispatcher(e);

	dispatcher.Dispatch<WindowResizedEvent>(MK_BIND_EVENT_FN(WindowsManagerLayer::WindowResize));

}

bool MKEngine::WindowsManagerLayer::WindowResize(WindowResizedEvent& e)
{
	e.GetWindow()->OnWindowResize(e);
	return true;
}
