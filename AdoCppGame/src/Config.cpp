#include "Config.h"
#include <rapidjson/error/en.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>
void Config::load()
{
    std::ifstream ifs("config.json");
    if (!ifs.is_open())
    {
        return;
    }
    rapidjson::Document doc;
    rapidjson::IStreamWrapper isw(ifs);
    rapidjson::AutoUTFInputStream<unsigned, rapidjson::IStreamWrapper> eis(isw);
    doc.ParseStream<rapidjson::kParseValidateEncodingFlag | rapidjson::kParseCommentsFlag |
                        rapidjson::kParseTrailingCommasFlag | rapidjson::kParseNanAndInfFlag |
                        rapidjson::kParseFullPrecisionFlag,
                    rapidjson::AutoUTF<unsigned>>(eis);
    if (doc.HasParseError())
    {
        const char* err = rapidjson::GetParseError_En(doc.GetParseError());
        throw std::runtime_error(std::string("Could not parse config file: ") + err);
    }
    if (!doc.IsObject())
        throw std::runtime_error("Could not parse config file: The json document is not an object");
    difficulty = static_cast<AdoCpp::Difficulty>(doc["difficulty"].GetInt());
    inputOffset = doc["inputOffset"].GetFloat();
    fpsLimit = doc["fpsLimit"].GetUint();
    blockKeyboardChatter = doc["blockKeyboardChatter"].GetBool();
    hidePerfects = doc["hidePerfects"].GetBool();
    syncWithMusic = doc["syncWithMusic"].GetBool();
    disableAnimationTrack = doc["disableAnimationTrack"].GetBool();
    rainSpeed = sf::seconds(doc["rainSpeed"].GetFloat());
    rainLength = doc["rainLength"].GetFloat();
    keySize = doc["keySize"].GetFloat();
    gapSize = doc["gapSize"].GetFloat();
    rainKeyGapSize = doc["rainKeyGapSize"].GetFloat();
    keyShowHitError = doc["keyShowHitError"].GetBool();
    rainShowHitError = doc["rainShowHitError"].GetBool();
    keyLimiter.clear();
    for (const auto& elem : doc["keyLimiter"].GetArray())
    {
        keyLimiter.push_back(static_cast<sf::Keyboard::Scan>(elem.GetInt()));
    }
}
void Config::save()
{
    std::ofstream ofs("config.json", std::ios::binary);
    if (!ofs.is_open())
        throw std::runtime_error("Could not open config file");
    rapidjson::Document doc;
    auto& alloc = doc.GetAllocator();
    doc.SetObject()
        .AddMember("difficulty", static_cast<int>(difficulty), alloc)
        .AddMember("inputOffset", inputOffset, alloc)
        .AddMember("fpsLimit", fpsLimit, alloc)
        .AddMember("blockKeyboardChatter", blockKeyboardChatter, alloc)
        .AddMember("hidePerfects", hidePerfects, alloc)
        .AddMember("syncWithMusic", syncWithMusic, alloc)
        .AddMember("disableAnimationTrack", disableAnimationTrack, alloc)
        .AddMember("rainSpeed", rainSpeed.asSeconds(), alloc)
        .AddMember("rainLength", rainLength, alloc)
        .AddMember("keySize", keySize, alloc)
        .AddMember("gapSize", gapSize, alloc)
        .AddMember("rainKeyGapSize", rainKeyGapSize, alloc)
        .AddMember("keyShowHitError", keyShowHitError, alloc)
        .AddMember("rainShowHitError", rainShowHitError, alloc);
    {
        rapidjson::Value array;
        array.SetArray();
        for (const auto& key : keyLimiter)
            array.PushBack(static_cast<int>(key), alloc);
        doc.AddMember("keyLimiter", array, alloc);
    }
    rapidjson::OStreamWrapper osw(ofs);
    rapidjson::EncodedOutputStream<rapidjson::UTF8<>, rapidjson::OStreamWrapper> eos(osw, false);
    rapidjson::PrettyWriter writer(eos);
    doc.Accept(writer);
}
