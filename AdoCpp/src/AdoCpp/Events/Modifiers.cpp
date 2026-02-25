#include "Modifiers.h"

namespace AdoCpp::Event::Modifiers
{
    RepeatEvents::RepeatEvents(const Json::Value& data) : Event(data)
    {
        if (data.isMember("repeatType") && !strcmp(data["repeatType"].asCString(), "Floor"))
            repeatType = RepeatType::Floor;
        else
            repeatType = RepeatType::Beat;
        repetitions = data["repetitions"].asUInt64();
        floorCount = data.isMember("floorCount") ? data["floorCount"].asUInt64() : 0;
        interval = data["interval"].asDouble();
        executeOnCurrentFloor = data.isMember("executeOnCurrentFloor") ? toBool(data["executeOnCurrentFloor"]) : false;
        tag = cstr2tags(data["tag"].asCString());
    }
    Json::Value RepeatEvents::intoJson() const
    {
        Json::Value val(Json::objectValue);
        val["floor"] = floor;
        val["eventType"] = name();
        if (!active)
            val["active"] = active;
        if (repeatType == RepeatType::Floor)
            val["repeatType"] = "Floor";
        else if (repeatType == RepeatType::Beat)
            val["repeatType"] = "Beat";
        val["repetitions"] = repetitions;
        val["floorCount"] = floorCount;
        autoRemoveDecimalPart(val, "interval", interval);
        val["executeOnCurrentFloor"] = executeOnCurrentFloor;
        addTag(val, tag, true);
        return val;
    }
} // namespace AdoCpp::Event::Modifiers
