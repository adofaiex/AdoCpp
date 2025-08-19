#pragma once

#include <complex.h>
#include <optional>
#include <rapidjson/document.h>
#include <stdexcept>
#include <string>
#include <vector>

namespace AdoCpp
{
    [[noreturn]] inline void unreachable()
    {
        // Uses compiler specific extensions if possible.
        // Even if no extension is used, undefined behavior is still raised by
        // an empty function body and the noreturn attribute.
#if defined(_MSC_VER) && !defined(__clang__) // MSVC
        __assume(false);
#else // GCC, Clang
        __builtin_unreachable();
#endif
    }

    typedef std::pair<std::optional<double>, std::optional<double>> OptionalPoint;
    inline std::optional<rapidjson::Value> optionalPoint2json(const OptionalPoint& op,
                                                              rapidjson::Document::AllocatorType& alloc)
    {
        if (!op.first && !op.second)
            return std::nullopt;
        rapidjson::Value val;
        val.SetArray();
        rapidjson::Value null(rapidjson::kNullType);
        if (op.first && !op.second)
        {
            if (static_cast<int>(*op.first) == *op.first)
                val.PushBack(static_cast<int>(*op.first), alloc);
            else
                val.PushBack(*op.first, alloc);
            val.PushBack(null, alloc);
        }
        else if (!op.first && op.second)
        {
            val.PushBack(null, alloc);
            if (static_cast<int>(*op.second) == *op.second)
                val.PushBack(static_cast<int>(*op.second), alloc);
            else
                val.PushBack(*op.second, alloc);
        }
        else
        {
            if (static_cast<int>(*op.first) == *op.first)
                val.PushBack(static_cast<int>(*op.first), alloc);
            else
                val.PushBack(*op.first, alloc);
            if (static_cast<int>(*op.second) == *op.second)
                val.PushBack(static_cast<int>(*op.second), alloc);
            else
                val.PushBack(*op.second, alloc);
        }
        return val;
    }

    // clang-format off
    constexpr double angles[] = {
        0,  15,  30,  45,  60,  75,
       90, 105, 120, 135, 150, 165,
      180, 195, 210, 225, 240, 255,
      270, 285, 300, 315, 330, 345,
      555, 666, 777, 888, 999
    };
    constexpr char paths[] = {
        'R', 'p', 'J', 'E', 'T', 'o',
        'U', 'q', 'G', 'Q', 'H', 'W',
        'L', 'x', 'N', 'Z', 'F', 'V',
        'D', 'Y', 'B', 'C', 'M', 'A',
        '5', '6', '7', '8', '!'
    };
    // clang-format on
    constexpr double path2angle(const char path)
    {
        for (size_t i = 0; i < std::size(angles); ++i)
            if (path == paths[i])
                return angles[i];
        throw std::invalid_argument("Invalid path");
    }
    constexpr char angle2path(const char angle)
    {
        for (size_t i = 0; i < std::size(angles); ++i)
            if (angle == angles[i])
                return paths[i];
        throw std::invalid_argument("Invalid angle");
    }


    enum RelativeToTile
    {
        Start,
        ThisTile,
        End
    };
    constexpr const char* const cstrRelativeToTile[] = {"Start", "ThisTile", "End"};
    constexpr RelativeToTile cstr2relativeToTile(const char* const cstr)
    {
        for (int i = 0; i < std::size(cstrRelativeToTile); ++i)
            if (strcmp(cstr, cstrRelativeToTile[i]) == 0)
                return static_cast<RelativeToTile>(i);
        throw std::invalid_argument("Invalid Cstr");
    }
    constexpr const char* relativeToTile2cstr(const RelativeToTile& rel)
    {
        return cstrRelativeToTile[static_cast<int>(rel)];
    }

    struct RelativeIndex
    {
        RelativeIndex() = default;
        RelativeIndex(const int64_t& index, const RelativeToTile& relativeTo) : index(index), relativeTo(relativeTo) {}

        explicit RelativeIndex(const rapidjson::Value& data) :
            index(data[0].GetInt64()), relativeTo(cstr2relativeToTile(data[1].GetString()))
        {
        }
        int64_t index{};
        RelativeToTile relativeTo{};
        std::unique_ptr<rapidjson::Value> intoJson(rapidjson::Document::AllocatorType& alloc) const;
    };

    enum class RelativeToCamera
    {
        Player,
        Tile,
        Global,
        LastPosition
    };
    constexpr const char* const cstrRelativeToCamera[] = {"Player", "Tile", "Global", "LastPosition"};
    constexpr RelativeToCamera cstr2relativeToCamera(const char* const cstr)
    {
        for (int i = 0; i < std::size(cstrRelativeToCamera); ++i)
            if (strcmp(cstr, cstrRelativeToCamera[i]) == 0)
                return static_cast<RelativeToCamera>(i);
        throw std::invalid_argument("Invalid Cstr");
    }
    constexpr const char* relativeToCamera2cstr(const RelativeToCamera& rel)
    {
        return cstrRelativeToCamera[static_cast<int>(rel)];
    }

    /**
     * @brief Convert json data to bool.
     * @param data The json data.
     * @return The bool.
     * data == true || data == "Enabled" => true.
     * data == false || data == "Disabled" => false.
     * Otherwise, throw an exception.
     */
    bool toBool(const rapidjson::Value& data);

    /**
     * @brief Convert bpm to spb(seconds per beat).
     * @param bpm The bpm.
     * @return The spb.
     */
    inline double bpm2crotchet(const double bpm) { return 60.0 / bpm; }
    /**
     * @brief Convert bpm to mspb(milliseconds per beat).
     * @param bpm The bpm.
     * @return The mspb.
     */
    inline double bpm2mspb(const double bpm)
    {
        // return 60 / bpm * 1000;
        return 60000 / bpm;
    }
    /**
     * @brief Get the included angle.
     * @param angleDeg
     * @param nextAngleDeg
     * @return The Angle.
     */
    double includedAngle(double angleDeg, double nextAngleDeg);

    std::vector<std::string> cstr2tags(const char* str);
    void tags2cstr(const std::vector<std::string>& tags, char* dest, rsize_t sizeInBytes);

    void addTag(rapidjson::Value& jsonValue, const std::vector<std::string>& tags,
                rapidjson::Document::AllocatorType& alloc, bool repeatEvents = false);
    void autoRemoveDecimalPart(rapidjson::Value& jsonValue, const char* name, double value,
                               rapidjson::Document::AllocatorType& alloc);
} // namespace AdoCpp
