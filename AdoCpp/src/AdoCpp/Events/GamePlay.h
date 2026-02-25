#pragma once
#include "AdoCpp/Easing.h"
#include "AdoCpp/Tile.h"
#include "Base.h"

namespace AdoCpp::Event::GamePlay
{
    class SetSpeed final : public DynamicEvent
    {
    public:
        enum class SpeedType
        {
            Bpm,
            Multiplier
        };
        SetSpeed() = default;
        explicit SetSpeed(const Json::Value& data);
        [[nodiscard]] constexpr bool stackable() const noexcept override { return true; }
        [[nodiscard]] constexpr const char* name() const noexcept override { return "SetSpeed"; }
        [[nodiscard]] constexpr SetSpeed* clone() const override { return new SetSpeed(*this); }
        [[nodiscard]] Json::Value
        intoJson() const override;
        SpeedType speedType = SpeedType::Bpm;
        double beatsPerMinute = 100;
        double bpmMultiplier = 1;
    };
    class Twirl final : public StaticEvent
    {
    public:
        Twirl() = default;
        explicit Twirl(const Json::Value& data);
        [[nodiscard]] constexpr bool stackable() const noexcept override { return false; }
        [[nodiscard]] constexpr const char* name() const noexcept override { return "Twirl"; }
        [[nodiscard]] constexpr Twirl* clone() const override { return new Twirl(*this); }
        [[nodiscard]] Json::Value
        intoJson() const override;
    };
    class Pause final : public StaticEvent
    {
    public:
        Pause() = default;
        explicit Pause(const Json::Value& data);
        [[nodiscard]] constexpr bool stackable() const noexcept override { return false; };
        [[nodiscard]] constexpr const char* name() const noexcept override { return "Pause"; };
        [[nodiscard]] constexpr Pause* clone() const override { return new Pause(*this); }
        [[nodiscard]] Json::Value
        intoJson() const override;
        double duration = 0;
        double countdownTicks = 0;
        enum class AngleCorrectionDir
        {
            Backward = -1,
            None = 0,
            Forward = 1
        };
        AngleCorrectionDir angleCorrectionDir = AngleCorrectionDir::None;
    };
    class SetHitsound final : public StaticEvent
    {
    public:
        enum class GameSound
        {
            Hitsound,
            Midspin
        };
        SetHitsound() = default;
        explicit SetHitsound(const Json::Value& data);
        [[nodiscard]] constexpr bool stackable() const noexcept override { return false; }
        [[nodiscard]] constexpr const char* name() const noexcept override { return "SetHitsound"; }
        [[nodiscard]] constexpr SetHitsound* clone() const override { return new SetHitsound(*this); }
        [[nodiscard]] Json::Value
        intoJson() const override;
        GameSound gameSound = GameSound::Hitsound;
        Hitsound hitsound = Hitsound::Kick;
        double hitsoundVolume = 100;
    };
    class SetPlanetRotation final : public StaticEvent
    {
    public:
        enum class EasePartBehavior
        {
            Repeat,
            Mirror
        };
        SetPlanetRotation() = default;
        explicit SetPlanetRotation(const Json::Value& data);
        [[nodiscard]] constexpr bool stackable() const noexcept override { return false; };
        [[nodiscard]] constexpr const char* name() const noexcept override { return "SetPlanetRotation"; };
        [[nodiscard]] constexpr SetPlanetRotation* clone() const override { return new SetPlanetRotation(*this); }
        [[nodiscard]] Json::Value
        intoJson() const override;
        Easing ease = Easing::Linear;
        uint64_t easeParts = 1;
        EasePartBehavior easePartBehavior = EasePartBehavior::Repeat;
    };
    using SetPlanetOrbit = SetPlanetRotation;
} // namespace AdoCpp::Event::GamePlay
