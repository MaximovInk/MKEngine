#pragma once
#include "Event.h"

namespace MKEngine {
	//https://github.com/TheCherno/Hazel/issues/85
	class EventDispatcher {

	private:
		MKEngine::Event& m_Event;

	public:
		EventDispatcher(Event& event)
			: m_Event(event) {};

		template<typename T, typename  F>
		bool Dispatch(const F& func) {

			if (m_Event.GetEventType() == T::GetStaticType()) {
				m_Event.Handled |= func(static_cast<T&>(m_Event));
				return true;
			}
			return false;
		}
	};
}