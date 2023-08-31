#pragma once
#include "KeyCode.h"
#include "mouseCode.h"

namespace MKEngine
{
	namespace Input {

		 int getMouseX();
		 int getMouseY();
		 int getMouseDeltaX();
		 int getMouseDeltaY();
		 int getMouseScrollDelta();
		 void getMousePos(int* x, int* y);

		 bool getMouseButton(int button);
		 bool getMouseButtonDown(int button);
		 bool getMouseButtonUp(int button);

		 bool getKey(KeyCode key);
		 bool getKeyDown(KeyCode key);
		 bool getKeyUp(KeyCode key);

		 void EventUpdate(void* event);
		 void update();
	}
}