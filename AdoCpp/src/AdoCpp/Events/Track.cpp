#include "Track.h"

#include "AdoCpp/Tile.h"


namespace AdoCpp::Event::Track
{
    ColorTrack::ColorTrack(const Json::Value& data) : StaticEvent(data)
    {
        trackColor = Color(data["trackColor"].asString());
        secondaryTrackColor = Color(data["secondaryTrackColor"].asString());
        trackColorAnimDuration = data["trackColorAnimDuration"].asDouble();
        trackColorType = cstr2trackColorType(data["trackColorType"].asCString());
        trackStyle = cstr2trackStyle(data["trackStyle"].asCString());
        if (data.isMember("trackColorPulse"))
            trackColorPulse = cstr2trackColorPulse(data["trackColorPulse"].asCString());
        if (data.isMember("trackPulseLength"))
            trackPulseLength = data["trackPulseLength"].asUInt();
        if (data.isMember("trackTexture"))
            trackTexture = data["trackTexture"].asString();
        if (data.isMember("trackGlowIntensity"))
            trackGlowIntensity = data["trackGlowIntensity"].asDouble();
    }
    Json::Value ColorTrack::intoJson() const
    {
        Json::Value val(Json::objectValue);
        val["floor"] = floor;
        val["eventType"] = name();
        if (!active)
            val["active"] = active;
        val["trackColor"] = trackColor.toString(false, false, Color::ToStringAlphaMode::Auto);
        val["secondaryTrackColor"] = secondaryTrackColor.toString(false, false, Color::ToStringAlphaMode::Auto);
        autoRemoveDecimalPart(val, "trackColorAnimDuration", trackColorAnimDuration);
        val["trackColorType"] = trackColorType2cstr(trackColorType);
        val["trackColorPulse"] = trackColorPulse2cstr(trackColorPulse);
        val["trackPulseLength"] = trackPulseLength;
        val["trackStyle"] = trackStyle2cstr(trackStyle);
        val["trackTexture"] = trackTexture;
        autoRemoveDecimalPart(val, "trackGlowIntensity", trackGlowIntensity);
        return val;
    }
    PositionTrack::PositionTrack(const Json::Value& data) : StaticEvent(data)
    {
        if (data.isMember("positionOffset"))
        {
            const auto posOff = data["positionOffset"];
            if (!posOff[0].isNull())
                positionOffset.x = posOff[0].asDouble();
            if (!posOff[1].isNull())
                positionOffset.y = posOff[1].asDouble();
        }
        relativeTo = data.isMember("relativeTo") ? RelativeIndex(data["relativeTo"]) : RelativeIndex(0, ThisTile);
        rotation = data.isMember("rotation") ? data["rotation"].asDouble() : 0;
        scale = data.isMember("scale") ? data["scale"].asDouble() : 100;
        opacity = data.isMember("opacity") ? data["opacity"].asDouble() : 100;
        justThisTile = data.isMember("justThisTile") ? toBool(data["justThisTile"]) : false;
        editorOnly = data.isMember("editorOnly") ? toBool(data["editorOnly"]) : false;
        if (data.isMember("stickToFloors"))
            stickToFloors = toBool(data["stickToFloors"]);
    }
    Json::Value PositionTrack::intoJson() const
    {
        Json::Value val(Json::objectValue);
        val["floor"] = floor;
        val["eventType"] = name();
        if (!active)
            val["active"] = active;
        {
            Json::Value posOffVal(Json::arrayValue);
            if (static_cast<int>(positionOffset.x) == positionOffset.x)
                posOffVal.append(static_cast<int>(positionOffset.x));
            else
                posOffVal.append(positionOffset.x);
            if (static_cast<int>(positionOffset.y) == positionOffset.y)
                posOffVal.append(static_cast<int>(positionOffset.y));
            else
                posOffVal.append(positionOffset.y);
            val["positionOffset"] = posOffVal;
        }
        val["relativeTo"] = relativeTo.intoJson();
        autoRemoveDecimalPart(val, "rotation", rotation);
        autoRemoveDecimalPart(val, "scale", scale);
        autoRemoveDecimalPart(val, "opacity", opacity);
        val["justThisTile"] = justThisTile;
        val["editorOnly"] = editorOnly;
        if (stickToFloors)
            val["stickToFloors"] = *stickToFloors;
        return val;
    }
    MoveTrack::MoveTrack(const Json::Value& data) : DynamicEvent(data)
    {
        startTile = RelativeIndex(data["startTile"]);
        endTile = RelativeIndex(data["endTile"]);
        duration = data["duration"].asDouble();
        if (data.isMember("positionOffset"))
        {
            if (!data["positionOffset"][0].isNull())
                positionOffset.first = data["positionOffset"][0].asDouble();
            if (!data["positionOffset"][1].isNull())
                positionOffset.second = data["positionOffset"][1].asDouble();
        }
        if (data.isMember("rotationOffset"))
            rotationOffset = data["rotationOffset"].asDouble();
        if (data.isMember("scale"))
        {
            if (data["scale"].isArray())
            {
                if (!data["scale"][0].isNull())
                    scale.first = data["scale"][0].asDouble();
                if (!data["scale"][1].isNull())
                    scale.second = data["scale"][1].asDouble();
            }
            else
            {
                scale.first = scale.second = data["scale"].asDouble();
            }
        }
        if (data.isMember("opacity"))
            opacity = data["opacity"].asDouble();
        ease = cstr2easing(data["ease"].asCString());
    }
    Json::Value MoveTrack::intoJson() const
    {
        Json::Value val(Json::objectValue);
        val["floor"] = floor;
        val["eventType"] = name();
        if (!active)
            val["active"] = active;
        val["startTile"] = startTile.intoJson();
        val["endTile"] = endTile.intoJson();
        autoRemoveDecimalPart(val, "duration", duration);
        // ReSharper disable once CppLocalVariableMayBeConst
        if (auto op = optionalPoint2json(positionOffset); op)
            val["positionOffset"] = *op;
        if (rotationOffset)
            autoRemoveDecimalPart(val, "rotationOffset", *rotationOffset);
        if (auto op = optionalPoint2json(scale); op)
            val["scale"] = *op;
        if (opacity)
            autoRemoveDecimalPart(val, "opacity", *opacity);
        autoRemoveDecimalPart(val, "angleOffset", angleOffset);
        val["ease"] = easing2cstr(ease);
        addTag(val, eventTag);
        return val;
    }
    AnimateTrack::AnimateTrack(const Json::Value& data) : StaticEvent(data)
    {
        if (data.isMember("trackAnimation"))
            trackAnimation = cstr2trackAnimation(data["trackAnimation"].asCString());
        beatsAhead = data["beatsAhead"].asDouble();
        if (data.isMember("trackDisappearAnimation"))
            trackDisappearAnimation = cstr2trackDisappearAnimation(data["trackDisappearAnimation"].asCString());
        beatsBehind = data["beatsBehind"].asDouble();
    }
    Json::Value AnimateTrack::intoJson() const
    {
        Json::Value val(Json::objectValue);
        val["floor"] = floor;
        val["eventType"] = name();
        if (!active)
            val["active"] = active;
        if (trackAnimation)
            val["trackAnimation"] = trackAnimation2cstr(*trackAnimation);
        autoRemoveDecimalPart(val, "beatsAhead", beatsAhead);
        if (trackDisappearAnimation)
            val["trackDisappearAnimation"] = trackDisappearAnimation2cstr(*trackDisappearAnimation);
        autoRemoveDecimalPart(val, "beatsBehind", beatsBehind);
        return val;
    }
    RecolorTrack::RecolorTrack(const Json::Value& data) : DynamicEvent(data)
    {
        startTile = RelativeIndex(data["startTile"]);
        endTile = RelativeIndex(data["endTile"]);
        if (data.isMember("duration"))
            duration = data["duration"].asDouble();
        trackColorType = cstr2trackColorType(data["trackColorType"].asCString());
        trackColor = Color(data["trackColor"].asCString());
        secondaryTrackColor = Color(data["secondaryTrackColor"].asCString());
        trackColorAnimDuration = data["trackColorAnimDuration"].asDouble();
        trackColorPulse = cstr2trackColorPulse(data["trackColorPulse"].asCString());
        trackPulseLength = data["trackPulseLength"].asUInt();
        trackStyle = cstr2trackStyle(data["trackStyle"].asCString());
        if (data.isMember("gapLength"))
            gapLength = data["gapLength"].asDouble();
        if (data.isMember("ease"))
            ease = cstr2easing(data["ease"].asCString());
        if (data.isMember("trackGlowIntensity"))
            trackGlowIntensity = data["trackGlowIntensity"].asDouble();
    }
    Json::Value RecolorTrack::intoJson() const
    {
        Json::Value val(Json::objectValue);
        val["floor"] = floor;
        val["eventType"] = name();
        if (!active)
            val["active"] = active;
        val["startTile"] = startTile.intoJson();
        val["endTile"] = endTile.intoJson();
        if (duration)
            autoRemoveDecimalPart(val, "duration", *duration);
        val["trackColorType"] = trackColorType2cstr(trackColorType);
        val["trackColor"] = trackColor.toString(false, false, Color::ToStringAlphaMode::Auto);
        val["secondaryTrackColor"] = secondaryTrackColor.toString(false, false, Color::ToStringAlphaMode::Auto);
        autoRemoveDecimalPart(val, "trackColorAnimDuration", trackColorAnimDuration);
        val["trackColorPulse"] = trackColorPulse2cstr(trackColorPulse);
        val["trackPulseLength"] = trackPulseLength;
        val["trackStyle"] = trackStyle2cstr(trackStyle);
        autoRemoveDecimalPart(val, "trackColorAnimDuration", trackColorAnimDuration);
        autoRemoveDecimalPart(val, "gapLength", gapLength);
        val["ease"] = easing2cstr(ease);
        autoRemoveDecimalPart(val, "trackGlowIntensity", trackGlowIntensity);
        addTag(val, eventTag);
        autoRemoveDecimalPart(val, "angleOffset", angleOffset);
        return val;
    }
} // namespace AdoCpp::Event::Track
