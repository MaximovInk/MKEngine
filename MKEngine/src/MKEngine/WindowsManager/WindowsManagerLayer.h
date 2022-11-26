#pragma once

#include "mkengine.h"

namespace MKEngine {
	class WindowsManagerLayer : public Layer
	{
	public:
		virtual void OnEvent(Event& e) override;

	private:
		bool WindowResize(WindowResizedEvent& e);
	};
}