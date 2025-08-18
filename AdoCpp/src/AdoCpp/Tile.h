#pragma once
#include <memory>
#include <optional>
#include <stdexcept>
#include <vector>
#include <cstring>

#include "Color.h"
#include "DynamicValue.h"
#include "Easing.h"
#include "Math/Angle.h"
#include "Math/Vector2.h"
#include "Utils.h"

namespace AdoCpp
{
    /**
     * @brief Orbit enum to represent the orbit of the planets.
     */
    enum Orbit
    {
        CounterClockwise,
        Clockwise,
    };
    /**
     * Reverse the orbit.
     * @param orbit the original orbit.
     * @return the reverse orbit.
     */
    inline Orbit operator!(const Orbit orbit) { return orbit == Clockwise ? CounterClockwise : Clockwise; }

    /**
     * TrackColorType enum to represent the type of the tile's color.
     */
    enum class TrackColorType
    {
        Single,
        Stripes,
        Glow,
        Blink,
        Switch,
        Rainbow,
        Volume,
    };
    /**
     * The C-style string array to represent the TrackColorType enum.
     */
    constexpr const char* const cstrTrackColorType[] = {
        "Single", "Stripes", "Glow", "Blink", "Switch", "Rainbow", "Volume",
    };
    /**
     *
     * @param trackColorType TrackColorType
     * @return the
     */
    constexpr const char* trackColorType2cstr(const TrackColorType& trackColorType)
    {
        return cstrTrackColorType[static_cast<int>(trackColorType)];
    }
    constexpr TrackColorType cstr2trackColorType(const char* cstr)
    {
        for (int i = 0; i < std::size(cstrTrackColorType); ++i)
            if (strcmp(cstr, cstrTrackColorType[i]) == 0)
                return static_cast<TrackColorType>(i);
        throw std::invalid_argument(cstr);
    }

    enum class TrackStyle
    {
        Standard,
        Neon,
        NeonLight,
        Basic,
        Minimal,
        Gems,
    };
    constexpr const char* const cstrTrackStyle[] = {"Standard", "Neon", "NeonLight", "Basic", "Minimal", "Gems"};
    constexpr const char* trackStyle2cstr(const TrackStyle& trackStyle)
    {
        return cstrTrackStyle[static_cast<int>(trackStyle)];
    }
    constexpr TrackStyle cstr2trackStyle(const char* cstr)
    {
        for (int i = 0; i < std::size(cstrTrackStyle); ++i)
            if (strcmp(cstr, cstrTrackStyle[i]) == 0)
                return static_cast<TrackStyle>(i);
        throw std::invalid_argument(cstr);
    }

    enum class TrackAnimation
    {
        // TODO
        None,
        Fade,
        Scatter,
        Scatter_Far,
        Assemble,
        Extend,
        Grow_Spin
    };
    constexpr const char* const cstrTrackAnimation[] = {
        // TODO
        "None", "Fade", "Scatter", "Scatter_Far", "Assemble", "Extend", "Grow_Spin"};
    constexpr const char* trackAnimation2cstr(const TrackAnimation& trackAnimation)
    {
        return cstrTrackAnimation[static_cast<int>(trackAnimation)];
    }
    constexpr TrackAnimation cstr2trackAnimation(const char* cstr)
    {
        for (int i = 0; i < std::size(cstrTrackAnimation); ++i)
            if (strcmp(cstr, cstrTrackAnimation[i]) == 0)
                return static_cast<TrackAnimation>(i);
        throw std::invalid_argument(cstr);
    }
    enum class TrackDisappearAnimation
    {
        // TODO
        None,
        Fade,
        Scatter,
        Scatter_Far,
        Retract,
        Shrink_Spin,
    };
    constexpr const char* const cstrTrackDisappearAnimation[] = {
        // TODO
        "None", "Fade", "Scatter", "Scatter_Far", "Retract", "Shrink_Spin"};
    constexpr const char* trackDisappearAnimation2cstr(const TrackDisappearAnimation& trackDisappearAnimation)
    {
        return cstrTrackDisappearAnimation[static_cast<int>(trackDisappearAnimation)];
    }
    constexpr TrackDisappearAnimation cstr2trackDisappearAnimation(const char* cstr)
    {
        for (int i = 0; i < std::size(cstrTrackDisappearAnimation); ++i)
            if (strcmp(cstr, cstrTrackDisappearAnimation[i]) == 0)
                return static_cast<TrackDisappearAnimation>(i);
        throw std::invalid_argument(cstr);
    }

    enum class TrackColorPulse
    {
        Backward = -1,
        None = 0,
        Forward = 1,
    };
    constexpr const char* const cstrTrackColorPulse[] = {"Backward", "None", "Forward"};
    constexpr const char* trackColorPulse2cstr(const TrackColorPulse& trackColorPulse)
    {
        return cstrTrackColorPulse[static_cast<int>(trackColorPulse) + 1];
    }
    constexpr TrackColorPulse cstr2trackColorPulse(const char* cstr)
    {
        for (int i = 0; i < std::size(cstrTrackColorPulse); ++i)
            if (strcmp(cstr, cstrTrackColorPulse[i]) == 0)
                return static_cast<TrackColorPulse>(i - 1);
        throw std::invalid_argument(cstr);
    }

    enum class Hitsound
    {
        None,
        Kick,
        kick, // THE MOON PATCH
        Sizzle,
        Shaker,
        FireTile,
        Hat,
        VehiclePositive,
        VehicleNegative,
        Squareshot,
        PowerDown,
        ReverbClap,
        ReverbClack,
        Hammer,
        SnareAcoustic2,
        SnareHouse,
        Sidestick,
        HatHouse,
        ShakerLoud,
        Chuck,
        KickHouse,
        KickRupture
    };
    constexpr const char* const cstrHitsound[] = {
        "None",
        "Kick",
        "kick",
        "Sizzle",
        "Shaker",
        "FireTile",
        "Hat",
        "VehiclePositive",
        "VehicleNegative",
        "Squareshot",
        "PowerDown",
        "ReverbClap",
        "ReverbClack",
        "Hammer",
        "SnareAcoustic2",
        "SnareHouse",
        "Sidestick",
        "HatHouse",
        "ShakerLoud",
        "Chuck",
        "KickHouse",
        "KickRupture"
    };
    constexpr const char* hitsound2cstr(const Hitsound& hitsound)
    {
        return cstrHitsound[static_cast<int>(hitsound)];
    }
    constexpr Hitsound cstr2hitsound(const char* cstr)
    {
        for (int i = 0; i < std::size(cstrHitsound); ++i)
            if (strcmp(cstr, cstrHitsound[i]) == 0)
                return static_cast<Hitsound>(i);
        throw std::invalid_argument(cstr);
    }

    namespace Event
    {
        class Event;
    }

    /**
     * @brief Tile struct used for storing tile datas.
     */
    struct Tile
    {
        /**
         * Default constructor.
         */
        Tile() = default;
        /**
         * Default deconstructor.
         */
        ~Tile() = default;

        /**
         * @brief The tile's angle.
         */
        Angle angle{};
        /**
         * @brief The event ptrs of the tile.
         */
        std::vector<std::shared_ptr<Event::Event>> events;

        /**
         * @brief The orbit of the planets when one of them lands on the tile.
         */
        Orbit orbit = Clockwise;
        /**
         * @brief The tile's beat.
         */
        double beat = 0;
        /**
         * @brief The tile's seconds.
         */
        double seconds = 0;
        /**
         * @brief The current opacity of the tile.
         */
        double opacity = 100;
        /**
         * @brief Whether the planets will stick to this tile.
         */
        bool stickToFloors = false;

        /**
         * @brief The position of the tile in editor.
         */
        Vector2lf editorPos;
        /**
         * @brief The position of the tile.
         */
        DynamicValue<Vector2lf> pos{};
        /**
         * @brief The scale of the tile.
         */
        DynamicValue<Vector2lf> scale{{100, 100}};
        /**
         * @brief The rotation of the tile.
         */
        DynamicValue<double> rotation{};

        /**
         *
         */
        DynamicValue<TrackColorType> trackColorType;
        /**
         * @brief The tile's color.
         */
        DynamicValue<Color> trackColor{Color(0xdebb7b)};
        /**
         * @brief The tile's secondary color.
         */
        DynamicValue<Color> secondaryTrackColor{Color(0xffffff)};
        /**
         *
         */
        DynamicValue<double> trackColorAnimDuration;
        /**
         * @brief The tile's style.
         */
        DynamicValue<TrackStyle> trackStyle{TrackStyle::Standard};
        /**
         *
         */
        DynamicValue<TrackColorPulse> trackColorPulse{TrackColorPulse::None};
        /**
         *
         */
        DynamicValue<uint32_t> trackPulseLength{10};
        /**
         * The tile's color.
         */
        Color color;


        size_t trackAnimationFloor = 0;
        TrackAnimation trackAnimation = TrackAnimation::None;
        double beatsAhead = 0;
        TrackDisappearAnimation trackDisappearAnimation = TrackDisappearAnimation::None;
        double beatsBehind = 0;

        Hitsound hitsound = Hitsound::Kick;
        double hitsoundVolume = 100;
        Hitsound midspinHitsound = Hitsound::Kick;
        double midspinHitsoundVolume = 100;

        struct MoveTrackData
        {
            size_t floor;
            double angleOffset;
            double beat;
            double seconds;
            RelativeIndex startTile;
            RelativeIndex endTile;
            double duration;
            OptionalPoint positionOffset;
            double xEndSec;
            double yEndSec;
            std::optional<double> rotationOffset;
            double rotEndSec;
            OptionalPoint scale;
            double scXEndSec;
            double scYEndSec;
            std::optional<double> opacity;
            double opEndSec;
            Easing ease;
            // double bpm; // TODO
        };
        std::vector<MoveTrackData> moveTrackDatas;

        /**
         * @brief Construct a tile.
         * @param angle The angle of the tile.
         */
        explicit Tile(const double angle) : angle(degrees(angle)) {}
    };
} // namespace AdoCpp
