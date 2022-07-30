#pragma once
#include "core.h"

namespace MKEngine {

	class MK_API Application {
	public:
		Application();
		~Application();

		void Run();
	};

	Application* CreateApplication();

}