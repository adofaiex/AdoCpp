#include "Base.h"

namespace AdoCpp::Event
{
    Event::Event(const Json::Value& data)
    {
        floor = data["floor"].asUInt64();
        active = !data.isMember("active") || toBool(data["active"]);
    }
    StaticEvent::StaticEvent(const Json::Value& data) : Event(data) {}
    DynamicEvent::DynamicEvent(const Json::Value& data) : Event(data)
    {
        if (data.isMember("angleOffset"))
            angleOffset = data["angleOffset"].asDouble();
        if (data.isMember("eventTag"))
            eventTag = cstr2tags(data["eventTag"].asCString());
    }
} // namespace AdoCpp::Event
