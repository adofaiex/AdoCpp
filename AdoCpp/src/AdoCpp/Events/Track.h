#pragma once
#include "AdoCpp/Color.h"
#include "AdoCpp/Easing.h"
#include "AdoCpp/Math/Vector2.h"
#include "AdoCpp/Tile.h"
#include "AdoCpp/Utils.h"
#include "Base.h"

namespace AdoCpp::Event::Track
{
    class ColorTrack final : public StaticEvent
    {
    public:
        ColorTrack() = default;
        explicit ColorTrack(const Json::Value& data);
        [[nodiscard]] constexpr bool stackable() const noexcept override { return false; }
        [[nodiscard]] constexpr const char* name() const noexcept override { return "ColorTrack"; }
        [[nodiscard]] constexpr ColorTrack* clone() const override { return new ColorTrack(*this); }
        [[nodiscard]] Json::Value
        intoJson() const override;
        TrackColorType trackColorType{};
        Color trackColor;
        Color secondaryTrackColor;
        double trackColorAnimDuration{};
        TrackColorPulse trackColorPulse{TrackColorPulse::None};
        uint32_t trackPulseLength = 10;
        TrackStyle trackStyle{};
        std::string trackTexture;
        double trackGlowIntensity{};
    };
    class AnimateTrack final : public StaticEvent
    {
    public:
        AnimateTrack() = default;
        explicit AnimateTrack(const Json::Value& data);
        [[nodiscard]] constexpr bool stackable() const noexcept override { return false; }
        [[nodiscard]] constexpr const char* name() const noexcept override { return "AnimateTrack"; }
        [[nodiscard]] constexpr AnimateTrack* clone() const override { return new AnimateTrack(*this); }
        [[nodiscard]] Json::Value
        intoJson() const override;
        std::optional<TrackAnimation> trackAnimation;
        double beatsAhead{};
        std::optional<TrackDisappearAnimation> trackDisappearAnimation;
        double beatsBehind{};
    };
    class RecolorTrack final : public DynamicEvent
    {
    public:
        RecolorTrack() = default;
        explicit RecolorTrack(const Json::Value& data);
        [[nodiscard]] constexpr bool stackable() const noexcept override { return true; }
        [[nodiscard]] constexpr const char* name() const noexcept override { return "RecolorTrack"; }
        [[nodiscard]] constexpr RecolorTrack* clone() const override { return new RecolorTrack(*this); }
        [[nodiscard]] Json::Value
        intoJson() const override;
        RelativeIndex startTile;
        RelativeIndex endTile;
        double gapLength{};
        std::optional<double> duration;
        TrackColorType trackColorType = TrackColorType::Single;
        Color trackColor;
        Color secondaryTrackColor;
        double trackColorAnimDuration{};
        TrackColorPulse trackColorPulse = TrackColorPulse::None;
        uint32_t trackPulseLength = 10;
        TrackStyle trackStyle{};
        double trackGlowIntensity{};
        Easing ease{};
    };
    class PositionTrack final : public StaticEvent
    {
    public:
        PositionTrack() = default;
        explicit PositionTrack(const Json::Value& data);
        [[nodiscard]] constexpr bool stackable() const noexcept override { return false; }
        [[nodiscard]] constexpr const char* name() const noexcept override { return "PositionTrack"; }
        [[nodiscard]] constexpr PositionTrack* clone() const override { return new PositionTrack(*this); }
        [[nodiscard]] Json::Value
        intoJson() const override;
        Vector2lf positionOffset;
        RelativeIndex relativeTo;
        double rotation{};
        double scale{};
        double opacity{};
        bool justThisTile{false};
        bool editorOnly{false};
        std::optional<bool> stickToFloors;
    };
    class MoveTrack final : public DynamicEvent
    {
    public:
        MoveTrack() = default;
        explicit MoveTrack(const Json::Value& data);
        [[nodiscard]] constexpr bool stackable() const noexcept override { return true; }
        [[nodiscard]] constexpr const char* name() const noexcept override { return "MoveTrack"; }
        [[nodiscard]] constexpr MoveTrack* clone() const override { return new MoveTrack(*this); }
        [[nodiscard]] Json::Value
        intoJson() const override;
        RelativeIndex startTile;
        RelativeIndex endTile;
        double duration = 0;
        OptionalPoint positionOffset;
        std::optional<double> rotationOffset;
        OptionalPoint scale;
        std::optional<double> opacity;
        Easing ease = Easing::Linear;
    };
} // namespace AdoCpp::Event::Track
