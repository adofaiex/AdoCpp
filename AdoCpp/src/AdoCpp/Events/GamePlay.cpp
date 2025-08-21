#include "GamePlay.h"
#include <cstring>
#include "rapidjson/document.h"

namespace AdoCpp::Event::GamePlay
{
    SetSpeed::SetSpeed(const rapidjson::Value& data) : DynamicEvent(data)
    {
        beatsPerMinute = data["beatsPerMinute"].GetDouble();
        if (!data.HasMember("speedType"))
        {
            speedType = SpeedType::Bpm;
            return;
        }
        if (strcmp(data["speedType"].GetString(), "Bpm") == 0)
            speedType = SpeedType::Bpm;
        else if (strcmp(data["speedType"].GetString(), "Multiplier") == 0)
            speedType = SpeedType::Multiplier;
        else
            throw std::invalid_argument("Invalid speedType");
        bpmMultiplier = data["bpmMultiplier"].GetDouble();
    }
    std::unique_ptr<rapidjson::Value> SetSpeed::intoJson(rapidjson::Document::AllocatorType& alloc) const
    {
        auto val = std::make_unique<rapidjson::Value>(rapidjson::kObjectType);
        val->AddMember("floor", floor, alloc).AddMember("eventType", rapidjson::StringRef(name()), alloc);

        if (speedType == SpeedType::Bpm)
            val->AddMember("speedType", "Bpm", alloc);
        else
            val->AddMember("speedType", "Multiplier", alloc);
        autoRemoveDecimalPart(*val, "beatsPerMinute", beatsPerMinute, alloc);
        autoRemoveDecimalPart(*val, "bpmMultiplier", bpmMultiplier, alloc);
        autoRemoveDecimalPart(*val, "angleOffset", angleOffset, alloc);
        return val;
    }
    Twirl::Twirl(const rapidjson::Value& data) : StaticEvent(data) {}

    std::unique_ptr<rapidjson::Value> Twirl::intoJson(rapidjson::Document::AllocatorType& alloc) const
    {
        auto val = std::make_unique<rapidjson::Value>(rapidjson::kObjectType);
        val->AddMember("floor", floor, alloc).AddMember("eventType", rapidjson::StringRef(name()), alloc);
        return val;
    }
    Pause::Pause(const rapidjson::Value& data) : StaticEvent(data)
    {
        duration = data["duration"].GetDouble();
        countdownTicks = data["countdownTicks"].GetDouble();
        if (data["angleCorrectionDir"].IsString())
        {
            if (const char* const str = data["angleCorrectionDir"].GetString(); strcmp(str, "Backward") == 0)
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
            if (const int dir = data["angleCorrectionDir"].GetInt(); -1 <= dir && dir <= 1)
                angleCorrectionDir = static_cast<AngleCorrectionDir>(data["angleCorrectionDir"].GetInt());
            else
                throw std::invalid_argument("Invalid angleCorrectionDir");
        }
    }
    std::unique_ptr<rapidjson::Value> Pause::intoJson(rapidjson::Document::AllocatorType& alloc) const
    {
        auto val = std::make_unique<rapidjson::Value>(rapidjson::kObjectType);
        val->AddMember("floor", floor, alloc).AddMember("eventType", rapidjson::StringRef(name()), alloc);
        autoRemoveDecimalPart(*val, "duration", duration, alloc);
        autoRemoveDecimalPart(*val, "countdownTicks", countdownTicks, alloc);
        if (angleCorrectionDir == AngleCorrectionDir::None)
            val->AddMember("angleCorrectionDir", "None", alloc);
        else if (angleCorrectionDir == AngleCorrectionDir::Forward)
            val->AddMember("angleCorrectionDir", "Forward", alloc);
        else
            val->AddMember("angleCorrectionDir", "Backward", alloc);
        return val;
    }
    SetHitsound::SetHitsound(const rapidjson::Value& data)
    {
        if (!data.HasMember("gameSound") || !strcmp(data["gameSound"].GetString(), "Hitsound"))
            gameSound = GameSound::Hitsound;
        else if (!strcmp(data["gameSound"].GetString(), "Midspin"))
            gameSound = GameSound::Midspin;
        else
            throw std::invalid_argument("Invalid gameSound");
        hitsound = cstr2hitsound(data["hitsound"].GetString());
        hitsoundVolume = data["hitsoundVolume"].GetDouble();
    }
    std::unique_ptr<rapidjson::Value> SetHitsound::intoJson(rapidjson::Document::AllocatorType& alloc) const
    {
        auto val = std::make_unique<rapidjson::Value>(rapidjson::kObjectType);
        val->AddMember("floor", floor, alloc).AddMember("eventType", rapidjson::StringRef(name()), alloc);

        if (gameSound == GameSound::Hitsound)
            val->AddMember("gameSound", "Hitsound", alloc);
        else if (gameSound == GameSound::Midspin)
            val->AddMember("gameSound", "Midspin", alloc);
        val->AddMember("hitsound", rapidjson::StringRef(hitsound2cstr(hitsound)), alloc);
        autoRemoveDecimalPart(*val, "hitsoundVolume", hitsoundVolume, alloc);
        return val;
    }

    SetPlanetRotation::SetPlanetRotation(const rapidjson::Value& data) : StaticEvent(data)
    {
        ease = cstr2easing(data["ease"].GetString());
        easeParts = data["easeParts"].GetUint64();
        easePartBehavior =
            !data.HasMember("easePartBehavior") || !strcmp(data["easePartBehavior"].GetString(), "Repeat")
            ? EasePartBehavior::Repeat
            : EasePartBehavior::Mirror;
    }
    std::unique_ptr<rapidjson::Value> SetPlanetRotation::intoJson(rapidjson::Document::AllocatorType& alloc) const
    {
        auto val = std::make_unique<rapidjson::Value>(rapidjson::kObjectType);
        val->AddMember("floor", floor, alloc).AddMember("eventType", rapidjson::StringRef(name()), alloc);
        if (!active)
            val->AddMember("active", active, alloc);
        val->AddMember("ease", rapidjson::StringRef(easing2cstr(ease)), alloc);
        val->AddMember("easeParts", easeParts, alloc);
        if (easePartBehavior == EasePartBehavior::Repeat)
            val->AddMember("easePartBehavior", "Repeat", alloc);
        else if (easePartBehavior == EasePartBehavior::Mirror)
            val->AddMember("easePartBehavior", "Mirror", alloc);
        return val;
    }
} // namespace AdoCpp::Event::GamePlay
