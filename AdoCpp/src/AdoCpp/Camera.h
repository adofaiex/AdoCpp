#pragma once
#include "Level.h"

namespace AdoCpp
{
    struct MoveCameraData
    {
        size_t floor;
        double angleOffset;
        double beat;
        double seconds;
        double duration;
        std::optional<RelativeToCamera> relativeTo;
        bool duplicatedRelPlayer;
        double relEndSec;
        std::optional<Vector2lf> playerLastPos;
        OptionalPoint position;
        double xEndSec;
        double yEndSec;
        std::optional<double> rotation;
        double rotEndSec;
        std::optional<double> zoom;
        double zoomEndSec;
        Easing ease;
    };

    class Camera
    {
    public:
        Camera() = default;
        ~Camera() = default;
        void init(Level& level);
        void update(const Level& level, double seconds, size_t floor);
        Vector2lf position;
        double rotation{};
        double zoom = 100;
    private:
        std::vector<MoveCameraData> m_moveCameraDatas;
        Vector2lf player;
        double lastSeconds = std::numeric_limits<double>::lowest();
        size_t lastFloor{};
        Vector2lf lastChangedPos;
        size_t lastEventIndex{};
    };
}
