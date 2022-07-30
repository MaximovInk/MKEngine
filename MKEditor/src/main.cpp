#include "mkengine.h"
#include <iostream>

using namespace MKEngine;

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
