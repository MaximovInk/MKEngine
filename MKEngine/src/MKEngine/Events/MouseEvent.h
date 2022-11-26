#pragma once
#include "Event.h"

namespace MKEngine {
	class MouseMovedEvent : public Event
	{
	public:

		MouseMovedEvent(int x, int y)
			: m_MouseX(x), m_MouseY(y) {}

		std::string ToString() const override {
			std::stringstream ss;
			ss << "MouseMovedEvent: " << m_MouseX << ", " << m_MouseY;
			return ss.str();
		}

		float GetX() const { return m_MouseX; }
		float GetY() const { return m_MouseY; }

		EVENT_CLASS_TYPE(MouseMoved)
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

	private:
		int m_MouseX;
		int m_MouseY;
	};
}