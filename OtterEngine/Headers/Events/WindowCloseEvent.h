#pragma once

#include "Events/Event.h"
#include <string>
#include <functional>
#include <iostream>

namespace OtterEngine {

    class WindowCloseEvent : public Event {

    public:
        EventType GetEventType() const override { return EventType::WindowClose; }
        const char* GetName() const override { return "WindowCloseEvent"; }
		static EventType GetStaticType() { return EventType::WindowClose; }
    };
}