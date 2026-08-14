#pragma once
#include "Base.h"

namespace AdoCpp::Event::Dlc
{
    class Hold final : public StaticEvent
    {
    public:
        Hold() = default;
        explicit Hold(const Json::Value& data);
        [[nodiscard]] constexpr bool stackable() const noexcept override { return false; }
        [[nodiscard]] constexpr const char* name() const noexcept override { return "Hold"; }
        [[nodiscard]] constexpr Hold* clone() const override { return new Hold(*this); }
        [[nodiscard]] Json::Value intoJson() const override;
        double duration = 1;
        double distanceMultiplier = 1;
        bool landingAnimation = false;
    };
    class MultiPlanet : public StaticEvent
    {
    public:
        MultiPlanet() = default;
        explicit MultiPlanet(const Json::Value& data);
        [[nodiscard]] constexpr bool stackable() const noexcept override { return false; }
        [[nodiscard]] constexpr const char* name() const noexcept override { return "MultiPlanet"; }
        [[nodiscard]] constexpr MultiPlanet* clone() const override { return new MultiPlanet(*this); }
        [[nodiscard]] Json::Value intoJson() const override;
        bool three = true;
    };
} // namespace AdoCpp::Event::Dlc
