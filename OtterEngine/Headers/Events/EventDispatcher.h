#pragma once

#include "Events/Event.h"

namespace OtterEngine {

	class EventDispatcher {

	private:
		Event& rEvent;

	public:
		EventDispatcher(Event& event) : rEvent(event) {}

		template<typename T, typename F>
		bool Dispatch(const F& function) {
			if (rEvent.GetEventType() == T::GetStaticType()) {
				function(static_cast<T&>(rEvent));
				return true;
			}
			return false;
		}
	};
}