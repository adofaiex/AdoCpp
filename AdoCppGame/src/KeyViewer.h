#pragma once

#include <SFML/Graphics.hpp>

#include "AdoCpp/Level.h"

class KeyViewerSystem final : public sf::Drawable, public sf::Transformable
{
public:
	struct Key
	{
		sf::Keyboard::Scan scan{};
		sf::Vector2u pos;
		sf::Color releasedColor{ 255, 255, 255, 0 };
		sf::Color pressedColor{ 255, 255, 255, 255 };
		sf::Color rainColor{ 255, 255, 255, 255 };
	};
	struct Stamp
	{
		bool press{};
		sf::Time time;
	    std::optional<AdoCpp::HitMargin> hitMargin;
	};

	KeyViewerSystem();
	std::vector<Key> getKeys();
	KeyViewerSystem& setKeys(const std::vector<Key>& keys);
	KeyViewerSystem& setKeyLimiter(const std::vector<sf::Keyboard::Scan>& keyLimiter);
    KeyViewerSystem& setKeyLimiterAuto(const std::vector<sf::Keyboard::Scan>& keyLimiter);
	bool press(sf::Keyboard::Scan scan, std::optional<AdoCpp::HitMargin> hitMargin = std::nullopt);
	KeyViewerSystem& release(sf::Keyboard::Scan scan);
	KeyViewerSystem& setReleasedColor(sf::Color releasedColor);
	KeyViewerSystem& setPressedColor(sf::Color pressedColor);
	KeyViewerSystem& setRainColor(sf::Color rainColor);
	KeyViewerSystem& setRainColorByRow(sf::Color rainColor, unsigned int row);
    bool getKeyShowHitError() const;
    KeyViewerSystem& setKeyShowHitError(bool flag);
    bool getRainShowHitError() const;
    KeyViewerSystem& setRainShowHitError(bool flag);
    sf::Time getRainSpeed() const;
    KeyViewerSystem& setRainSpeed(sf::Time rainSpeed);
    float getRainLength() const;
    KeyViewerSystem& setRainLength(float rainLength);
    float getKeySize() const;
    KeyViewerSystem& setKeySize(float keySize);
    float getGapSize() const;
    KeyViewerSystem& setGapSize(float gapSize);
    float getRainKeyGapSize() const;
    KeyViewerSystem& setRainKeyGapSize(float rainKeyGapSize);
	void update();

	Key& operator[] (size_t index);
	const Key& operator[] (size_t index) const;

private:
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

	std::vector<Key> m_keys;
	std::unordered_map<sf::Keyboard::Scan, std::vector<Stamp>> m_keyPressed;
	sf::Clock m_clock;
	sf::Time m_rainSpeed;
	float m_rainLength;
	float m_keySize;
	float m_gapSize;
	float m_rainKeyGapSize;
    bool m_keyShowHitError;
    bool m_rainShowHitError;
};