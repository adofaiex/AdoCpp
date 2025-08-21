#include "Utils.h"

#include <cstring>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "Math/Vector2.h"

namespace AdoCpp
{
    std::unique_ptr<rapidjson::Value> RelativeIndex::intoJson(rapidjson::Document::AllocatorType& alloc) const
    {
        auto val = std::make_unique<rapidjson::Value>(rapidjson::kArrayType);
        val->PushBack(index, alloc);
        val->PushBack(rapidjson::StringRef(relativeToTile2cstr(relativeTo)), alloc);
        return val;
    }
    bool toBool(const rapidjson::Value& data)
    {
        if (data.IsBool())
            return data.GetBool();
        if (data.IsString())
        {
            if (!strcmp(data.GetString(), "Enabled"))
                return true;
            if (!strcmp(data.GetString(), "Disabled"))
                return false;
        }
        throw std::invalid_argument(R"(data is not a boolean or a string "Enabled" or a string "Disabled")");
    }
    double includedAngle(const double angleDeg, const double nextAngleDeg)
    {
        double incAng = angleDeg + 180 - nextAngleDeg;
        while (incAng <= 0)
            incAng += 360;
        while (incAng > 360)
            incAng -= 360;
        return incAng;
    }
    std::vector<std::string> cstr2tags(const char* const str)
    {
        std::istringstream iss(str);
        std::string token;
        std::vector<std::string> vec;
        while (getline(iss, token, ' '))
            vec.push_back(token);
        return vec;
    }
    void tags2cstr(const std::vector<std::string>& tags, char* dest, const rsize_t sizeInBytes)
    {
        bool first = true;
        std::string str;
        for (const auto& tag : tags)
        {
            if (!first)
                str += ' ';
            str += tag;
            first = false;
        }
        strcpy_s(dest, sizeInBytes, str.c_str());
    }
    void addTag(rapidjson::Value& jsonValue, const std::vector<std::string>& tags,
                rapidjson::Document::AllocatorType& alloc, bool repeatEvents)
    {
        char tagBuf[1145]{};
        tags2cstr(tags, tagBuf, 1145);
        rapidjson::Value tagValue;
        tagValue.SetString(tagBuf, strlen(tagBuf), alloc);
        if (repeatEvents)
            jsonValue.AddMember("tag", tagValue, alloc);
        else
            jsonValue.AddMember("eventTag", tagValue, alloc);
    }
    void autoRemoveDecimalPart(rapidjson::Value& jsonValue, const char* name, const double value,
                               rapidjson::Document::AllocatorType& alloc)
    {
        if (static_cast<int>(value) == value)
            jsonValue.AddMember(rapidjson::StringRef(name), static_cast<int>(value), alloc);
        else
            jsonValue.AddMember(rapidjson::StringRef(name), value, alloc);
    }
    template <class T>
    std::unique_ptr<rapidjson::Value> vector2ToJson(Vector2<T> vec2, rapidjson::Document::AllocatorType& alloc)
    {
        auto val = std::make_unique<rapidjson::Value>();

    }
} // namespace AdoCpp
