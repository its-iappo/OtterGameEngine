#pragma once

#include <string>
#include <iostream>
#include <functional>

#include "Events/Event.h"

namespace OtterEngine {

    class WindowCloseEvent : public Event {

    public:
        EventType GetEventType() const override { return EventType::WindowClose; }
        const char* GetName() const override { return "WindowCloseEvent"; }
		static EventType GetStaticType() { return EventType::WindowClose; }
    };
}