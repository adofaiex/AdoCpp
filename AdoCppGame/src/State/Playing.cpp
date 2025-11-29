#include "Playing.h"
#include <imgui-SFML.h>
#include <imgui.h>

StatePlaying StatePlaying::m_statePlaying;

void StatePlaying::init(Game* _game)
{
    game = _game;

    planet1.setFillColor(sf::Color::Red);
    planet2.setFillColor(sf::Color::Blue);
    planet1.setRadius(0.25);
    planet2.setRadius(0.25);
    planet1.setOrigin({planet1.getRadius(), planet1.getRadius()});
    planet2.setOrigin({planet2.getRadius(), planet2.getRadius()});

    hitTextSystem.clear();
    hitTextSystem.hidePerfects = game->config.hidePerfects;
    hitErrorMeterSystem.setScale({4, 4});
    hitErrorMeterSystem.clear();
    keyViewerSystem.setKeyLimiterAuto(game->config.keyLimiter)
        .setRainSpeed(game->config.rainSpeed)
        .setRainLength(game->config.rainLength)
        .setKeySize(game->config.keySize)
        .setGapSize(game->config.gapSize)
        .setRainKeyGapSize(game->config.rainKeyGapSize)
        .setKeyShowHitError(game->config.keyShowHitError)
        .setRainShowHitError(game->config.rainShowHitError)
        .setReleasedColor({255, 100, 100, 63})
        .setRainColorByRow({255, 100, 100, 255}, 0)
        .setRainColorByRow({255, 255, 255, 191}, 1);

    game->tileSystem.setActiveTileIndex(std::nullopt);

    keyInputCnt = 0;
    waiting = true;
    if (game->activeTileIndex.value_or(0) == 0)
    {
        playerTileIndex = 0;
        const auto& settings = game->level.settings;
        beat = -settings.countdownTicks;
        seconds = beat * AdoCpp::bpm2crotchet(settings.bpm);
    }
    else
    {
        playerTileIndex = *game->activeTileIndex;
        beat = game->level.tiles[playerTileIndex].beat;
        seconds = game->level.beat2seconds(beat);
    }
    game->window.setKeyRepeatEnabled(false);
    isMusicPlayed = false;

    for (auto& hitCount : hitCounts)
        hitCount = 0;
}

void StatePlaying::cleanup()
{
    if (musicPlayable())
        game->music.stop();
    game->window.setKeyRepeatEnabled(true);
}

void StatePlaying::pause()
{
    if (musicPlayable())
        game->music.pause();
    game->window.setKeyRepeatEnabled(true);
}

void StatePlaying::resume()
{
    if (musicPlayable() && game->music.getStatus() == sf::Music::Status::Paused)
        game->music.play();
    game->window.setKeyRepeatEnabled(false);
}

void StatePlaying::handleEvent(const sf::Event event)
{
    using enum sf::Keyboard::Key;
    if (!ImGui::GetIO().WantCaptureKeyboard)
    {
        if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>())
        {
            if (keyPressed->code == F12)
                game->autoplay = !game->autoplay;
            else if (keyPressed->code == Escape)
                game->activeTileIndex = playerTileIndex, game->popState();
            for (const auto& scan : game->config.keyLimiter)
            {
                if (scan == keyPressed->scancode)
                {
                    keyInputCnt++;
                    std::optional<AdoCpp::HitMargin> hitMargin;
                    if (playerTileIndex == 0 &&
                        game->level.getHitMargin(playerTileIndex + 1, seconds, game->config.difficulty) ==
                            AdoCpp::HitMargin::TooEarly)
                        hitMargin = std::nullopt;
                    else
                    {
                        size_t cnt = 1;
                        do
                        {
                            if (playerTileIndex + cnt >= game->level.tiles.size())
                                hitMargin = std::nullopt;
                            else if (!hitMargin || hitMargin && *hitMargin != AdoCpp::HitMargin::TooEarly)
                                hitMargin =
                                    game->level.getHitMargin(playerTileIndex + cnt, seconds, game->config.difficulty);
                            cnt++;
                        }
                        while (cnt <= keyInputCnt);
                    }
                    const bool needToBlock = !keyViewerSystem.press(scan, hitMargin);
                    if (needToBlock && game->config.blockKeyboardChatter)
                        keyInputCnt--; // keyboardChatterBlocker
                    break;
                }
            }
        }
        else if (const auto* keyReleased = event.getIf<sf::Event::KeyReleased>())
        {
            keyViewerSystem.release(keyReleased->scancode);
        }
    }
}

void StatePlaying::update()
{
    auto& tiles = game->level.tiles;
    const auto& settings = game->level.settings;

    // ReSharper disable CppFunctionalStyleCast
    // Time
    if (game->config.syncWithMusic)
    {
        if (waiting && keyInputCnt > 0)
        {
            // Start the music/timer
            waiting = false;
            keyInputCnt--;

            spareClock.restart();
            if (game->activeTileIndex.value_or(0) != 0)
            {
                const float beginTimer =
                    static_cast<float>(game->level.beat2seconds(tiles[*game->activeTileIndex].beat)) -
                    game->config.inputOffset / 1000;

                if (musicPlayable())
                    game->music.setPlayingOffset(sf::seconds(std::max(0.f, beginTimer)));
                else
                    spareClockOffset =
                        game->level.beat2seconds(tiles[*game->activeTileIndex].beat) - game->config.inputOffset / 1000;

                if (musicPlayable())
                    seconds = game->music.getPlayingOffset().asSeconds() + game->config.inputOffset / 1000;
                else
                    seconds =
                        spareClock.getElapsedTime().asSeconds() + game->config.inputOffset / 1000 + spareClockOffset;
                beat = game->level.seconds2beat(seconds), currentTileIndex = game->level.getFloorByBeat(beat);
            }
            else
            {
                seconds =
                    (std::min)(-settings.countdownTicks * AdoCpp::bpm2crotchet(settings.bpm), -settings.offset / 1000) +
                    game->config.inputOffset / 1000,
                beat = game->level.seconds2beat(seconds);
                if (!musicPlayable())
                    spareClockOffset = -game->config.inputOffset / 1000;
            }
        }
    }
    else
    {
        if (waiting && keyInputCnt > 0)
        {
            // Start the music/timer
            waiting = false;
            keyInputCnt--;

            if (game->activeTileIndex.value_or(0) != 0)
            {
                const float beginTimer =
                    static_cast<float>(game->level.beat2seconds(tiles[*game->activeTileIndex].beat)) -
                    game->config.inputOffset / 1000;
                if (musicPlayable())
                    game->music.setPlayingOffset(sf::seconds(std::max(0.f, beginTimer)));
                seconds = game->level.beat2seconds(tiles[*game->activeTileIndex].beat);
            }
            else
                seconds =
                    (std::min)(-settings.countdownTicks * AdoCpp::bpm2crotchet(settings.bpm), -settings.offset / 1000);
            spareClock.restart();
        }
    }
    updateTime();

    // Update the level
    game->level.update(seconds);

    // Judgement
    if (!waiting)
    {
        // Process
        if (game->autoplay)
        {
            keyInputCnt = 0;
            for (size_t i = playerTileIndex; i < currentTileIndex; i++)
            {
                if (tiles[i + 1].angle.deg() != 999)
                    keyInputCnt++;
            }
        }
        // Judgement
        using enum AdoCpp::HitMargin;
        while (playerTileIndex < tiles.size() - 1 && keyInputCnt-- > 0)
        {
            playerTileIndex++;
            const auto [p, lep, vle] = game->level.getTimingBoundary(playerTileIndex, game->config.difficulty);
            const double timing = game->level.getTiming(playerTileIndex, seconds),
                         x = std::min(65.0 / 2, std::max(-65.0 / 2, timing / vle * 65.0 / 2.0));
            const AdoCpp::HitMargin hitMargin =
                game->level.getHitMargin(playerTileIndex, seconds, game->config.difficulty);
            if (hitMargin == TooEarly)
            {
                playerTileIndex--;
                if (playerTileIndex == 0)
                    break;
                AdoCpp::Vector2lf pos;
                if (AdoCpp::Level::isFirePlanetStatic(playerTileIndex))
                    pos = game->level.getPlanetsPos(playerTileIndex, seconds).second;
                else
                    pos = game->level.getPlanetsPos(playerTileIndex, seconds).first;
                hitTextSystem.addHitText(seconds, hitMargin, {float(pos.x), float(pos.y)});
            }
            else
            {
                if (playerTileIndex != tiles.size() - 1 && tiles[playerTileIndex + 1].angle.deg() == 999)
                    playerTileIndex++;
                hitTextSystem.addHitText(
                    seconds, hitMargin, {float(tiles[playerTileIndex].pos.c.x), float(tiles[playerTileIndex].pos.c.y)});
            }
            hitCounts[static_cast<int>(hitMargin)]++;
            hitErrorMeterSystem.addTick(seconds, hitMargin, x);
        }
        keyInputCnt = 0;
        // "Too late" judgement
        while (playerTileIndex < tiles.size() - 1 &&
               game->level.getHitMargin(playerTileIndex + 1, seconds, game->config.difficulty) == TooLate)
        {
            playerTileIndex++;
            if (tiles[playerTileIndex].angle.deg() != 999)
            {
                hitTextSystem.addHitText(seconds, TooLate,
                                         {static_cast<float>(tiles[playerTileIndex].pos.c.x),
                                          static_cast<float>(tiles[playerTileIndex].pos.c.y)});
                hitErrorMeterSystem.addTick(seconds, TooLate, 65.0 / 2);
                hitCounts[static_cast<int>(TooLate)]++;
            }
        }
    }

    // Update planets' positions
    if (!waiting)
    {
        const auto [p1pos, p2pos] = game->level.getPlanetsPos(playerTileIndex, seconds);
        planet1.setPosition({float(p1pos.x), float(p1pos.y)});
        planet2.setPosition({float(p2pos.x), float(p2pos.y)});
    }
    else
    {
        const auto pos = tiles[playerTileIndex].pos.o;

        if (AdoCpp::Level::isFirePlanetStatic(playerTileIndex))
            planet1.setPosition({float(pos.x), float(pos.y)});
        else
            planet2.setPosition({float(pos.x), float(pos.y)});
    }

    // Update Systems
    game->tileSystem.update();
    hitTextSystem.update(seconds);
    hitErrorMeterSystem.update(seconds);
    hitErrorMeterSystem.setPosition({float(game->windowSize.x) / 2, float(game->windowSize.y) - 100});
    keyViewerSystem.update();
    keyViewerSystem.setPosition({50.f, float(game->windowSize.y) - 500});

    // Update the camera
    game->camera.update(game->level, seconds, playerTileIndex);
    const auto pos = game->camera.position;
    const auto rot = game->camera.rotation, zoom = game->camera.zoom;
    game->view.setCenter({float(pos.x), float(pos.y)});
    game->view.setRotation(sf::degrees(float(rot)));
    const auto w = float(game->windowSize.x), h = float(game->windowSize.y);
    /* aw / (aw + ah) = aw / a(w + h) = w / (w + h)
       ah / (aw + ah) = ah / a(w + h) = h / (w + h) */
    game->zoom = {float(zoom) / 100, float(zoom) / 100};
    game->view.setSize({w / (w + h) * 16 * game->zoom.x, -h / (w + h) * 16 * game->zoom.y});
    // ReSharper restore CppFunctionalStyleCast
}
void StatePlaying::updateTime()
{
    if (waiting)
        return;
    if (game->config.syncWithMusic)
    {
        if (musicPlayable())
        {
            if (game->music.getStatus() == sf::Music::Status::Stopped)
            {
                seconds += spareClock.restart().asSeconds();
                if (!isMusicPlayed && seconds >= game->config.inputOffset / 1000)
                    game->music.play(), spareClock.reset(), isMusicPlayed = true;
            }
            else
                seconds = game->music.getPlayingOffset().asSeconds() + game->config.inputOffset / 1000;
        }
        else
            seconds = spareClock.getElapsedTime().asSeconds() + game->config.inputOffset / 1000 + spareClockOffset;
        beat = game->level.seconds2beat(seconds), currentTileIndex = game->level.getFloorByBeat(beat);
    }
    else
    {
        seconds += spareClock.restart().asSeconds();
        beat = game->level.seconds2beat(seconds), currentTileIndex = game->level.getFloorByBeat(beat);
        if (musicPlayable() && game->music.getStatus() == sf::Music::Status::Stopped && !isMusicPlayed &&
            seconds >= game->config.inputOffset / 1000)
            game->music.play(), isMusicPlayed = true;
    }
}

void StatePlaying::render()
{
    auto& tiles = game->level.tiles;

    // render the world
    game->window.setView(game->view);

    game->window.draw(game->tileSystem);

    if (!waiting || AdoCpp::Level::isFirePlanetStatic(playerTileIndex))
        game->window.draw(planet1);
    if (!waiting || !AdoCpp::Level::isFirePlanetStatic(playerTileIndex))
        game->window.draw(planet2);

    game->window.draw(hitTextSystem);

    // render the GUI
    sf::View defaultView = game->window.getDefaultView();
    defaultView.setSize(sf::Vector2f(game->windowSize));
    defaultView.setCenter(sf::Vector2f(game->windowSize) / 2.f);
    game->window.setView(defaultView);
    game->window.draw(hitErrorMeterSystem);
    game->window.draw(keyViewerSystem);

    static constexpr ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground |
        ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::SetNextWindowSize(ImVec2(0, 0));
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetFontSize(), ImGui::GetFontSize()));
    if (ImGui::Begin("LeftText", nullptr, flags))
    {
        ImGui::Text("FPS: %.0f avg, %.0f min, %.0f max", game->avgFps, game->minFps, game->maxFps);
        static double progress, bpm, kps;
        progress = 100 * static_cast<double>(playerTileIndex) / static_cast<double>(tiles.size() - 1);
        bpm = game->level.getBpmByBeat(beat);
        kps = bpm / 60 / (game->level.getAngle(playerTileIndex + (playerTileIndex + 1 == tiles.size() ? 0 : 1)) / 180);
        ImGui::Text("Progress: %.2f%%", progress);
        ImGui::Text("BPM: %.2f", bpm);
        ImGui::Text("KPS: %.2f", kps);
        ImGui::Text("Floor: %llu", currentTileIndex);
        using enum AdoCpp::HitMargin;
        const float p = hitCounts[(int)Perfect], ep = hitCounts[(int)EarlyPerfect], lp = hitCounts[(int)LatePerfect],
                    ve = hitCounts[(int)VeryEarly], vl = hitCounts[(int)VeryLate], te = hitCounts[(int)TooEarly],
                    tl = hitCounts[(int)TooLate];
        const float all = p + ep + lp + ve + vl + tl + te + tl;
        const float acc = all + te + tl == 0 ? 1 : (p + ep + lp) / (all + te + tl) + p * 0.0001f;
        ImGui::Text("Acc: %.2f%%", acc * 100);
        const float xacc = all == 0 ? 1 : (p + (ep + lp) * 0.75f + (ve + vl) * 0.4f + te * 0.2f) / all;
        ImGui::Text("X-Acc: %.2f%%", xacc * 100);
    }
    ImGui::End();
    ImGui::SetNextWindowSize(ImVec2(0, 0));
    ImGui::SetNextWindowPos(ImVec2(game->windowSize.x / 2, game->windowSize.y), 0, ImVec2(0.5, 1));
    if (ImGui::Begin("BottomText", nullptr, flags))
    {
        using enum AdoCpp::HitMargin;
        constexpr std::array<size_t, 7> indices = {static_cast<int>(TooEarly),     static_cast<int>(VeryEarly),
                                                   static_cast<int>(EarlyPerfect), static_cast<int>(Perfect),
                                                   static_cast<int>(LatePerfect),  static_cast<int>(VeryLate),
                                                   static_cast<int>(TooLate)};
        constexpr std::array<ImVec4, 7> colors = {
            ImVec4(1, 0, 0, 1), ImVec4(1, 0.5, 0, 1), ImVec4(1, 1, 0, 1), ImVec4(0, 1, 0, 1),
            ImVec4(1, 1, 0, 1), ImVec4(1, 0.5, 0, 1), ImVec4(1, 0, 0, 1),
        };
        for (size_t i = 0; i < 7; i++)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, colors[i]);
            ImGui::Text("%llu", hitCounts[indices[i]]);
            if (i != 6)
                ImGui::SameLine();
            ImGui::PopStyleColor();
        }
    }
    ImGui::End();
}
