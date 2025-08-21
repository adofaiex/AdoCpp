#include "Track.h"

#include "AdoCpp/Tile.h"


namespace AdoCpp::Event::Track
{
    ColorTrack::ColorTrack(const rapidjson::Value& data) : StaticEvent(data)
    {
        trackColor = Color(data["trackColor"].GetString());
        secondaryTrackColor = Color(data["secondaryTrackColor"].GetString());
        trackColorAnimDuration = data["trackColorAnimDuration"].GetDouble();
        trackColorType = cstr2trackColorType(data["trackColorType"].GetString());
        trackStyle = cstr2trackStyle(data["trackStyle"].GetString());
        if (data.HasMember("trackColorPulse"))
            trackColorPulse = cstr2trackColorPulse(data["trackColorPulse"].GetString());
        if (data.HasMember("trackPulseLength"))
            trackPulseLength = data["trackPulseLength"].GetUint();
        if (data.HasMember("trackTexture"))
            trackTexture = data["trackTexture"].GetString();
        if (data.HasMember("trackGlowIntensity"))
            trackGlowIntensity = data["trackGlowIntensity"].GetDouble();
    }
    std::unique_ptr<rapidjson::Value> ColorTrack::intoJson(rapidjson::Document::AllocatorType& alloc) const
    {
        auto val = std::make_unique<rapidjson::Value>(rapidjson::kObjectType);
        val->AddMember("floor", floor, alloc).AddMember("eventType", rapidjson::StringRef(name()), alloc);
        if (!active)
            val->AddMember("active", active, alloc);
        rapidjson::Value trackColorValue;
        const std::string trackColorStr = trackColor.toString(false, false, Color::ToStringAlphaMode::Auto);
        trackColorValue.SetString(trackColorStr.c_str(), trackColorStr.length(), alloc);
        val->AddMember("trackColor", trackColorValue, alloc);
        rapidjson::Value secondaryTrackColorValue;
        const std::string secondaryTrackColorStr = trackColor.toString(false, false, Color::ToStringAlphaMode::Auto);
        secondaryTrackColorValue.SetString(secondaryTrackColorStr.c_str(), secondaryTrackColorStr.length(), alloc);
        val->AddMember("secondaryTrackColor", secondaryTrackColorValue, alloc);
        autoRemoveDecimalPart(*val, "trackColorAnimDuration", trackColorAnimDuration, alloc);
        val->AddMember("trackColorType", rapidjson::StringRef(trackColorType2cstr(trackColorType)), alloc);
        val->AddMember("trackColorPulse", rapidjson::StringRef(trackColorPulse2cstr(trackColorPulse)), alloc);
        val->AddMember("trackPulseLength", trackPulseLength, alloc);
        val->AddMember("trackStyle", rapidjson::StringRef(trackStyle2cstr(trackStyle)), alloc);
        rapidjson::Value trackTextureValue;
        trackTextureValue.SetString(trackTexture.c_str(), trackTexture.length(), alloc);
        val->AddMember("trackTexture", trackTextureValue, alloc);
        autoRemoveDecimalPart(*val, "trackGlowIntensity", trackGlowIntensity, alloc);
        return val;
    }
    PositionTrack::PositionTrack(const rapidjson::Value& data) : StaticEvent(data)
    {
        if (data.HasMember("positionOffset"))
        {
            const auto posOff = data["positionOffset"].GetArray();
            if (!posOff[0].IsNull())
                positionOffset.x = posOff[0].GetDouble();
            if (!posOff[1].IsNull())
                positionOffset.y = posOff[1].GetDouble();
        }
        relativeTo = data.HasMember("relativeTo") ? RelativeIndex(data["relativeTo"]) : RelativeIndex(0, ThisTile);
        rotation = data.HasMember("rotation") ? data["rotation"].GetDouble() : 0;
        scale = data.HasMember("scale") ? data["scale"].GetDouble() : 100;
        opacity = data.HasMember("opacity") ? data["opacity"].GetDouble() : 100;
        justThisTile = data.HasMember("justThisTile") ? toBool(data["justThisTile"]) : false;
        editorOnly = data.HasMember("editorOnly") ? toBool(data["editorOnly"]) : false;
        if (data.HasMember("stickToFloors"))
            stickToFloors = toBool(data["stickToFloors"]);
    }
    std::unique_ptr<rapidjson::Value> PositionTrack::intoJson(rapidjson::Document::AllocatorType& alloc) const
    {
        auto val = std::make_unique<rapidjson::Value>(rapidjson::kObjectType);
        val->AddMember("floor", floor, alloc).AddMember("eventType", rapidjson::StringRef(name()), alloc);
        if (!active)
            val->AddMember("active", active, alloc);
        {
            rapidjson::Value posOffVal(rapidjson::kArrayType);
            if (static_cast<int>(positionOffset.x) == positionOffset.x)
                posOffVal.PushBack(static_cast<int>(positionOffset.x), alloc);
            else
                posOffVal.PushBack(positionOffset.x, alloc);
            if (static_cast<int>(positionOffset.y) == positionOffset.y)
                posOffVal.PushBack(static_cast<int>(positionOffset.y), alloc);
            else
                posOffVal.PushBack(positionOffset.y, alloc);
            val->AddMember("positionOffset", posOffVal, alloc);
        }
        val->AddMember("relativeTo", *relativeTo.intoJson(alloc), alloc);
        autoRemoveDecimalPart(*val, "rotation", rotation, alloc);
        autoRemoveDecimalPart(*val, "scale", scale, alloc);
        autoRemoveDecimalPart(*val, "opacity", opacity, alloc);
        val->AddMember("justThisTile", justThisTile, alloc);
        val->AddMember("editorOnly", editorOnly, alloc);
        if (stickToFloors)
            val->AddMember("stickToFloors", *stickToFloors, alloc);
        return val;
    }
    MoveTrack::MoveTrack(const rapidjson::Value& data) : DynamicEvent(data)
    {
        startTile = RelativeIndex(data["startTile"]);
        endTile = RelativeIndex(data["endTile"]);
        duration = data["duration"].GetDouble();
        if (data.HasMember("positionOffset"))
        {
            if (!data["positionOffset"][0].IsNull())
                positionOffset.first = data["positionOffset"][0].GetDouble();
            if (!data["positionOffset"][1].IsNull())
                positionOffset.second = data["positionOffset"][1].GetDouble();
        }
        if (data.HasMember("rotationOffset"))
            rotationOffset = data["rotationOffset"].GetDouble();
        if (data.HasMember("scale"))
        {
            if (data["scale"].IsArray())
            {
                if (!data["scale"][0].IsNull())
                    scale.first = data["scale"][0].GetDouble();
                if (!data["scale"][1].IsNull())
                    scale.second = data["scale"][1].GetDouble();
            }
            else
            {
                scale.first = scale.second = data["scale"].GetDouble();
            }
        }
        if (data.HasMember("opacity"))
            opacity = data["opacity"].GetDouble();
        ease = cstr2easing(data["ease"].GetString());
    }
    std::unique_ptr<rapidjson::Value> MoveTrack::intoJson(rapidjson::Document::AllocatorType& alloc) const
    {
        auto val = std::make_unique<rapidjson::Value>(rapidjson::kObjectType);
        val->AddMember("floor", floor, alloc).AddMember("eventType", rapidjson::StringRef(name()), alloc);
        if (!active)
            val->AddMember("active", active, alloc);
        val->AddMember("startTile", *startTile.intoJson(alloc), alloc);
        val->AddMember("endTile", *endTile.intoJson(alloc), alloc);
        autoRemoveDecimalPart(*val, "duration", duration, alloc);
        // ReSharper disable once CppLocalVariableMayBeConst
        if (auto op = optionalPoint2json(positionOffset, alloc); op)
            val->AddMember("positionOffset", *op, alloc);
        if (rotationOffset)
            autoRemoveDecimalPart(*val, "rotationOffset", *rotationOffset, alloc);
        if (auto op = optionalPoint2json(scale, alloc); op)
            val->AddMember("scale", *op, alloc);
        if (opacity)
            autoRemoveDecimalPart(*val, "opacity", *opacity, alloc);
        autoRemoveDecimalPart(*val, "angleOffset", angleOffset, alloc);
        val->AddMember("ease", rapidjson::StringRef(easing2cstr(ease)), alloc);
        addTag(*val, eventTag, alloc);
        return val;
    }
    AnimateTrack::AnimateTrack(const rapidjson::Value& data) : StaticEvent(data)
    {
        if (data.HasMember("trackAnimation"))
            trackAnimation = cstr2trackAnimation(data["trackAnimation"].GetString());
        beatsAhead = data["beatsAhead"].GetDouble();
        if (data.HasMember("trackDisappearAnimation"))
            trackDisappearAnimation = cstr2trackDisappearAnimation(data["trackDisappearAnimation"].GetString());
        beatsBehind = data["beatsBehind"].GetDouble();
    }
    std::unique_ptr<rapidjson::Value> AnimateTrack::intoJson(rapidjson::Document::AllocatorType& alloc) const
    {
        auto val = std::make_unique<rapidjson::Value>(rapidjson::kObjectType);
        val->AddMember("floor", floor, alloc).AddMember("eventType", rapidjson::StringRef(name()), alloc);
        if (!active)
            val->AddMember("active", active, alloc);
        if (trackAnimation)
            val->AddMember("trackAnimation", rapidjson::StringRef(trackAnimation2cstr(*trackAnimation)), alloc);
        autoRemoveDecimalPart(*val, "beatsAhead", beatsAhead, alloc);
        if (trackDisappearAnimation)
            val->AddMember("trackDisappearAnimation",
                          rapidjson::StringRef(trackDisappearAnimation2cstr(*trackDisappearAnimation)), alloc);
        autoRemoveDecimalPart(*val, "beatsBehind", beatsBehind, alloc);
        return val;
    }
    RecolorTrack::RecolorTrack(const rapidjson::Value& data) : DynamicEvent(data)
    {
        startTile = RelativeIndex(data["startTile"]);
        endTile = RelativeIndex(data["endTile"]);
        if (data.HasMember("duration"))
            duration = data["duration"].GetDouble();
        trackColorType = cstr2trackColorType(data["trackColorType"].GetString());
        trackColor = Color(data["trackColor"].GetString());
        secondaryTrackColor = Color(data["secondaryTrackColor"].GetString());
        trackColorAnimDuration = data["trackColorAnimDuration"].GetDouble();
        trackColorPulse = cstr2trackColorPulse(data["trackColorPulse"].GetString());
        trackPulseLength = data["trackPulseLength"].GetUint();
        trackStyle = cstr2trackStyle(data["trackStyle"].GetString());
        if (data.HasMember("gapLength"))
            gapLength = data["gapLength"].GetDouble();
        if (data.HasMember("ease"))
            ease = cstr2easing(data["ease"].GetString());
        if (data.HasMember("trackGlowIntensity"))
            trackGlowIntensity = data["trackGlowIntensity"].GetDouble();
    }
    std::unique_ptr<rapidjson::Value> RecolorTrack::intoJson(rapidjson::Document::AllocatorType& alloc) const
    {
        auto val = std::make_unique<rapidjson::Value>(rapidjson::kObjectType);
        val->AddMember("floor", floor, alloc).AddMember("eventType", rapidjson::StringRef(name()), alloc);
        if (!active)
            val->AddMember("active", active, alloc);
        val->AddMember("startTile", *startTile.intoJson(alloc), alloc);
        val->AddMember("endTile", *endTile.intoJson(alloc), alloc);
        if (duration)
            autoRemoveDecimalPart(*val, "duration", *duration, alloc);
        val->AddMember("trackColorType", rapidjson::StringRef(trackColorType2cstr(trackColorType)), alloc);
        rapidjson::Value trackColorValue;
        const std::string trackColorStr = trackColor.toString(false, false, Color::ToStringAlphaMode::Auto);
        trackColorValue.SetString(trackColorStr.c_str(), trackColorStr.length(), alloc);
        val->AddMember("trackColor", trackColorValue, alloc);
        rapidjson::Value secondaryTrackColorValue;
        const std::string secondaryTrackColorStr = trackColor.toString(false, false, Color::ToStringAlphaMode::Auto);
        secondaryTrackColorValue.SetString(secondaryTrackColorStr.c_str(), secondaryTrackColorStr.length(), alloc);
        val->AddMember("secondaryTrackColor", secondaryTrackColorValue, alloc);
        autoRemoveDecimalPart(*val, "trackColorAnimDuration", trackColorAnimDuration, alloc);
        val->AddMember("trackColorPulse", rapidjson::StringRef(trackColorPulse2cstr(trackColorPulse)), alloc);
        val->AddMember("trackPulseLength", trackPulseLength, alloc);
        val->AddMember("trackStyle", rapidjson::StringRef(trackStyle2cstr(trackStyle)), alloc);
        autoRemoveDecimalPart(*val, "trackColorAnimDuration", trackColorAnimDuration, alloc);
        autoRemoveDecimalPart(*val, "gapLength", gapLength, alloc);
        val->AddMember("ease", rapidjson::StringRef(easing2cstr(ease)), alloc);
        autoRemoveDecimalPart(*val, "trackGlowIntensity", trackGlowIntensity, alloc);
        addTag(*val, eventTag, alloc);
        autoRemoveDecimalPart(*val, "angleOffset", angleOffset, alloc);
        return val;
    }
} // namespace AdoCpp::Event::Track
