#include "Modifiers.h"

namespace AdoCpp::Event::Modifiers
{
    RepeatEvents::RepeatEvents(const rapidjson::Value& data) : Event(data)
    {
        if (data.HasMember("repeatType") && !strcmp(data["repeatType"].GetString(), "Floor"))
            repeatType = RepeatType::Floor;
        else
            repeatType = RepeatType::Beat;
        repetitions = data["repetitions"].GetUint64();
        floorCount = data.HasMember("floorCount") ? data["floorCount"].GetUint64() : 0;
        interval = data["interval"].GetDouble();
        executeOnCurrentFloor = data.HasMember("executeOnCurrentFloor") ? toBool(data["executeOnCurrentFloor"]) : false;
        tag = cstr2tags(data["tag"].GetString());
    }
    std::unique_ptr<rapidjson::Value> RepeatEvents::intoJson(rapidjson::Document::AllocatorType& alloc) const
    {
        auto val = std::make_unique<rapidjson::Value>(rapidjson::kObjectType);
        val->AddMember("floor", floor, alloc).AddMember("eventType", rapidjson::StringRef(name()), alloc);
        if (!active)
            val->AddMember("active", active, alloc);
        if (repeatType == RepeatType::Floor)
            val->AddMember("repeatType", "Floor", alloc);
        else if (repeatType == RepeatType::Beat)
            val->AddMember("repeatType", "Beat", alloc);
        val->AddMember("repetitions", repetitions, alloc);
        val->AddMember("floorCount", floorCount, alloc);
        autoRemoveDecimalPart(*val, "interval", interval, alloc);
        val->AddMember("executeOnCurrentFloor", executeOnCurrentFloor, alloc);
        addTag(*val, tag, alloc, true);
        return val;
    }
} // namespace AdoCpp::Event::Modifiers
