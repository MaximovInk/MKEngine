#pragma once
#include "Log.h"
#include "MKEngine/Platform/Application.h"

extern MKEngine::Application* MKEngine::CreateApplication();

int main(int argc, char* argv[])
{
	MK_LOG_INIT();

	auto app = MKEngine::CreateApplication();

	app->Run();

	delete app;
}