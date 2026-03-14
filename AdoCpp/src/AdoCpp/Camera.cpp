// FIXME

#include "Camera.h"
#include <ranges>
#include <cmath>

namespace AdoCpp
{
    void Camera::init(const Level& level)
    {
        const Settings& settings = level.settings;
        relativeTo = settings.relativeTo;
        anchorFloor = 0;
        transition = CameraTransition();
        transition.fromX = transition.toX = position.x = settings.position.x;
        transition.fromY = transition.toY = position.y = settings.position.y;
        transition.fromRotation = transition.toRotation = rotation = settings.rotation;
        transition.fromZoom = transition.toRotation = zoom = settings.zoom;
        transition.fromPlayer = transition.toPlayer = player = double(settings.relativeTo == RelativeToCamera::Player);
        timeline.clear();
        timelineIndex = std::nullopt;
        for (const auto& dynamicEvent : level.m_processedDynamicEvents)
        {
            const auto moveCamera = std::dynamic_pointer_cast<Event::Visual::MoveCamera>(dynamicEvent);
            if (moveCamera)
                timeline.push_back(*moveCamera);
        }
    }
    void Camera::handleTransition(const double seconds, double& var, const double fromVar, const double toVar, State& state)
    {
        if (!state.active) return;
        double transitionSec = seconds - state.startSec;
        double t = state.durationSec == 0.0 ? 1.0 : transitionSec / state.durationSec;
        t = std::max(0.0, std::min(1.0, t));
        double progress = ease(state.ease, t);
        var = std::lerp(fromVar, toVar, progress);
        if (t >= 1.0) state.active = false;
    }

    void Camera::processEvent(const Level& level, const Event::Visual::MoveCamera& moveCamera, double seconds)
    {
        if (!moveCamera.active) return;
        using enum RelativeToCamera;

        auto handleLastTransition = [&] (double& fromVar, const double toVar, State& state) {
            handleTransition(seconds, fromVar, fromVar, toVar, state);
            state.active = false;
        };

        bool isOffset = !moveCamera.relativeTo.has_value() || moveCamera.relativeTo == LastPosition || moveCamera.relativeTo == LastPositionNoRotation;
        double currentBPM = level.getBpmForDynamicEvent(moveCamera.floor, moveCamera.angleOffset);
        double durationSeconds = moveCamera.duration * bpm2crotchet(currentBPM);
        const State state = {
            .active = true,
            .startSec = moveCamera.seconds,
            .durationSec = durationSeconds,
            .ease = moveCamera.ease
        };
        if (moveCamera.position.first) {
            handleLastTransition(transition.fromX, transition.toX, transition.xState);
            transition.toX = (isOffset ? transition.toX : 0) + *moveCamera.position.first;
            transition.xState = state;
        }
        if (moveCamera.position.second) {
            handleLastTransition(transition.fromY, transition.toY, transition.yState);
            transition.toY = (isOffset ? transition.toY : 0) + *moveCamera.position.second;
            transition.yState = state;
        }
        if (moveCamera.rotation) {
            handleLastTransition(transition.fromRotation, transition.toRotation, transition.rotationState);
            transition.toRotation = *moveCamera.rotation;
            transition.rotationState = state;
        }
        if (moveCamera.zoom) {
            handleLastTransition(transition.fromZoom, transition.toZoom, transition.zoomState);
            transition.toZoom = *moveCamera.zoom;
            transition.zoomState = state;
        }

        // Update RelativeTo & Anchor (only if specified)
        if (moveCamera.relativeTo) {
            if (moveCamera.relativeTo == LastPosition || moveCamera.relativeTo == LastPositionNoRotation) {
                // Keep current relativeTo and anchorFloor
            } else {
                handleLastTransition(transition.fromPlayer, transition.toPlayer, transition.relativeToState);
                if (moveCamera.relativeTo == Tile || moveCamera.relativeTo == Global) {
                    anchorFloor = moveCamera.relativeTo == Global ? 0 : moveCamera.floor;
                    transition.toPlayer = 0;
                } else { // Player
                    transition.toPlayer = 1;
                }
                transition.relativeToState = state;
            }
        }
    }
    void Camera::update(const Level& level, double seconds, const size_t floor)
    {
        const double delta = std::isinf(lastSeconds) ? 0 : seconds - lastSeconds;
        lastSeconds = seconds;

        // 1. Process new camera events (using high-performance Timeline)
        // We check against timeInLevel because timeline is built using tileStartTimes (logic time)
        if (timelineIndex) {
            if (timelineIndex >= timeline.size()
            || seconds < timeline[*timelineIndex].seconds) {
                // Backward seek detected
                init(level);
                timelineIndex = std::nullopt;
            }
        }

        if (!timelineIndex) {
            if (!timeline.empty() && timeline[0].seconds <= seconds) {
                timelineIndex = 0;
                auto& entry = timeline[*timelineIndex];
                processEvent(level, entry, seconds);
            }
        }
        if (timelineIndex) {
            while (*timelineIndex + 1 < timeline.size() && 
                    timeline[*timelineIndex + 1].seconds <= seconds) {
                (*timelineIndex)++;
                auto& entry = timeline[*timelineIndex];
                processEvent(level, entry, seconds);
            }
        }
        
        // // 1.5 Process Bloom events
        // if (lastBloomTimelineIndex >= 0) {
        //     const currentEntry = bloomTimeline[lastBloomTimelineIndex];
        //     if (currentEntry && timeInLevel < currentEntry.time) {
        //         // Backward seek detected
        //         bloomEnabled = false;
        //         lastBloomTimelineIndex = -1; // FIXME
        //     }
        // }
        
        // while (lastBloomTimelineIndex + 1 < bloomTimeline.length && 
        //         bloomTimeline[lastBloomTimelineIndex + 1].time <= timeInLevel) {
        //     lastBloomTimelineIndex++;
        //     const entry = bloomTimeline[lastBloomTimelineIndex];
        //     processBloomEvent(entry.event);
        // }
        
        // 2. Interpolate Camera State (if transition active)
        auto handleTransition = [&] (double& var, const double fromVar, const double toVar, State& state) {
            this->handleTransition(seconds, var, fromVar, toVar, state);
        };

        handleTransition(positionOffset.x, transition.fromX, transition.toX, transition.xState);
        handleTransition(positionOffset.y, transition.fromY, transition.toX, transition.yState);
        handleTransition(rotation, transition.fromRotation, transition.toRotation, transition.rotationState);
        handleTransition(zoom, transition.fromZoom, transition.toZoom, transition.zoomState);
        handleTransition(player, transition.fromPlayer, transition.toPlayer, transition.relativeToState);

        // 3. Calculate World Target Position
        Vector2lf playerPos, nonPlayerPos;
        playerPos = nonPlayerPos = level.tiles[anchorFloor].pos.o;
        if (relativeTo == RelativeToCamera::Player)
        {
            Vector2lf currentPivotPosition = level.tiles[floor].pos.o;
            playerPos = position - positionOffset;
            // position move towards target
            if (floor != lastFloor)
                lastFloor = floor, lastPlayerChangedPos = playerPos;
            const double gapDis = (currentPivotPosition - lastPlayerChangedPos).length(),
                speed = gapDis * level.getBpmBySeconds(seconds) / 60.0 / 2.0;
            if (currentPivotPosition != playerPos)
            {
                const Vector2lf v = currentPivotPosition - playerPos;
                const Vector2lf n = v.normalized() * delta * speed;
                if (v.lengthSquared() > n.lengthSquared())
                    playerPos += n;
                else
                    playerPos = currentPivotPosition;
            }
        }
        position = playerPos * player + nonPlayerPos * (1 - player) + positionOffset;
    }
}
