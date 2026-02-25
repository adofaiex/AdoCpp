#include "Utils.h"

#include <cstring>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "Math/Vector2.h"

namespace AdoCpp
{
    Json::Value RelativeIndex::intoJson() const
    {
        Json::Value val(Json::arrayValue);
        val.append(index);
        val.append(relativeTo);
        return val;
    }
    bool toBool(const Json::Value& data)
    {
        if (data.isBool())
            return data.asBool();
        if (data.isString())
        {
            if (!strcmp(data.asCString(), "Enabled"))
                return true;
            if (!strcmp(data.asCString(), "Disabled"))
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
    std::string tags2string(const std::vector<std::string>& tags)
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
        return str;
    }
    void addTag(Json::Value& jsonValue, const std::vector<std::string>& tags, bool repeatEvents)
    {
        jsonValue[repeatEvents ? "tag" : "eventTag"] = tags2string(tags);
    }
    void autoRemoveDecimalPart(Json::Value& jsonValue, const char* name, const double value)
    {
        if (static_cast<int64_t>(value) == value)
            jsonValue[name] = static_cast<int64_t>(value);
        else
            jsonValue[name] = value;
    }
    template <class T>
    Json::Value vector2ToJson(Vector2<T> vec2)
    {
        // TODO
    }
} // namespace AdoCpp
