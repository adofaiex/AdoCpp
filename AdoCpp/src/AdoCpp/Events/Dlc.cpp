#include "Dlc.h"

namespace AdoCpp::Event::Dlc
{
    Hold::Hold(const Json::Value& data) : StaticEvent(data)
    {
        duration = data["duration"].asDouble();
        distanceMultiplier = data["distanceMultiplier"].asDouble();
        landingAnimation = data["landingAnimation"].asBool();
    }
    Json::Value Hold::intoJson() const
    {
        Json::Value val(Json::objectValue);
        val["floor"] = floor;
        val["eventType"] = name();
        autoRemoveDecimalPart(val, "duration", duration);
        autoRemoveDecimalPart(val, "distanceMultiplier", distanceMultiplier);
        val["landingAnimation"] = landingAnimation;
        return val;
    }
} // namespace AdoCpp::Event::Dlc
