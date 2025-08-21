#include "Visual.h"

namespace AdoCpp::Event::Visual
{
    MoveCamera::MoveCamera(const rapidjson::Value& data) : DynamicEvent(data)
    {
        duration = data["duration"].GetDouble();
        if (data.HasMember("relativeTo"))
            relativeTo = cstr2relativeToCamera(data["relativeTo"].GetString());
        if (data.HasMember("position"))
        {
            if (!data["position"][0].IsNull())
                position.first = data["position"][0].GetDouble();
            if (!data["position"][1].IsNull())
                position.second = data["position"][1].GetDouble();
        }
        if (data.HasMember("rotation"))
            rotation = data["rotation"].GetDouble();
        if (data.HasMember("zoom"))
            zoom = data["zoom"].GetDouble();
        ease = cstr2easing(data["ease"].GetString());
    }
    std::unique_ptr<rapidjson::Value> MoveCamera::intoJson(rapidjson::Document::AllocatorType& alloc) const
    {
        auto val = std::make_unique<rapidjson::Value>(rapidjson::kObjectType);
        val->AddMember("floor", floor, alloc).AddMember("eventType", rapidjson::StringRef(name()), alloc);
        if (!active)
            val->AddMember("active", active, alloc);
        autoRemoveDecimalPart(*val, "duration", duration, alloc);
        if (relativeTo)
            val->AddMember("relativeTo", rapidjson::StringRef(relativeToCamera2cstr(*relativeTo)), alloc);
        // ReSharper disable once CppLocalVariableMayBeConst
        if (auto op = optionalPoint2json(position, alloc); op)
            val->AddMember("position", *op, alloc);
        if (rotation)
            autoRemoveDecimalPart(*val, "rotation", *rotation, alloc);
        if (zoom)
            autoRemoveDecimalPart(*val, "zoom", *zoom, alloc);
        val->AddMember("ease", rapidjson::StringRef(easing2cstr(ease)), alloc);
        autoRemoveDecimalPart(*val, "angleOffset", angleOffset, alloc);
        addTag(*val, eventTag, alloc);
        return val;
    }
} // namespace AdoCpp::Event::Visual
