#include "Config.h"
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/error/en.h>
void Config::load()
{
    std::ifstream ifs("config.json");
    if (!ifs.is_open())
    {
        return;
    }
    rapidjson::Document document;
    rapidjson::IStreamWrapper isw(ifs);
    rapidjson::AutoUTFInputStream<unsigned, rapidjson::IStreamWrapper> eis(isw);
    document.ParseStream<rapidjson::kParseValidateEncodingFlag | rapidjson::kParseCommentsFlag |
                             rapidjson::kParseTrailingCommasFlag | rapidjson::kParseNanAndInfFlag |
                             rapidjson::kParseFullPrecisionFlag,
                         rapidjson::AutoUTF<unsigned>>(eis);
    if (document.HasParseError())
    {
        const char* err = rapidjson::GetParseError_En(document.GetParseError());
        throw std::runtime_error(std::string("Could not parse config file: ") + err);
    }
    if (!document.IsObject())
        throw std::runtime_error("Could not parse config file: The json document is not an object");
    difficulty = static_cast<AdoCpp::Difficulty>(document["difficulty"].GetInt());
    inputOffset = document["inputOffset"].GetFloat();
    fpsLimit = document["fpsLimit"].GetUint();
    blockKeyboardChatter = document["blockKeyboardChatter"].GetBool();
    hidePerfects = document["hidePerfects"].GetBool();
    syncWithMusic = document["syncWithMusic"].GetBool();
    disableAnimationTrack = document["disableAnimationTrack"].GetBool();
    rainSpeed = sf::seconds(document["rainSpeed"].GetFloat());
    rainLength = document["rainLength"].GetFloat();
    keySize = document["keySize"].GetFloat();
    gapSize = document["gapSize"].GetFloat();
    rainKeyGapSize = document["rainKeyGapSize"].GetFloat();
    keyShowHitError = document["keyShowHitError"].GetBool();
    rainShowHitError = document["rainShowHitError"].GetBool();
    keyLimiter.clear();
    const auto array = document["keyLimiter"].GetArray();
    for (const auto& elem : array)
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
    doc.SetObject();
    doc.AddMember("difficulty", static_cast<int>(difficulty), alloc);
    doc.AddMember("inputOffset", inputOffset, alloc);
    doc.AddMember("fpsLimit", fpsLimit, alloc);
    doc.AddMember("blockKeyboardChatter", blockKeyboardChatter, alloc);
    doc.AddMember("hidePerfects", hidePerfects, alloc);
    doc.AddMember("syncWithMusic", syncWithMusic, alloc);
    doc.AddMember("disableAnimationTrack", disableAnimationTrack, alloc);
    doc.AddMember("rainSpeed", rainSpeed.asSeconds(), alloc);
    doc.AddMember("rainLength", rainLength, alloc);
    doc.AddMember("keySize", keySize, alloc);
    doc.AddMember("gapSize", gapSize, alloc);
    doc.AddMember("rainKeyGapSize", rainKeyGapSize, alloc);
    doc.AddMember("keyShowHitError", keyShowHitError, alloc);
    doc.AddMember("rainShowHitError", rainShowHitError, alloc);
    {
        rapidjson::Value array;
        array.SetArray();
        for (const auto& key : keyLimiter)
        {
            array.PushBack(static_cast<int>(key), alloc);
        }
        doc.AddMember("keyLimiter", array, alloc);
    }
    rapidjson::OStreamWrapper osw(ofs);
    rapidjson::EncodedOutputStream<rapidjson::UTF8<>, rapidjson::OStreamWrapper> eos(osw, false);
    rapidjson::PrettyWriter writer(eos);
    doc.Accept(writer);
}
