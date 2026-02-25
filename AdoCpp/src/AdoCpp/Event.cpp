#include "Event.h"
#include <sstream>
#include "Level.h"

namespace AdoCpp
{
    Event::Event* Event::newEvent(const Json::Value& json)
    {
        const char* eventType = json["eventType"].asCString();
        if (strcmp(eventType, "SetSpeed") == 0)
            return new GamePlay::SetSpeed(json);
        if (strcmp(eventType, "Twirl") == 0)
            return new GamePlay::Twirl(json);
        if (strcmp(eventType, "Pause") == 0)
            return new GamePlay::Pause(json);
        if (strcmp(eventType, "SetHitsound") == 0)
            return new GamePlay::SetHitsound(json);
        if (strcmp(eventType, "SetPlanetRotation") == 0)
            return new GamePlay::SetPlanetRotation(json);

        if (strcmp(eventType, "ColorTrack") == 0)
            return new Track::ColorTrack(json);
        if (strcmp(eventType, "AnimateTrack") == 0)
            return new Track::AnimateTrack(json);
        if (strcmp(eventType, "RecolorTrack") == 0)
            return new Track::RecolorTrack(json);
        if (strcmp(eventType, "PositionTrack") == 0)
            return new Track::PositionTrack(json);
        if (strcmp(eventType, "MoveTrack") == 0)
            return new Track::MoveTrack(json);

        if (strcmp(eventType, "MoveCamera") == 0)
            return new Visual::MoveCamera(json);

        if (strcmp(eventType, "RepeatEvents") == 0)
            return new Modifiers::RepeatEvents(json);

        if (strcmp(eventType, "Hold") == 0)
            return new Dlc::Hold(json);

        return nullptr;
    }
} // namespace AdoCpp
