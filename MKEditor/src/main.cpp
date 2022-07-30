#include "mkengine.h"
#include <iostream>

class MKEditor : public MKEngine::Application {
public:
	MKEditor()
	{
		MK_TRACE("MKEditor ctor");
	}
};

MKEngine::Application* MKEngine::CreateApplication() {
	return new MKEditor();
};
