#pragma once

#include "mkengine.h"

namespace MKEngine {
	class WindowsManagerLayer final : public Layer
	{
	public:
		virtual void OnEvent(Event& e) override;

	private:
		static bool WindowResize(WindowResizedEvent& e);
		static bool WindowClose(WindowCloseEvent& e);
	};
}