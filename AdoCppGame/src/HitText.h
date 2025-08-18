#pragma once

#include <AdoCpp.h>
#include <SFML/Graphics.hpp>
#include <array>
#include <cmath>
#include <random>

class TooEarlySprite final : public sf::Drawable, public sf::Transformable
{
public:
    TooEarlySprite()
    {
        circle.setFillColor(sf::Color::White);
        circle.setRadius(0.125f);
        circle.setOrigin({0.125f, 0.125f});
        rect1.setFillColor(sf::Color::Red);
        rect1.setSize({0.05f, 0.2f});
        rect1.setOrigin({0.025f, 0.1f});
        rect1.setRotation(sf::degrees(45));
        rect2.setFillColor(sf::Color::Red);
        rect2.setSize({0.05f, 0.2f});
        rect2.setOrigin({0.025f, 0.1f});
        rect2.setRotation(sf::degrees(135));
    }

    void setOpacity(const double opacity)
    {
        const sf::Color color(255, 255, 255, static_cast<uint8_t>(opacity * 255));
        circle.setFillColor(sf::Color::White * color);
        rect1.setFillColor(sf::Color::Red * color);
        rect2.setFillColor(sf::Color::Red * color);
    }

private:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        states.transform *= getTransform();
        states.texture = nullptr;
        target.draw(circle, states);
        target.draw(rect1, states);
        target.draw(rect2, states);
    }
    sf::CircleShape circle;
    sf::RectangleShape rect1;
    sf::RectangleShape rect2;
};

class HitTextSystem final : public sf::Drawable, public sf::Transformable
{
public:
    explicit HitTextSystem(sf::Font& l_font) : m_font(l_font) { setFont(l_font); }

    void clear() { m_hitTexts.clear(); }

    void update(const double seconds)
    {
        for (auto it = m_hitTexts.begin(); it < m_hitTexts.end();)
        {
            const auto elapsed = static_cast<float>(seconds - it->secondsBegin);
            if (elapsed > 4)
                it = m_hitTexts.erase(it);
            else
            {
                float a, b;
                if (elapsed <= 0.1f)
                    a = b = elapsed / 0.1f;
                else
                    a = 1.f - (elapsed - 0.1f) / 3.9f, b = 1.f - (elapsed - 0.1f) / 10.f;
                it->opacity = a;
                it->angle = it->endAngle * static_cast<float>(AdoCpp::ease(AdoCpp::Easing::OutQuint, elapsed / 4.f));
                it->scale = {b, b};
                if (it->hitMargin == AdoCpp::HitMargin::TooEarly)
                {
                    it->tes.setPosition(it->position);
                    it->tes.setOpacity(1 - std::fmod(elapsed, 1.0));
                }
                ++it;
            }
        }
    }
    void setFont(const sf::Font& font)
    {
        m_font = font;
        for (size_t i = 0; i < 7; i++)
        {
            m_text[i].setCharacterSize(32);
        }
        static const sf::Color green = sf::Color::Green, yellow = sf::Color::Yellow, orange = sf::Color(0xff8800ff),
                               red = sf::Color::Red;
        using enum AdoCpp::HitMargin;
        // ReSharper disable CppCStyleCast
        m_text[(int)Perfect].setFillColor(green);
        m_text[(int)Perfect].setString("Perfect!");
        m_text[(int)LatePerfect].setFillColor(yellow);
        m_text[(int)LatePerfect].setString("LPerfect!");
        m_text[(int)EarlyPerfect].setFillColor(yellow);
        m_text[(int)EarlyPerfect].setString("EPerfect!");
        m_text[(int)VeryLate].setFillColor(orange);
        m_text[(int)VeryLate].setString("Late!");
        m_text[(int)VeryEarly].setFillColor(orange);
        m_text[(int)VeryEarly].setString("Early!");
        m_text[(int)TooLate].setFillColor(red);
        m_text[(int)TooLate].setString("Late!!");
        m_text[(int)TooEarly].setFillColor(red);
        m_text[(int)TooEarly].setString("Early!!");
        // ReSharper restore CppCStyleCast
    }
    sf::String getText(const AdoCpp::HitMargin hitMargin) const
    {
        return m_text[static_cast<int>(hitMargin)].getString();
    }
    void setText(const AdoCpp::HitMargin hitMargin, const sf::String& text)
    {
        m_text[static_cast<int>(hitMargin)].setString(text);
    }
    void addHitText(const double seconds, const AdoCpp::HitMargin hitMargin, const sf::Vector2f position)
    {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        using urd = std::uniform_real_distribution<float>;
        using enum AdoCpp::HitMargin;
        const float deg = hitMargin == Perfect                      ? 0
            : hitMargin == EarlyPerfect || hitMargin == LatePerfect ? urd(-30.f, 30.f)(gen)
            : hitMargin == VeryEarly || hitMargin == VeryLate       ? urd(-60.f, 60.f)(gen)
                                                                    : urd(-90.f, 90.f)(gen);
        m_hitTexts.push_back({seconds, hitMargin, position, sf::degrees(deg)});
    }
    bool hidePerfects = true;

private:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        states.transform *= getTransform();
        states.texture = nullptr;

        const sf::Vector2f viewCenter(target.getView().getCenter());
        sf::Vector2f viewSize(target.getView().getSize());
        viewSize.x = viewSize.y = (std::max)(viewSize.x, viewSize.y) * 1.5f;
        const sf::FloatRect currentViewRect(viewCenter - viewSize / 2.f, viewSize);
        for (auto& hitText : m_hitTexts)
        {
            if (hidePerfects && hitText.hitMargin == AdoCpp::HitMargin::Perfect)
                continue;
            sf::Text text = m_text[static_cast<int>(hitText.hitMargin)];
            text.setPosition(hitText.position + sf::Vector2f({0, 1}));
            text.setOrigin(text.getLocalBounds().size / 2.f);
            text.setRotation(hitText.angle);
            sf::Vector2f scale = hitText.scale * 0.007f;
            scale.y *= -1;
            text.setScale(scale);
            sf::Color color = text.getFillColor();
            color.a = static_cast<std::uint8_t>(hitText.opacity * 255);
            text.setFillColor(color);
            if (currentViewRect.findIntersection(text.getGlobalBounds()))
            {
                if (hitText.hitMargin == AdoCpp::HitMargin::TooEarly)
                    target.draw(hitText.tes, states);
                target.draw(text, states);
            }
        }
    }

    struct HitText
    {
        double secondsBegin;
        AdoCpp::HitMargin hitMargin;
        sf::Vector2f position;
        sf::Angle endAngle;
        sf::Angle angle;
        sf::Vector2f scale;
        double opacity;
        TooEarlySprite tes;
    };

    std::vector<HitText> m_hitTexts;
    sf::Font& m_font;
    std::array<sf::Text, 7> m_text = {sf::Text(m_font), sf::Text(m_font), sf::Text(m_font), sf::Text(m_font),
                                      sf::Text(m_font), sf::Text(m_font), sf::Text(m_font)};
};
