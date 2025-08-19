#include "KeyViewer.h"
#include <algorithm>
#include <ranges>

constexpr std::array<sf::Vector2u, 10> posArray10 = {{
    {0, 0},
    {1, 0},
    {2, 0},
    {3, 0},
    {3, 1},
    {4, 1},
    {4, 0},
    {5, 0},
    {6, 0},
    {7, 0},
}};
constexpr std::array<sf::Vector2u, 12> posArray12 = {{
    {0, 0},
    {1, 0},
    {2, 0},
    {3, 0},
    {2, 1},
    {3, 1},
    {4, 1},
    {5, 1},
    {4, 0},
    {5, 0},
    {6, 0},
    {7, 0},
}};
constexpr std::array<sf::Vector2u, 16> posArray16 = {{
    {0, 1},
    {1, 1},
    {0, 0},
    {1, 0},
    {2, 0},
    {3, 0},
    {2, 1},
    {3, 1},
    {4, 1},
    {5, 1},
    {4, 0},
    {5, 0},
    {6, 0},
    {7, 0},
    {6, 1},
    {7, 1},
}};

KeyViewerSystem::KeyViewerSystem() :
    m_rainSpeed(sf::seconds(0.4f)), m_rainLength(40.f), m_keySize(10.f), m_gapSize(0.5f), m_rainKeyGapSize(0.5f),
    m_keyShowHitError(false), m_rainShowHitError(false)
{
    m_clock.restart();
}

std::vector<KeyViewerSystem::Key> KeyViewerSystem::getKeys() { return m_keys; }
void KeyViewerSystem::setKeys(const std::vector<Key>& keys)
{
    m_keys = keys;
    for (auto& key : keys)
    {
        m_keyPressed.try_emplace(key.scan, std::vector<Stamp>());
    }
}

void KeyViewerSystem::setKeyLimiter(const std::vector<sf::Keyboard::Scan>& keyLimiter)
{
    m_keys.clear();
    for (size_t i = 0; i < keyLimiter.size(); i++)
    {
        m_keys.push_back({keyLimiter[i], sf::Vector2u(i, 0)});
        m_keyPressed.try_emplace(keyLimiter[i], std::vector<Stamp>());
    }
}
void KeyViewerSystem::setKeyLimiterAuto(const std::vector<sf::Keyboard::Scan>& keyLimiter)
{
    m_keys.clear();
    for (size_t i = 0; i < keyLimiter.size(); i++)
    {
        sf::Vector2u pos;
        switch (keyLimiter.size())
        {
        case 10:
            pos = posArray10[i];
            break;
        case 12:
            pos = posArray12[i];
            break;
        case 16:
            pos = posArray16[i];
            break;
        default:
            pos = sf::Vector2u(i, 0);
            break;
        }
        m_keys.push_back({keyLimiter[i], pos});
        m_keyPressed.try_emplace(keyLimiter[i], std::vector<Stamp>());
    }
}

bool KeyViewerSystem::press(const sf::Keyboard::Scan scan, const std::optional<AdoCpp::HitMargin> hitMargin)
{
    const auto it = m_keyPressed.find(scan);
    if (it != m_keyPressed.end() && (it->second.empty() || !it->second.back().press))
    {
        it->second.push_back(Stamp(true, m_clock.getElapsedTime(), hitMargin));
        if (it->second.size() >= 3 && it->second[it->second.size() - 3].press &&
            it->second[it->second.size() - 3].time.asMilliseconds() + 70 > it->second.back().time.asMilliseconds())
        {
            it->second.back().hitMargin = std::nullopt;
            return false; // keyboardChatterBlocker
        }
    }
    return true; // keyboardChatterBlocker
}

void KeyViewerSystem::release(const sf::Keyboard::Scan scan)
{
    const auto it = m_keyPressed.find(scan);
    if (it != m_keyPressed.end() && !it->second.empty() && it->second.back().press)
        it->second.push_back(Stamp(false, m_clock.getElapsedTime()));
}

void KeyViewerSystem::setReleasedColor(const sf::Color releasedColor)
{
    for (auto& key : m_keys)
        key.releasedColor = releasedColor;
}

void KeyViewerSystem::setPressedColor(const sf::Color pressedColor)
{
    for (auto& key : m_keys)
        key.pressedColor = pressedColor;
}

void KeyViewerSystem::setRainColor(const sf::Color rainColor)
{
    for (auto& key : m_keys)
        key.rainColor = rainColor;
}

void KeyViewerSystem::setRainColorByRow(const sf::Color rainColor, unsigned int row)
{
    for (auto& key : m_keys)
        if (key.pos.y == row)
            key.rainColor = rainColor;
}
bool KeyViewerSystem::getKeyShowHitError() const { return m_keyShowHitError; }
void KeyViewerSystem::setKeyShowHitError(const bool flag) { m_keyShowHitError = flag; }
bool KeyViewerSystem::getRainShowHitError() const { return m_rainShowHitError; }
void KeyViewerSystem::setRainShowHitError(const bool flag) { m_rainShowHitError = flag; }
sf::Time KeyViewerSystem::getRainSpeed() const { return m_rainSpeed; }
void KeyViewerSystem::setRainSpeed(const sf::Time rainSpeed) { m_rainSpeed = rainSpeed; }
float KeyViewerSystem::getRainLength() const { return m_rainLength; }
void KeyViewerSystem::setRainLength(const float rainLength) { m_rainLength = rainLength; }
float KeyViewerSystem::getKeySize() const { return m_keySize; }
void KeyViewerSystem::setKeySize(const float keySize) { m_keySize = keySize; }
float KeyViewerSystem::getGapSize() const { return m_gapSize; }
void KeyViewerSystem::setGapSize(const float gapSize) { m_gapSize = gapSize; }
float KeyViewerSystem::getRainKeyGapSize() const { return m_rainKeyGapSize; }
void KeyViewerSystem::setRainKeyGapSize(const float rainKeyGapSize) { m_rainKeyGapSize = rainKeyGapSize; }

// ReSharper disable once CppMemberFunctionMayBeStatic
void KeyViewerSystem::update()
{
    for (auto& v : m_keyPressed | std::views::values)
    {
        while (v.size() >= 2 && (m_clock.getElapsedTime() - v.front().time).asSeconds() > 10)
            v.erase(v.begin());
    }
}

KeyViewerSystem::Key& KeyViewerSystem::operator[](const size_t index) { return m_keys[index]; }

const KeyViewerSystem::Key& KeyViewerSystem::operator[](const size_t index) const { return m_keys[index]; }

void KeyViewerSystem::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    using enum AdoCpp::HitMargin;
    static constexpr sf::Color green = sf::Color::Green, yellow = sf::Color::Yellow, orange = sf::Color(0xff8800ff),
                               red = sf::Color::Red;

    states.transform *= getTransform();
    states.texture = nullptr;
    sf::RectangleShape sqrPrs{{m_keySize, m_keySize}};
    sf::RectangleShape sqrRls{{m_keySize, m_keySize}};
    const sf::Time time = m_clock.getElapsedTime();
    std::vector<Key> keyLimiter = m_keys;
    std::ranges::sort(keyLimiter, [](const Key& a, const Key& b)
                      { return a.pos.y != b.pos.y ? a.pos.y < b.pos.y : a.pos.x < b.pos.x; });
    for (const auto& [scan, pos, releasedColor, pressedColor, rainColor] : keyLimiter)
    {
        sqrPrs.setFillColor(pressedColor);
        sqrRls.setFillColor(releasedColor);

        // ReSharper disable CppFunctionalStyleCast
        const sf::Vector2f sqrPos{float(pos.x) * (m_keySize + m_gapSize),
                                  m_rainLength + m_rainKeyGapSize + float(pos.y) * (m_keySize + m_gapSize)};
        // ReSharper restore CppFunctionalStyleCast
        std::vector<Stamp> stamps = m_keyPressed.at(scan);
        if (!stamps.empty() && stamps.back().press)
        {
            sqrPrs.setPosition(sqrPos);
            if (m_keyShowHitError)
            {
                std::optional<AdoCpp::HitMargin> hitMargin = stamps.back().hitMargin;
                sf::Color color;
                if (hitMargin)
                    color = hitMargin == Perfect                                ? green
                        : hitMargin == LatePerfect || hitMargin == EarlyPerfect ? yellow
                        : hitMargin == VeryLate || hitMargin == VeryEarly       ? orange
                                                                                : red;
                else
                    color = sf::Color(0x888888ff);
                sqrPrs.setFillColor(color);
            }
            target.draw(sqrPrs, states);
        }
        else
        {
            sqrRls.setPosition(sqrPos);
            target.draw(sqrRls, states);
        }
        if (!stamps.empty() && stamps.back().press)
            stamps.push_back(Stamp(false, time));
        bool press = false;
        float begin = 0, end = 0;
        std::optional<AdoCpp::HitMargin> hitMargin;
        for (const auto& [s_press, s_time, s_hitMargin] : stamps)
        {
            press = s_press;
            if (press)
                begin = (s_time - time) / m_rainSpeed * m_rainLength + m_rainLength, hitMargin = s_hitMargin;
            else
            {
                end = (s_time - time) / m_rainSpeed * m_rainLength + m_rainLength;
                if (end <= 0)
                    continue;
                if (begin < 0)
                    begin = 0;
                sf::RectangleShape rect{{m_keySize, end - begin}};
                if (m_rainShowHitError)
                {
                    sf::Color color;
                    if (hitMargin)
                        color = hitMargin == Perfect                                ? green
                            : hitMargin == LatePerfect || hitMargin == EarlyPerfect ? yellow
                            : hitMargin == VeryLate || hitMargin == VeryEarly       ? orange
                                                                                    : red;
                    else
                        color = sf::Color(0x888888ff);
                    uint8_t value = static_cast<uint8_t>(std::min(255u, 192 * pos.y));
                    color = sf::Color(std::max(color.r, value), std::max(color.g, value), std::max(color.b, value),
                                      255 - value / 2);
                    rect.setFillColor(color);
                }
                else
                {
                    rect.setFillColor(rainColor);
                }
                // ReSharper disable once CppFunctionalStyleCast
                rect.setPosition({float(pos.x) * (m_keySize + m_gapSize), begin});
                target.draw(rect, states);
            }
        }
    }
}
