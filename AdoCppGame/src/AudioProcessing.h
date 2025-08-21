#pragma once
#include <SFML/Audio.hpp>

inline std::filesystem::path addHitsound(std::filesystem::path path, const std::vector<AdoCpp::Tile>& tiles,
                                         float* progress = nullptr)
{
    if (progress)
        *progress = -1;
    sf::SoundBuffer origSb{path}, newSb{}, hitSb{"assets/sound/hit.wav"};
    assert(hitSb.getChannelCount() == 2);
    auto samples = new int16_t[origSb.getSampleCount()];
    static constexpr short MAX = 32767, MIN = -32768;
    // ReSharper disable CppFunctionalStyleCast
    for (size_t k = 1 /* tile[0].beat = -INF */; k < tiles.size(); k++)
    {
        if (tiles[k].angle.deg() == 999)
            continue;
        const bool midspin = k != tiles.size() - 1 && tiles[k + 1].angle.deg() == 999;
        const float hitVolume =
            4.f * float(midspin ? tiles[k].midspinHitsoundVolume : tiles[k].hitsoundVolume) /
            100;
        float f = 1, val;
        for (size_t i = 0; i < hitSb.getSampleCount() / 4; i++)
        {
            for (size_t j = 0; j < origSb.getChannelCount(); j++)
            {
                const size_t idx =
                    (size_t(tiles[k].seconds * origSb.getSampleRate()) + i) * origSb.getChannelCount() + j;
                const size_t hitIdx = i * 4 + j % 4;
                if (idx >= origSb.getSampleCount())
                    break;
                val = (samples[idx] + hitSb.getSamples()[hitIdx] * hitVolume) * f;
                if (val > MAX)
                    f = MAX / val, val = MAX;
                if (val < MIN)
                    f = MIN / val, val = MIN;
                if (f < 1)
                    f += (1 - f) / 32;
                samples[idx] = short(val);
            }
        }
        if (progress)
            *progress = float(k) / float(tiles.size());
    }
    for (size_t i = 0; i < origSb.getSampleCount(); i++)
    {
        samples[i] = samples[i] / 2 + origSb.getSamples()[i] / 2;
    }
    // ReSharper restore CppFunctionalStyleCast
    if (progress)
        *progress = 2;
    if (!newSb.loadFromSamples(samples, origSb.getSampleCount(), origSb.getChannelCount(), origSb.getSampleRate(),
                               origSb.getChannelMap()))
        throw std::runtime_error("Failed to load sound.");
    std::string ext = path.extension().string();
    if (ext == ".mp3")
        ext = ".wav";
    path.replace_extension().concat("-hitsound").concat(ext);
    if (!newSb.saveToFile(path))
        throw std::runtime_error("Failed to save sound.");
    delete[] samples;
    return path;
}
