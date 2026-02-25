#include "Config.h"
#include <json5cpp.h>

void Config::load()
{
    std::ifstream ifs("config.json");
    if (!ifs.is_open())
    {
        return;
    }
    Json::Value doc;
    std::string err;
    bool success = Json5::parse(ifs, doc, &err);
    if (!success)
    {
        throw std::runtime_error(std::string("Could not parse config file: ") + err);
    }
    if (!doc.isObject())
        throw std::runtime_error("Could not parse config file: The json document is not an object");
    difficulty = static_cast<AdoCpp::Difficulty>(doc["difficulty"].asInt());
    inputOffset = doc["inputOffset"].asFloat();
    fpsLimit = doc["fpsLimit"].asUInt();
    blockKeyboardChatter = doc["blockKeyboardChatter"].asBool();
    hidePerfects = doc["hidePerfects"].asBool();
    syncWithMusic = doc["syncWithMusic"].asBool();
    disableAnimationTrack = doc["disableAnimationTrack"].asBool();
    rainSpeed = sf::seconds(doc["rainSpeed"].asFloat());
    rainLength = doc["rainLength"].asFloat();
    keySize = doc["keySize"].asFloat();
    gapSize = doc["gapSize"].asFloat();
    rainKeyGapSize = doc["rainKeyGapSize"].asFloat();
    keyShowHitError = doc["keyShowHitError"].asBool();
    rainShowHitError = doc["rainShowHitError"].asBool();
    keyLimiter.clear();
    for (const auto& elem : doc["keyLimiter"])
    {
        keyLimiter.push_back(static_cast<sf::Keyboard::Scan>(elem.asInt()));
    }
}
void Config::save()
{
    std::ofstream ofs("config.json", std::ios::binary);
    if (!ofs.is_open())
        throw std::runtime_error("Could not open config file");
    Json::Value doc;
    doc["difficulty"] = static_cast<int>(difficulty);
    doc["inputOffset"] = inputOffset;
    doc["fpsLimit"] = fpsLimit;
    doc["blockKeyboardChatter"] = blockKeyboardChatter;
    doc["hidePerfects"] = hidePerfects;
    doc["syncWithMusic"] = syncWithMusic;
    doc["disableAnimationTrack"] = disableAnimationTrack;
    doc["rainSpeed"] = rainSpeed.asSeconds();
    doc["rainLength"] = rainLength;
    doc["keySize"] = keySize;
    doc["gapSize"] = gapSize;
    doc["rainKeyGapSize"] = rainKeyGapSize;
    doc["keyShowHitError"] = keyShowHitError;
    doc["rainShowHitError"] = rainShowHitError;
    {
        Json::Value array(Json::arrayValue);
        for (const auto& key : keyLimiter)
            array.append(static_cast<int>(key));
        doc["keyLimiter"] = array;
    }
    Json5::serialize(ofs, doc);
}
