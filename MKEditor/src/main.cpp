#include "mkengine.h"
#include "MKEngine/Core/entryPoint.h"

#include "MainWindow.h"

#include <iostream>

using namespace MKEngine;

namespace MKEditor {

	class MKEditor : public MKEngine::Application {
	public:
		MKEditor()
		{
			auto settings = MKEngine::WindowSettings("1");
			settings.resizable = true;

			MainWindow* wnd = new MainWindow(settings);
			MainWindow* wnd1 = new MainWindow(MKEngine::WindowSettings("2"));
			MainWindow* wnd2 = new MainWindow(MKEngine::WindowSettings("3"));
		}
	};

}

MKEngine::Application* MKEngine::CreateApplication() {
	return new MKEditor::MKEditor();
};