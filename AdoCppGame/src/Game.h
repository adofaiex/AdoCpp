#pragma once

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <array>

#include <AdoCpp.h>

#include "Config.h"
#include "Tile.h"

class State;

class Game
{
public:
    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;
    Game();
    ~Game();
    void run();
    void changeState(State* state);
    void pushState(State* state);
    void popState();

    void handleEvent();
    void update();
    void render();

    void createWindow();
    void calcFps();

    sf::RenderWindow window;
    sf::Vector2u windowSize;
    sf::Time deltaTime;
    float fps;
    float planetRadiusPx;

    std::array<float, 60> arrFps;
    float avgFps, minFps, maxFps;

    sf::ContextSettings settings;
    sf::View view;
    sf::Vector2f zoom;
    sf::Font font;
    sf::Text textFps;

    sf::Music music;

    AdoCpp::Level level;
    std::filesystem::path levelPath, musicPath, origMusicPath;

    TileSystem tileSystem;

    using enum sf::Keyboard::Scan;

    std::vector<State*> states;
    std::optional<size_t> activeTileIndex;
    bool autoplay;
    bool fullscreen;

    sf::Clock deltaClock;

    Config config;
};
