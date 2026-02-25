#pragma once
#include "AdoCpp/Easing.h"
#include "AdoCpp/Utils.h"
#include "Base.h"

namespace AdoCpp::Event::Visual
{
    class MoveCamera final : public DynamicEvent
    {
    public:
        MoveCamera() = default;
        explicit MoveCamera(const Json::Value& data);
        [[nodiscard]] constexpr bool stackable() const noexcept override { return true; }
        [[nodiscard]] constexpr const char* name() const noexcept override { return "MoveCamera"; }
        [[nodiscard]] constexpr MoveCamera* clone() const override { return new MoveCamera(*this); }
        [[nodiscard]] Json::Value
        intoJson() const override;
        double duration = 1;
        std::optional<RelativeToCamera> relativeTo;
        OptionalPoint position;
        std::optional<double> rotation;
        std::optional<double> zoom;
        Easing ease = Easing::Linear;
    };
} // namespace AdoCpp::Event::Visual
