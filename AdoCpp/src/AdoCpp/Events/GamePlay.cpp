#include "GamePlay.h"
#include <cstring>

namespace AdoCpp::Event::GamePlay
{
    SetSpeed::SetSpeed(const Json::Value& data) : DynamicEvent(data)
    {
        beatsPerMinute = data["beatsPerMinute"].asDouble();
        if (!data.isMember("speedType"))
        {
            speedType = SpeedType::Bpm;
            return;
        }
        if (strcmp(data["speedType"].asCString(), "Bpm") == 0)
            speedType = SpeedType::Bpm;
        else if (strcmp(data["speedType"].asCString(), "Multiplier") == 0)
            speedType = SpeedType::Multiplier;
        else
            throw std::invalid_argument("Invalid speedType");
        bpmMultiplier = data["bpmMultiplier"].asDouble();
    }
    Json::Value SetSpeed::intoJson() const
    {
        Json::Value val(Json::objectValue);
        val["floor"] = floor;
        val["eventType"] = name();

        if (speedType == SpeedType::Bpm)
            val["speedType"] = "Bpm";
        else
            val["speedType"] = "Multiplier";
        autoRemoveDecimalPart(val, "beatsPerMinute", beatsPerMinute);
        autoRemoveDecimalPart(val, "bpmMultiplier", bpmMultiplier);
        autoRemoveDecimalPart(val, "angleOffset", angleOffset);
        return val;
    }
    Twirl::Twirl(const Json::Value& data) : StaticEvent(data) {}

    Json::Value Twirl::intoJson() const
    {
        Json::Value val(Json::objectValue);
        val["floor"] = floor;
        val["eventType"] = name();
        return val;
    }
    Pause::Pause(const Json::Value& data) : StaticEvent(data)
    {
        duration = data["duration"].asDouble();
        countdownTicks = data["countdownTicks"].asDouble();
        if (data["angleCorrectionDir"].isString())
        {
            if (const char* const str = data["angleCorrectionDir"].asCString(); strcmp(str, "Backward") == 0)
                angleCorrectionDir = AngleCorrectionDir::Backward;
            else if (strcmp(str, "None") == 0)
                angleCorrectionDir = AngleCorrectionDir::None;
            else if (strcmp(str, "Forward") == 0)
                angleCorrectionDir = AngleCorrectionDir::Forward;
            else
                throw std::invalid_argument("Invalid angleCorrectionDir");
        }
        else
        {
            if (const int dir = data["angleCorrectionDir"].asInt(); -1 <= dir && dir <= 1)
                angleCorrectionDir = static_cast<AngleCorrectionDir>(data["angleCorrectionDir"].asInt());
            else
                throw std::invalid_argument("Invalid angleCorrectionDir");
        }
    }
    Json::Value Pause::intoJson() const
    {
        Json::Value val(Json::objectValue);
        val["floor"] = floor;
        val["eventType"] = name();
        autoRemoveDecimalPart(val, "duration", duration);
        autoRemoveDecimalPart(val, "countdownTicks", countdownTicks);
        if (angleCorrectionDir == AngleCorrectionDir::None)
            val["angleCorrectionDir"] = "None";
        else if (angleCorrectionDir == AngleCorrectionDir::Forward)
            val["angleCorrectionDir"] = "Forward";
        else
            val["angleCorrectionDir"] = "Backward";
        return val;
    }
    SetHitsound::SetHitsound(const Json::Value& data)
    {
        if (!data.isMember("gameSound") || !strcmp(data["gameSound"].asCString(), "Hitsound"))
            gameSound = GameSound::Hitsound;
        else if (!strcmp(data["gameSound"].asCString(), "Midspin"))
            gameSound = GameSound::Midspin;
        else
            throw std::invalid_argument("Invalid gameSound");
        hitsound = cstr2hitsound(data["hitsound"].asCString());
        hitsoundVolume = data["hitsoundVolume"].asDouble();
    }
    Json::Value SetHitsound::intoJson() const
    {
        Json::Value val(Json::objectValue);
        val["floor"] = floor;
        val["eventType"] = name();

        if (gameSound == GameSound::Hitsound)
            val["gameSound"] = "Hitsound";
        else if (gameSound == GameSound::Midspin)
            val["gameSound"] = "Midspin";
        val["hitsound"] = hitsound2cstr(hitsound);
        autoRemoveDecimalPart(val, "hitsoundVolume", hitsoundVolume);
        return val;
    }

    SetPlanetRotation::SetPlanetRotation(const Json::Value& data) : StaticEvent(data)
    {
        ease = cstr2easing(data["ease"].asCString());
        easeParts = data["easeParts"].asUInt64();
        easePartBehavior =
            !data.isMember("easePartBehavior") || !strcmp(data["easePartBehavior"].asCString(), "Repeat")
            ? EasePartBehavior::Repeat
            : EasePartBehavior::Mirror;
    }
    Json::Value SetPlanetRotation::intoJson() const
    {
        Json::Value val(Json::objectValue);
        val["floor"] = floor;
        val["eventType"] = name();
        if (!active)
            val["active"] = active;
        val["ease"] = easing2cstr(ease);
        val["easeParts"] = easeParts;
        if (easePartBehavior == EasePartBehavior::Repeat)
            val["easePartBehavior"] = "Repeat";
        else if (easePartBehavior == EasePartBehavior::Mirror)
            val["easePartBehavior"] = "Mirror";
        return val;
    }
} // namespace AdoCpp::Event::GamePlay
