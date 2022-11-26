#pragma once
#include "mkpch.h"
#include "MKEngine/Core/core.h"
//#include "MKEngine/Platform/Window.h"

namespace MKEngine {

	enum class EventType {
		None = 0,
		WindowCloseed,WindowResized,WindowFocused,WindowLostedFoucs,WindowMoved,
		AppUpdate, AppFixedUpdate, AppRender,
		KeyPressed,KeyReleased,
		MouseMoved, MouseButtonPressed, MouseButtonReleased, MouseScrolled
	};

	enum EventCategory
	{
		None = 0,
		EventCategoryApplication = BIT(0),
		EventCategoryWindow = BIT(1),
		EventCategoryInput = BIT(2),
		EventCategoryKeyboard = BIT(3),
		EventCategoryMouse = BIT(4),
		EventCategoryMouseButton = BIT(5)
	};

#define MK_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

#define EVENT_CLASS_TYPE(type) static EventType GetStaticType() { return EventType::type; }\
								virtual EventType GetEventType() const override { return GetStaticType(); }\
								virtual const char* GetName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category) virtual int GetCategoryFlags() const override { return category; }

	class Window;

	class Event {
	public:
		bool Handled = false;
		//MKEngine::Window* Window;

		virtual const char* GetName() const = 0;
		virtual MKEngine::EventType GetEventType() const = 0;
		virtual int GetCategoryFlags() const = 0;
		virtual std::string ToString() const { return GetName(); };


		bool IsInCategory(EventCategory category)
		{
			return GetCategoryFlags() & category;
		}
	};

	inline std::ostream& operator<<(std::ostream& os, const Event& e)
	{
		return os << e.ToString();
	}
}
