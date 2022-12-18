#include <iostream>
#include "mkengine.h"
#include "MKEngine/Core/entryPoint.h"
#include "MainWindow.h"

using namespace MKEngine;

namespace MKEditor {

	class MKEditor : public MKEngine::Application {
	public:
		MKEditor()
		{
			auto settings = MKEngine::WindowSettings("1");
			settings.resizable = true;

			MainWindow* wnd = new MainWindow(settings);
			//MainWindow* wnd2 = new MainWindow(settings);
		}
	};

}

MKEngine::Application* MKEngine::CreateApplication() {
	return new MKEditor::MKEditor();
};