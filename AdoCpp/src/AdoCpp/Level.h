#pragma once
#include <filesystem>
#include <fstream>
#include <functional>
#include <list>
#include <vector>
#include <json5cpp.h>

#include "Event.h"
#include "Math/Vector2.h"
#include "Tile.h"
#include "Utils.h"

namespace AdoCpp
{
    class LevelCouldNotOpenFileException final : public std::exception
    {
    public:
        LevelCouldNotOpenFileException() = default;
        [[nodiscard]] const char* what() const noexcept override
        {
            return "LevelCouldNotOpenFileException: could not open file";
        }
    };

    class LevelJsonException final : public std::exception
    {
        std::string err;
    public:
        explicit LevelJsonException(const std::string& err) : err(err) {}
        [[nodiscard]] const char* what() const noexcept override
        {
            return err.c_str();
        }
    };

    /**
     * @brief Adofai's difficulty.
     */
    enum class Difficulty
    {
        Lenient,
        Normal,
        Strict
    };

    /**
     * @brief Adofai's hit margin.
     */
    enum class HitMargin
    {
        Perfect,
        LatePerfect,
        EarlyPerfect,
        VeryLate,
        VeryEarly,
        TooLate,
        TooEarly
    };

    /**
     * @brief Settings struct.
     */
    struct Settings
    {
        int version = 15;

        // Song Settings
        std::string artist;
        std::string song;
        std::string author;
        bool separateCountdownTime = false;
        std::string songFilename;
        double bpm = 100;
        double volume = 100;
        double offset = 0;
        double pitch = 100;
        Hitsound hitsound = Hitsound::Kick;
        double hitsoundVolume = 100;
        double countdownTicks = 4;

        // Track Settings
        TrackColorType trackColorType = TrackColorType::Single;
        Color trackColor = Color(0xdebb7bff);
        Color secondaryTrackColor = Color(0xdebb7bff);
        double trackColorAnimDuration = 0;
        TrackColorPulse trackColorPulse = TrackColorPulse::None;
        uint32_t trackPulseLength = 10;
        TrackStyle trackStyle = TrackStyle::Standard;
        TrackAnimation trackAnimation = TrackAnimation::None;
        double beatsAhead = 0;
        TrackDisappearAnimation trackDisappearAnimation = TrackDisappearAnimation::None;
        double beatsBehind = 0;

        // Background Settings
        Color backgroundColor = Color(0, 0, 0);
        bool stickToFloors = false;
        double unscaledSize = 100;
        
        // Camera Settings
        RelativeToCamera relativeTo = RelativeToCamera::Player;
        Vector2lf position;
        double rotation = 0;
        double zoom = 100;

        Settings() = default;
        explicit Settings(const Json::Value& jsonSettings);

        [[nodiscard]] static Settings fromJson(const Json::Value& jsonSettings);
        [[nodiscard]] Json::Value intoJson() const;

        /**
         * Apply the settings to the tile.
         * @param tile The tile.
         */
        void apply(Tile& tile) const;
    };

    /**
     * @brief Planet struct.
     */
    struct Planet
    {
        Angle angle;
        Vector2lf position;
        double radius{};
    };

    /**
     * @brief Planets struct.
     */
    struct Planets
    {
        Planet fire;
        Planet ice;
        Planet third;
    };

    /**
     * @brief Level class.
     */
    class Level
    {
    public:
        /**
         * Create a new empty Level object.
         * @brief Default constructor.
         */
        Level() = default;
        /**
         * Create a new Level object from json data.
         * @brief Constructor.
         */
        explicit Level(const Json::Value& value);
        /**
         * Create a new Level object from a file encoded in UTF-8 BOM.
         * @brief Constructor.
         */
        explicit Level(std::ifstream& ifs);
        /**
         * Create a new Level object from a file encoded in UTF-8 BOM.
         * @brief Constructor.
         */
        explicit Level(const std::filesystem::path& path);

        Level(const Level&) = delete;

        /**
         * @brief Default destructor.
         */
        ~Level() = default;

        /**
         * @brief Clear the level class.
         */
        void clear();

        /**
         * @brief Generate the default level.
         * (parsed & updated)
         */
        void defaultLevel();

        /**
         * @brief Import json data into the level.
         * @param value Json data.
         */
        void fromJson(const Json::Value& value);

        /**
         * @brief Import a file into the level (encoded in UTF-8 BOM).
         * @param ifs The input file stream.
         */
        void fromFile(std::ifstream& ifs);
        /**
         * @brief Import a file into the level (encoded in UTF-8 BOM).
         * @param path The path to the file.
         */
        void fromFile(const std::filesystem::path& path);

        [[nodiscard]] Json::Value intoJson() const;

        /**
         * @brief Parse the level.
         */
        void parse(size_t floorStart = 0, bool basic = false, bool force = false);

        /**
         * @brief Update the level.
         */
        void update();
        /**
         * @brief Update the level.
         * @param seconds The seconds.
         */
        void update(double seconds);

        /**
         * @brief Insert the tile.
         * @param floor The index.
         * @param tile The tile.
         */
        void insertTile(size_t floor, const Tile& tile);
        /**
         * @brief Insert the tile.
         * @param floor The index.
         * @param angle The angle.
         */
        void insertTile(size_t floor, double angle);
        /**
         * Change the tile's angle into a new one.
         * @param floor The floor.
         * @param angle The new angle.
         */
        void changeTileAngle(size_t floor, double angle);

        /**
         * @brief Erase the tiles.
         * @param first The position.
         * @param last The number.
         */
        void eraseTile(size_t first, size_t last = -1ull);

        /**
         * @brief Push back the tile.
         * @param tile The tile.
         */
        void pushBackTile(const Tile& tile);
        /**
         * @brief Push back the tile.
         * @param angle The angle.
         */
        void pushBackTile(double angle);

        /**
         * @brief Pop back the tile.
         */
        void popBackTile();

        /**
         * @brief Convert baseIndex + relativeIndex into absolute index.
         * @param baseIndex The base index.
         * @param relativeIndex The index relative to the base index.
         * @return The absolute index.
         */
        [[nodiscard]] size_t rel2absIndex(size_t baseIndex, RelativeIndex relativeIndex) const;

        /**
         * @brief Get the included angle between the two planets.
         * @param floor The index of the tile.
         * @param seconds The seconds.
         * @return The position of the two planets.
         */
        [[nodiscard]] double getPlanetsDir(size_t floor, double seconds) const;

        /**
         * @brief Get the position of the two planets.
         * @param floor The index of the tile.
         * @param seconds The seconds.
         * @return The position of the two planets.
         */
        [[nodiscard]] std::pair<Vector2lf, Vector2lf> getPlanetsPos(size_t floor, double seconds) const;

        [[nodiscard]] static bool isFirePlanetStatic(size_t floor);

        /**
         * @brief Get the index of the tile that one of the planets lands on.
         * @param beat The beat.
         * @return The index of the tile.
         */
        [[nodiscard]] size_t getFloorByBeat(double beat) const;

        /**
         * @brief Get the index of the tile that one of the planets lands on.
         * @param seconds The seconds.
         * @return The index of the tile.
         */
        [[nodiscard]] size_t getFloorBySeconds(double seconds) const;

        template <class Val, class Pred>
        auto speedDataUpperBound(Val val, Pred pred) const;

        /**
         * @brief Get the bpm.
         * @param func The bool function.
         * @return The bpm.
         * @deprecated O(n) algorithm. Use speedDataUpperBound() instead.
         */
        [[nodiscard]] double getBpm(const std::function<bool(const Event::GamePlay::SetSpeed&)>& func) const;

        /**
         * @brief Get the bpm.
         * @param beat The beat.
         * @return The bpm.
         */
        [[nodiscard]] double getBpmByBeat(double beat) const;
        /**
         * @brief Get the bpm.
         * @param seconds The seconds.
         * @return The bpm.
         */
        [[nodiscard]] double getBpmBySeconds(double seconds) const;

        /**
         * @brief Get the bpm.
         * @param beat The beat.
         * @return The bpm.
         */
        [[nodiscard]] double getBpmExcludingBeat(double beat) const;

        /**
         * @brief Get the bpm.
         * @param floor The floor.
         * @param angleOffset The angle offset.
         * @return The bpm.
         */
        [[nodiscard]] double getBpmForDynamicEvent(size_t floor, double angleOffset) const;

        /**
         * @brief Convert beat to seconds.
         * @param beat The beat.
         * @return The time in seconds.
         */
        [[nodiscard]] double beat2seconds(double beat) const;

        /**
         * @brief Convert seconds to beat.
         * @param seconds The time in seconds.
         * @return The beat.
         */
        [[nodiscard]] double seconds2beat(double seconds) const;

        [[nodiscard]] double getAngle(size_t floor) const;

        /**
         * @brief Get the timing.
         * @param floor The index of the tile.
         * @param seconds The time in seconds.
         * @return The timing.
         */
        [[nodiscard]] double getTiming(size_t floor, double seconds) const;

        struct TimingBoundary
        {
            double perfect;
            double lateEarlyPerfect;
            double veryLateEarly;
        };

        [[nodiscard]] TimingBoundary getTimingBoundary(size_t floor, Difficulty difficulty) const;

        /**
         * @brief Get the hit margin.
         * @param floor The index of the tile.
         * @param seconds The time in seconds.
         * @param difficulty The difficulty.
         * @return The hit margin.
         */
        [[nodiscard]] HitMargin getHitMargin(size_t floor, double seconds, Difficulty difficulty) const;

        /**
         * @brief Get whether the level has been parsed.
         * @return Whether the level has been parsed.
         */
        [[nodiscard]] bool isParsed() const noexcept;

        [[nodiscard]] bool disableAnimateTrack() const;
        void disableAnimateTrack(bool disable);

        /**
         * @brief The level's settings.
         */
        Settings settings;

        /**
         * @brief The level's tiles.
         */
        std::vector<Tile> tiles;

    protected: // I do not know if I should use private or protected
        /**
         * @brief Whether the level has been parsed.
         */
        bool parsed = false;
        bool onlyBasic = false;
        bool m_disableAnimateTrack = false;

    private:
        void parseTiles(size_t beginFloor = 0);
        void parseSetSpeed();
        void parseDynamicEvents(std::vector<Event::DynamicEvent*>& dynamicEvents,
                                std::vector<std::vector<Event::Modifiers::RepeatEvents*>>& vecRe);
        void parseAnimateTrack();
        void parseRepeatEvents(const std::vector<Event::DynamicEvent*>& dynamicEvents,
                               const std::vector<std::vector<Event::Modifiers::RepeatEvents*>>& vecRe);
        void parseMoveTrackData();

        void updateTileColorInfo(const Event::Track::RecolorTrack* recolorTrack);
        void updateTileColor(double seconds, size_t i);
        void updateTilePos(double seconds, size_t i);

        std::list<std::shared_ptr<Event::DynamicEvent>> m_processedDynamicEvents;
        std::vector<std::shared_ptr<Event::GamePlay::SetSpeed>> m_setSpeeds;
        // y = kx + b
        // (x, y, k)
        struct SpeedData
        {
            double beat;
            double seconds;
            double bpm;

            size_t floor;
            double angleOffset;
        };
        std::vector<SpeedData> m_speedData;

        friend class Camera;
    };
} // namespace AdoCpp
