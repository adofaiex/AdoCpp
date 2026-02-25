#include "Camera.h"
#include <ranges>
#include <cmath>

namespace AdoCpp
{
    void Camera::init(Level& level)
    {
        m_moveCameraDatas.clear();
        double rotEndSec, zoomEndSec, xEndSec, yEndSec,
            relEndSec = xEndSec = yEndSec = rotEndSec = zoomEndSec = std::numeric_limits<double>::infinity();
        for (const auto& dynamicEvent : std::ranges::reverse_view(level.m_processedDynamicEvents))
        {
            const auto mc = std::dynamic_pointer_cast<Event::Visual::MoveCamera>(dynamicEvent);
            if (mc == nullptr)
                continue;
            m_moveCameraDatas.emplace(m_moveCameraDatas.begin(), mc->floor, mc->angleOffset, mc->beat, mc->seconds,
                                      mc->duration, mc->relativeTo, false, 114514, std::nullopt, mc->position, xEndSec,
                                      yEndSec, mc->rotation, rotEndSec, mc->zoom, zoomEndSec, mc->ease);
            if (mc->relativeTo)
            {
                if (*mc->relativeTo == RelativeToCamera::LastPosition)
                    xEndSec = yEndSec = mc->seconds;
            }
            if (mc->position.first)
                xEndSec = mc->seconds;
            if (mc->position.second)
                yEndSec = mc->seconds;
            if (mc->rotation)
                rotEndSec = mc->seconds;
            if (mc->zoom)
                zoomEndSec = mc->seconds;
        }
        m_moveCameraDatas.emplace(m_moveCameraDatas.begin(), 0, 0, -INFINITY, -INFINITY, 0.0, level.settings.relativeTo,
                                  false, relEndSec, std::nullopt, OptionalPoint(), xEndSec, yEndSec,
                                  level.settings.rotation, rotEndSec, level.settings.zoom, zoomEndSec, Easing::Linear);
        RelativeToCamera lastRel = level.settings.relativeTo;
        for (auto it = m_moveCameraDatas.begin() + 1; it != m_moveCameraDatas.end(); ++it)
        {
            if (!it->relativeTo.has_value()) continue;
            it->duplicatedRelPlayer = lastRel == RelativeToCamera::Player && (*it->relativeTo == RelativeToCamera::Player || *it->relativeTo == RelativeToCamera::LastPosition);
            if (!it->duplicatedRelPlayer)
                lastRel = *it->relativeTo;
        }
        for (auto& m_moveCameraData : std::ranges::reverse_view(m_moveCameraDatas))
        {
            m_moveCameraData.relEndSec = relEndSec;
            if (!m_moveCameraData.duplicatedRelPlayer)
                relEndSec = m_moveCameraData.seconds;
        }
    }
    void Camera::update(const Level& level, double seconds, const size_t floor)
    {
        Vector2lf pos, posOff;
        for (auto& data : m_moveCameraDatas)
        {
            const double bpm = level.getBpmForDynamicEvent(data.floor, data.angleOffset), spb = bpm2crotchet(bpm);
            if (seconds < data.seconds)
                break;

            auto calcX = [&seconds, &data, &spb](const double endSec)
            { return data.duration != 0.0 ? (std::min(seconds, endSec) - data.seconds) / spb / data.duration : 1.0; };

            if (data.relativeTo)
            {
                if (!data.duplicatedRelPlayer)
                {
                    const double x = calcX(data.relEndSec), y = ease(data.ease, x);
                    using enum RelativeToCamera;
                    switch (*data.relativeTo)
                    {
                    case Player:
                    {
                        if (seconds > data.relEndSec)
                        {
                            if (data.playerLastPos)
                                pos = *data.playerLastPos;
                            break;
                        }

                        const double delta = std::isinf(lastSeconds) ? 0 : seconds - lastSeconds;
                        if (!std::isnormal(delta) && delta != 0)
                            throw std::logic_error("Delta (seconds - lastSeconds) is not normal and is not zero");
                        const Vector2lf& targetPos = level.tiles[floor].pos.o;
                        if (floor != lastFloor)
                            lastFloor = floor, lastChangedPos = player;
                        const double gapDis = (targetPos - lastChangedPos).length(),
                                     speed = gapDis * level.getBpmBySeconds(seconds) / 60.0 / 2.0;
                        if (targetPos != player)
                        {
                            const Vector2lf v = targetPos - player;
                            if (const Vector2lf n = v.normalized() * delta * speed;
                                (player - targetPos).lengthSquared() > n.lengthSquared())
                                player += n;
                            else
                                player = targetPos;
                        }

                        pos += (player - pos) * y;
                        data.playerLastPos = pos;
                        break;
                    }
                    case Tile:
                        pos += (level.tiles[data.floor].pos.o - pos) * y;
                        if (seconds <= data.relEndSec)
                            player = pos;
                        break;
                    case Global:
                        // pos += (Vector2lf(0, 0) - pos) * y;
                        // pos += -pos * y;
                        pos *= (1 - y);
                        if (seconds <= data.relEndSec)
                            player = pos;
                        break;
                    case LastPosition:
                        pos += posOff;
                        posOff = Vector2lf(0, 0);
                        if (seconds <= data.relEndSec)
                            player = pos;
                        break;
                    }
                }
            }

            if (data.position.first)
            {
                const double x = calcX(data.xEndSec), y = ease(data.ease, x);
                posOff.x += (*data.position.first - posOff.x) * y;
            }
            if (data.position.second)
            {
                const double x = calcX(data.yEndSec), y = ease(data.ease, x);
                posOff.y += (*data.position.second - posOff.y) * y;
            }
            if (data.rotation)
            {
                const double x = calcX(data.rotEndSec), y = ease(data.ease, x);
                rotation += (*data.rotation - rotation) * y;
            }
            if (data.zoom)
            {
                const double x = calcX(data.zoomEndSec), y = ease(data.ease, x);
                zoom += (*data.zoom - zoom) * y;
            }
        }
        position = pos + posOff;
        lastSeconds = seconds;
    }
}
