#pragma once
#include <AdoCpp.h>
#include "SFML/Window.hpp"
struct Config
{
    AdoCpp::Difficulty difficulty = AdoCpp::Difficulty::Strict;
    float inputOffset = -200;
    uint32_t fpsLimit = 0;
    bool blockKeyboardChatter = true;
    bool hidePerfects = true;
    bool syncWithMusic = false;
    bool disableAnimationTrack = false;
    sf::Time rainSpeed = sf::seconds(0.4f);
    float rainLength = 240.f;
    float keySize = 60.f;
    float gapSize = 3.f;
    float rainKeyGapSize = 3.f;
    bool keyShowHitError = false;
    bool rainShowHitError = false;
    using enum sf::Keyboard::Scan;
    std::vector<sf::Keyboard::Scan> keyLimiter = {
        LControl, CapsLock, Tab, Q, W, E, C, Space,
        A, Period, P, LBracket, RBracket, Backslash, Enter, Down
    };
    // std::vector<sf::Keyboard::Scan> keyLimiter = {
    // 	LControl, CapsLock, Tab, Num1, Num2, E, C, Space,
    // 	A, Period, P, Equal, Backspace, Backslash, Enter, Down
    // };
    // std::vector<sf::Keyboard::Scan> keyLimiter = {Tab, Q, W, E, C, Space, A, Period, P, LBracket, RBracket, Backslash};

    void load();
    void save();
};
