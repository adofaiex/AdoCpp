#pragma once

#include <SFML/Graphics.hpp>
#include <AdoCpp/Level.h>

class CountDownSystem final : public sf::Drawable, public sf::Transformable
{
public:
    explicit CountDownSystem(const sf::Font& font) : text(font)
    {
        text.setCharacterSize(72);
    }
    void setActiveFloor(const std::optional<size_t> floor)
    {
        activeFloor = floor;
    }
    void update(const AdoCpp::Level& level, const double beat) const
    {
        double countdown;
        if (activeFloor)
            countdown = level.tiles[*activeFloor].beat - beat;
        else
            countdown = -beat;
        if (0 < countdown && countdown < level.settings.countdownTicks)
        {
            const int intCountDown = std::floor(countdown);
            text.setString(intCountDown ? std::to_string(intCountDown) : "Go!");
        }
        else
        {
            text.setString("");
        }
    }
private:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        if (text.getString().isEmpty())
            return;
        text.setOrigin(text.getLocalBounds().getCenter());
        text.setPosition(sf::Vector2f(target.getSize()) / 2.f);
        target.draw(text);
    }
    std::optional<size_t> activeFloor;
    sf::Font font;
    mutable sf::Text text;
};
