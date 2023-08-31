#include "mkpch.h"

#include "input.h"

#include <SDL_events.h>
#include <SDL_keycode.h>
#include <SDL_stdinc.h>

#include "MKEngine/Core/Log.h"

namespace MKEngine {

	int x = 0, y = 0;
	int lastX, lastY;
	int mouseScroll;

	Uint8* keyboardState = nullptr;
	int keyCount = 0;
	Uint8* lastKeyboardState;

	Uint32 mouseState = 0;
	Uint32 lastMouseState;

	int Input::getMouseX()
	{
		return x;
	}

	int Input::getMouseY()
	{
		return y;
	}

	int Input::getMouseDeltaX()
	{
		return x - lastX;
	}

	int Input::getMouseDeltaY()
	{
		return y - lastY;
	}

	int Input::getMouseScrollDelta()
	{
		return mouseScroll;
	}

	void Input::getMousePos(int* _x, int* _y)
	{
		*_x = x;
		*_y = y;
	}

	bool Input::getMouseButton(int button)
	{
		return (mouseState & button) != 0;
	}

	bool Input::getMouseButtonDown(int button)
	{
		return ((mouseState & button) != 0) && ((lastMouseState & button) == 0);
	}

	bool Input::getMouseButtonUp(int button)
	{
		return ((mouseState & button) == 0) && ((lastMouseState & button) != 0);
	}

	bool Input::getKey(KeyCode key)
	{
		return keyboardState[key];
	}

	bool Input::getKeyDown(KeyCode key)
	{
		return keyboardState[key] && !lastKeyboardState[key];
	}

	bool Input::getKeyUp(KeyCode key)
	{
		return !keyboardState[key] && lastKeyboardState[key];
	}

	void Input::EventUpdate(void* event) {
		const auto unpackedEvent = static_cast<SDL_Event*>(event);

		if (unpackedEvent->type == SDL_MOUSEWHEEL && unpackedEvent->wheel.y != 0)
		{
			mouseScroll = unpackedEvent->wheel.y;
		}

	}

	void Input::update()
	{
		
		mouseScroll = 0;

		for (int i = 0; i < keyCount; i++) {
			lastKeyboardState[i] = keyboardState[i];
		}

		lastMouseState = mouseState;
		lastX = x;
		lastY = y;


		int kCount = 0;
		auto k = SDL_GetKeyboardState(&kCount);
		if (keyCount != kCount) {
			delete[] keyboardState;
			delete[] lastKeyboardState;
			keyCount = kCount;
			keyboardState = new Uint8[kCount];
			lastKeyboardState = new Uint8[kCount];
		}

		for (int i = 0; i < kCount; i++) {
			keyboardState[i] = k[i];
		}

		mouseState = SDL_GetMouseState(&x, &y);
	}
}
