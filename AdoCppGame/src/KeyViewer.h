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
	void setKeys(const std::vector<Key>& keys);
	void setKeyLimiter(const std::vector<sf::Keyboard::Scan>& keyLimiter);
    void setKeyLimiterAuto(const std::vector<sf::Keyboard::Scan>& keyLimiter);
	bool press(sf::Keyboard::Scan scan, std::optional<AdoCpp::HitMargin> hitMargin = std::nullopt);
	void release(sf::Keyboard::Scan scan);
	void setReleasedColor(sf::Color releasedColor);
	void setPressedColor(sf::Color pressedColor);
	void setRainColor(sf::Color rainColor);
	void setRainColorByRow(sf::Color rainColor, unsigned int row);
    bool getKeyShowHitError() const;
    void setKeyShowHitError(bool flag);
    bool getRainShowHitError() const;
    void setRainShowHitError(bool flag);
    sf::Time getRainSpeed() const;
    void setRainSpeed(sf::Time rainSpeed);
    float getRainLength() const;
    void setRainLength(float rainLength);
    float getKeySize() const;
    void setKeySize(float keySize);
    float getGapSize() const;
    void setGapSize(float gapSize);
    float getRainKeyGapSize() const;
    void setRainKeyGapSize(float rainKeyGapSize);
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