#include "Dlc.h"
#include <cassert>

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
    MultiPlanet::MultiPlanet(const Json::Value& data) : StaticEvent(data)
    {
        std::string planets = data["planets"].asString();
        if (planets == "ThreePlanets") three = true;
        else if (planets == "TwoPlanets") three = false;
        else assert(0 && "MultiPlanet Error");
    }
    Json::Value MultiPlanet::intoJson() const
    {
        Json::Value val(Json::objectValue);
        val["floor"] = floor;
        val["eventType"] = name();
        val["planets"] = three ? "ThreePlanets" : "TwoPlanets";
        return val;
    }
} // namespace AdoCpp::Event::Dlc
