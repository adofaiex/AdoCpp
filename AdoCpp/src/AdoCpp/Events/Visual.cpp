#include "Visual.h"

namespace AdoCpp::Event::Visual
{
    MoveCamera::MoveCamera(const Json::Value& data) : DynamicEvent(data)
    {
        duration = data["duration"].asDouble();
        if (data.isMember("relativeTo"))
            relativeTo = cstr2relativeToCamera(data["relativeTo"].asCString());
        if (data.isMember("position"))
        {
            if (!data["position"][0].isNull())
                position.first = data["position"][0].asDouble();
            if (!data["position"][1].isNull())
                position.second = data["position"][1].asDouble();
        }
        if (data.isMember("rotation"))
            rotation = data["rotation"].asDouble();
        if (data.isMember("zoom"))
            zoom = data["zoom"].asDouble();
        ease = cstr2easing(data["ease"].asCString());
    }
    Json::Value MoveCamera::intoJson() const
    {
        Json::Value val(Json::objectValue);
        val["floor"] = floor;
        val["eventType"] = name();
        if (!active)
            val["active"] = active;
        autoRemoveDecimalPart(val, "duration", duration);
        if (relativeTo)
            val["relativeTo"] = relativeToCamera2cstr(*relativeTo);
        // ReSharper disable once CppLocalVariableMayBeConst
        if (auto op = optionalPoint2json(position); op)
            val["position"] = *op;
        if (rotation)
            autoRemoveDecimalPart(val, "rotation", *rotation);
        if (zoom)
            autoRemoveDecimalPart(val, "zoom", *zoom);
        val["ease"] = easing2cstr(ease);
        autoRemoveDecimalPart(val, "angleOffset", angleOffset);
        addTag(val, eventTag);
        return val;
    }
} // namespace AdoCpp::Event::Visual
