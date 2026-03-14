#pragma once
#include "State.h"

class LiveCharting final : public State
{
public:
    void init(Game* _game) override;
    void cleanup() override;

    void pause() override;
    void resume() override;

    void handleEvent(sf::Event event) override;
    void update() override;
    void render() override;
    void renderAudioWindow();
    void renderLevelSettings() const;

    void renderSSong() const;
    void renderSLevel() const;
    void renderSTrack() const;
    void renderSBackground() const;
    void renderSCamera() const;
    void renderSMiscellaneous() const;
    void renderSDecorations() const;

    void renderEventBar() const;
    void renderEventSettings() const;

    void renderEventSetSpeed(AdoCpp::Event::GamePlay::SetSpeed* setSpeed) const;
    void renderEventPositionTrack(AdoCpp::Event::Track::PositionTrack* pt) const;
    void renderEventColorTrack(AdoCpp::Event::Track::ColorTrack* ct) const;
    void renderEventMoveCamera(AdoCpp::Event::Visual::MoveCamera* mc) const;

    void parseUpdateLevel(size_t floor) const;

    static LiveCharting* instance() { return &m_stateLiveCharting; }

protected:
    LiveCharting() = default;

    bool musicPlayable() const { return game->music.getDuration().asMilliseconds() != 0; }

private:
    static LiveCharting m_stateLiveCharting;
    sf::CircleShape planet1, planet2;
    size_t nowTileIndex{}, playerTileIndex{};
    double seconds{}, beat{};
    sf::Clock spareClock;
    double spareClockOffset{};
    std::optional<sf::SoundBuffer> soundBuffer;
    std::optional<sf::Sound> music;
    std::optional<std::vector<double>> samples;
    bool render_needToUpdateOscillogram{};
    bool dragging{};
};
