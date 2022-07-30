#pragma once
#include "MKEngine/Core/core.h"

namespace MKEngine {

	class Application {
	public:
		Application();
		~Application();

		void Run();
	};

	Application* CreateApplication();

}