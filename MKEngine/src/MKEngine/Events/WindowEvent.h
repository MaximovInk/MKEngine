#pragma once

#include "Event.h"

namespace MKEngine {
	class WindowResizedEvent : public Event
	{

	public:
		WindowResizedEvent(int width, int height, MKEngine::Window* window)
			: m_Width(width), m_Height(height), m_Window(window) {
		}

		std::string ToString() const override {
			std::stringstream ss;
			ss << "WindowResizeEvent: " << m_Width << ", " << m_Height;
			return ss.str();
		}

		float GetWidth() const { return m_Width; }
		float GetHeight() const { return m_Height; }
		Window* GetWindow() const { return m_Window;  }

		EVENT_CLASS_TYPE(WindowResized)
		EVENT_CLASS_CATEGORY(EventCategoryWindow)
	private:
		int m_Width;
		int m_Height;
		Window* m_Window;

	};

	class WindowCloseEvent : public Event
	{
	public:
		WindowCloseEvent(MKEngine::Window* window)
			:  m_Window(window) {
		}

		Window* GetWindow() const { return m_Window; }

		EVENT_CLASS_TYPE(WindowClosed)
		EVENT_CLASS_CATEGORY(EventCategoryWindow)

		std::string ToString() const override {
			std::stringstream ss;
			ss << "WindowCloseEvent";
			return ss.str();
		}
	private:
		Window* m_Window;
	};
}