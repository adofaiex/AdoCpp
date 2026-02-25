#pragma once
#include "Base.h"

namespace AdoCpp::Event::Modifiers
{
    class RepeatEvents final : public Event
    {
    public:
        enum class RepeatType
        {
            Beat,
            Floor
        };
        RepeatEvents() = default;
        explicit RepeatEvents(const Json::Value& data);
        [[nodiscard]] constexpr bool stackable() const noexcept override { return true; }
        [[nodiscard]] constexpr const char* name() const noexcept override { return "RepeatEvents"; }
        [[nodiscard]] constexpr RepeatEvents* clone() const override { return new RepeatEvents(*this); }
        [[nodiscard]] Json::Value
        intoJson() const override;
        RepeatType repeatType = RepeatType::Beat;
        size_t repetitions = 1;
        size_t floorCount = 1;
        double interval = 1;
        bool executeOnCurrentFloor = false;
        std::vector<std::string> tag;
        double duration = 1;
    };
} // namespace AdoCpp::Event::Modifiers
