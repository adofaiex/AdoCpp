#pragma once
#include "Level.h"

namespace AdoCpp
{
    struct State {
        bool active = false;
        double startSec = 0;
        double durationSec = 0; // measured in seconds
        Easing ease;
    };
    struct CameraTransition
    {
        // notice that x and y are position offset, not position
        double fromX, toX;
        State xState;
        double fromY, toY;
        State yState;
        double fromRotation, toRotation;
        State rotationState;
        double fromZoom, toZoom;
        State zoomState;
        double fromPlayer, toPlayer;
        State relativeToState;
    };

    class Camera
    {
    public:
        Camera() = default;
        ~Camera() = default;
        void init(const Level& level);
        void processEvent(const Level& level, const Event::Visual::MoveCamera& moveCamera, double seconds);
        void update(const Level& level, double seconds, size_t floor);
        Vector2lf position;
        double rotation{};
        double zoom = 100;
    private:
        void handleTransition(const double seconds, double& var, const double fromVar, const double toVar, State& state);
        Vector2lf positionOffset;
        double lastSeconds = std::numeric_limits<double>::lowest();
        std::vector<Event::Visual::MoveCamera> timeline;
        std::optional<size_t> timelineIndex;
        Vector2lf lastPlayerChangedPos;
        size_t lastFloor;
        RelativeToCamera relativeTo;
        size_t anchorFloor;
        CameraTransition transition;
        double player;
    };
}
