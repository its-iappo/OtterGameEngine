#pragma once

#include <string>
#include <functional>
#include <iostream>

namespace OtterEngine {

    enum class EventType {
        None = 0,
        WindowClose
    };

    class Event {
    public:
        virtual EventType GetEventType() const = 0;
        virtual const char* GetName() const = 0;
        virtual std::string ToString() const { return GetName(); }
        
        static EventType GetStaticType() { return EventType::None; }
        
        virtual ~Event() = default;
    };

    using EventCallback = std::function<void(Event&)>;
}
