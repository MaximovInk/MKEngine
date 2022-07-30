#include "mkpch.h"
#include "Application.h"
#include "MKEngine/Core/Log.h"

MKEngine::Application::Application()
{
}

MKEngine::Application::~Application()
{
	MK_TRACE("Application created");
}

void MKEngine::Application::Run()
{
	MK_TRACE("Application run");
}
