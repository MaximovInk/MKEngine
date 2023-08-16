#include <iostream>
#include "mkengine.h"
#include "MKEngine/Core/entryPoint.h"
#include "MainWindow.h"
#include "SecondaryWindow.h"

using namespace MKEngine;

namespace MKEditor {

	class MKEditor : public MKEngine::Application {
	public:
		MKEditor()
		{
			auto settings = MKEngine::WindowSettings("1");
			settings.Resizable = true;

			auto wnd = new MainWindow(settings);
			auto* wnd2 = new SecondaryWindow(settings);
		}
	};

}

MKEngine::Application* MKEngine::CreateApplication() {
	return new MKEditor::MKEditor();
};