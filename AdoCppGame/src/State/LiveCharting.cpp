#include "LiveCharting.h"
#include <IconsFontAwesome6.h>
#include <imgui.h>
#include <implot.h>
#include <iostream>
#include <map>
#include <misc/cpp/imgui_stdlib.h>

#include <cmath>
#include "ImGuiFileDialog.h"

LiveCharting LiveCharting::m_stateLiveCharting;

using namespace AdoCpp::Event;

static bool ImGuiInputFilename(const IGFD::FileDialogConfig& fdConfig, const char* fdTitle, const char* filter,
                               const char* text, const char* hint, std::string* pathPtr)
{
    char buffer[1145]{};
    sprintf_s(buffer, "iif %s %s", fdTitle, text);
    if (ImGui::Button(" " ICON_FA_FOLDER " "))
    {
        ImGuiFileDialog::Instance()->OpenDialog(buffer, fdTitle, filter, fdConfig);
    }
    ImGui::SameLine();
    bool val = ImGui::InputTextWithHint(text, hint, pathPtr, ImGuiInputTextFlags_ElideLeft);
    if (ImGuiFileDialog::Instance()->Display(buffer))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            *pathPtr = ImGuiFileDialog::Instance()->GetFilePathName();
            val = true;
        }
        ImGuiFileDialog::Instance()->Close();
    }
    return val;
}

static std::map<std::string, float*> colorBuffers;
static bool ImGuiInputColor(const char* text, AdoCpp::Color* colorPtr)
{
    if (!colorBuffers.contains(text))
        colorBuffers[text] = new float[4];
    const bool val = ImGui::ColorEdit4(text, colorBuffers[text]);
    if (ImGui::IsItemEdited())
        *colorPtr = AdoCpp::Color(
            static_cast<uint8_t>(colorBuffers[text][0] * 255), static_cast<uint8_t>(colorBuffers[text][1] * 255),
            static_cast<uint8_t>(colorBuffers[text][2] * 255), static_cast<uint8_t>(colorBuffers[text][3] * 255));
    if (!ImGui::IsItemActive())
    {
        colorBuffers[text][0] = static_cast<float>(colorPtr->r) / 255.f;
        colorBuffers[text][1] = static_cast<float>(colorPtr->g) / 255.f;
        colorBuffers[text][2] = static_cast<float>(colorPtr->b) / 255.f;
        colorBuffers[text][3] = static_cast<float>(colorPtr->a) / 255.f;
    }
    return val;
}

void LiveCharting::init(Game* _game)
{
    game = _game;

    render_needToUpdateOscillogram = true;
    seconds = 0;

    planet1.setFillColor(sf::Color::Red);
    planet2.setFillColor(sf::Color::Blue);
    planet1.setRadius(0.25);
    planet2.setRadius(0.25);
    planet1.setOrigin({planet1.getRadius(), planet1.getRadius()});
    planet2.setOrigin({planet2.getRadius(), planet2.getRadius()});
    if (!game->origMusicPath.empty())
    {
        try
        {
            soundBuffer = sf::SoundBuffer(game->origMusicPath);
            music = sf::Sound(*soundBuffer);
            const int16_t* origSamples = soundBuffer->getSamples();
            const size_t sampleCount = soundBuffer->getSampleCount();
            samples = std::vector<double>(sampleCount);
            int16_t maxSample = 1;
            for (size_t i = 0; i < sampleCount; i++)
                maxSample = std::max(maxSample, static_cast<int16_t>(std::abs(origSamples[i])));
            for (size_t i = 0; i < sampleCount; i++)
                (*samples)[i] = static_cast<double>(origSamples[i]) / maxSample;
        }
        catch (std::exception& ex)
        {
            std::cerr << ex.what() << std::endl;
            soundBuffer = std::nullopt;
        }
    }
}
void LiveCharting::cleanup()
{
    game->level.parse(false, true);
    game->level.update();
    game->tileSystem.parse();
    game->tileSystem.update();
    if (music)
        music->stop();
}
void LiveCharting::pause() {}
void LiveCharting::resume() {}
void LiveCharting::handleEvent(const sf::Event event)
{
    using enum sf::Keyboard::Key;
    if (!ImGui::GetIO().WantCaptureKeyboard)
    {
        if (const auto keyPressed = event.getIf<sf::Event::KeyPressed>())
        {
            if (keyPressed->code == F12)
                game->autoplay = !game->autoplay;
            else if (keyPressed->code == Escape)
                game->popState();
            const std::map<sf::Keyboard::Key, double> keyMap = {{D, 0},   {E, 45},  {W, 90},  {Q, 135},
                                                                {A, 180}, {Z, 225}, {X, 270}, {C, 315}};
            const std::map<sf::Keyboard::Key, double> shiftKeyMap = {{J, 30},  {Y, 60},  {T, 120}, {H, 150},
                                                                     {N, 210}, {V, 240}, {B, 300}, {M, 330}};
            const std::map<sf::Keyboard::Key, double> shiftGraveKeyMap = {{J, 15},  {Y, 75},  {T, 105}, {H, 165},
                                                                          {N, 195}, {V, 255}, {B, 285}, {M, 345}};
            for (const auto& [key, value] : !sf::Keyboard::isKeyPressed(LShift) ? keyMap
                     : !sf::Keyboard::isKeyPressed(Grave)                       ? shiftKeyMap
                                                                                : shiftGraveKeyMap)
                if (keyPressed->code == key)
                {
                    if (game->activeTileIndex)
                    {
                        game->level.insertTile(*game->activeTileIndex + 1, value);
                        (*game->activeTileIndex)++;
                    }
                    else
                    {
                        game->level.pushBackTile(value);
                        game->activeTileIndex = game->level.tiles.size() - 1;
                    }
                    parseUpdateLevel(*game->activeTileIndex);
                }
            if (game->activeTileIndex)
            {
                if ((keyPressed->code == Backspace || keyPressed->code == Delete))
                {
                    if (sf::Keyboard::isKeyPressed(LControl))
                    {
                        if (keyPressed->code == Backspace)
                            game->level.eraseTile(0, *game->activeTileIndex), *game->activeTileIndex = 0;
                        else
                            game->level.eraseTile(*game->activeTileIndex + 1, game->level.tiles.size());
                    }
                    else
                    {
                        game->level.eraseTile(*game->activeTileIndex, *game->activeTileIndex + 1);
                        (*game->activeTileIndex)--;
                    }
                    parseUpdateLevel(*game->activeTileIndex);
                }
                if (keyPressed->code == Tab)
                {
                    game->level.insertTile(*game->activeTileIndex + 1, 999);
                    parseUpdateLevel(*game->activeTileIndex);
                }
                if (keyPressed->code == Space && sf::Keyboard::isKeyPressed(LShift))
                {
                    const double tileAngle = game->level.tiles[*game->activeTileIndex].angle.deg();
                    const double angle = AdoCpp::degrees(tileAngle + 180).wrapUnsigned().deg();
                    game->level.insertTile(*game->activeTileIndex + 1, angle);
                    parseUpdateLevel(*game->activeTileIndex);
                }
                if (keyPressed->code == Left)
                {
                    if (sf::Keyboard::isKeyPressed(LControl))
                        *game->activeTileIndex = 0;
                    else if (*game->activeTileIndex != 0)
                        (*game->activeTileIndex)--;
                }
                if (keyPressed->code == Right)
                {
                    if (sf::Keyboard::isKeyPressed(LControl))
                        *game->activeTileIndex = game->level.tiles.size() - 1;
                    else if (*game->activeTileIndex != game->level.tiles.size() - 1)
                        (*game->activeTileIndex)++;
                }
                if (keyPressed->code == O)
                {
                    AdoCpp::Angle& angle = game->level.tiles[*game->activeTileIndex].angle;
                    angle += AdoCpp::degrees(15), angle = angle.wrapUnsigned();
                    parseUpdateLevel(*game->activeTileIndex);
                }
                if (keyPressed->code == P)
                {
                    AdoCpp::Angle& angle = game->level.tiles[*game->activeTileIndex].angle;
                    angle -= AdoCpp::degrees(15), angle = angle.wrapUnsigned();
                    parseUpdateLevel(*game->activeTileIndex);
                }
            }
        }
    }
    static sf::Vector2f draggingPosition;
    if (!ImGui::GetIO().WantCaptureMouse)
    {
        if (const auto mws = event.getIf<sf::Event::MouseWheelScrolled>())
        {
            if (mws->delta > 0)
                game->zoom /= 1.5f;
            else
                game->zoom *= 1.5f;
        }
        if (const auto mbp = event.getIf<sf::Event::MouseButtonPressed>())
        {
            if (mbp->button == sf::Mouse::Button::Left)
            {
                bool active = false;
                game->window.setView(game->view);
                const auto mouseCoords = game->window.mapPixelToCoords(mbp->position);
                for (size_t i = 0; i < game->level.tiles.size(); i++)
                {
                    if (game->tileSystem[i].isPointInside(mouseCoords))
                    {
                        game->activeTileIndex = i;
                        active = true;
                        break;
                    }
                }
                if (!active)
                {
                    game->activeTileIndex = std::nullopt;
                    dragging = true;
                    const sf::Vector2f center = game->view.getCenter();
                    game->view.setCenter({0, 0});
                    game->window.setView(game->view);
                    draggingPosition = game->window.mapPixelToCoords(mbp->position);
                    game->view.setCenter(center);
                }
            }
        }
        if (const auto mm = event.getIf<sf::Event::MouseMoved>())
        {
            if (dragging)
            {
                const sf::Vector2f center = game->view.getCenter();
                game->view.setCenter({0, 0});
                game->window.setView(game->view);
                const sf::Vector2f now = game->window.mapPixelToCoords(mm->position), delta = now - draggingPosition;
                draggingPosition = now;
                game->view.setCenter(center - delta);
            }
        }
    }
    else
        dragging = false;
}
void LiveCharting::update()
{
    const auto w = static_cast<float>(game->windowSize.x), h = static_cast<float>(game->windowSize.y);
    game->view.setSize({w / (w + h) * 16 * game->zoom.x, -h / (w + h) * 16 * game->zoom.y});

    if (game->level.isParsed())
    {
        const auto [pos1, pos2] = game->level.getPlanetsPos(game->level.getFloorBySeconds(seconds), seconds);
        planet1.setPosition({static_cast<float>(pos1.x), static_cast<float>(pos1.y)});
        planet2.setPosition({static_cast<float>(pos2.x), static_cast<float>(pos2.y)});
    }

    game->tileSystem.setActiveTileIndex(game->activeTileIndex);
    if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift))
        game->tileSystem.setTilePlaceMode(1);
    else if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Grave))
        game->tileSystem.setTilePlaceMode(2);
    else
        game->tileSystem.setTilePlaceMode(3);
    game->tileSystem.update();

    if (!sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
        dragging = false;
}
void LiveCharting::render()
{
    // render the world
    game->window.setView(game->view);
    game->window.draw(game->tileSystem);
    game->window.draw(planet1);
    game->window.draw(planet2);

    // render the GUI
    sf::View defaultView = game->window.getDefaultView();
    defaultView.setSize(sf::Vector2f(game->windowSize));
    defaultView.setCenter(sf::Vector2f(game->windowSize) / 2.f);
    game->window.setView(defaultView);

    renderAudioWindow();
    renderLevelSettings();
    renderEventBar();
    renderEventSettings();
}
void LiveCharting::renderAudioWindow()
{
    constexpr ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground;
    ImGui::SetNextWindowPos({0, 0});
    ImGui::SetNextWindowSize({static_cast<float>(game->windowSize.x), -1});
    if (ImGui::Begin("AudioWindow", nullptr, flags))
    {
        if (ImPlot::BeginPlot("Audio"))
        {
            // Thanks to https://github.com/epezent/implot/issues/323
            ImPlot::SetupAxes("Time [s]", "Amplitude");
            ImPlot::SetupAxisLimits(ImAxis_Y1, -1, 1, ImPlotCond_Always);

            if (samples)
            {
                const double audioLengthInSeconds = static_cast<double>(soundBuffer->getSampleCount()) /
                    soundBuffer->getChannelCount() / soundBuffer->getSampleRate();
                ImPlot::SetupAxisLimitsConstraints(ImAxis_X1, -audioLengthInSeconds / 2, audioLengthInSeconds * 3 / 2);
                const size_t widthPx = ImPlot::GetPlotSize().x;
                double t1 = ImPlot::GetPlotLimits().X.Min;
                double t2 = ImPlot::GetPlotLimits().X.Max;
                static int lastWp;
                static double lastT1, lastT2;
                static std::vector<double> byLow, byHigh, bTime;

                if (widthPx != lastWp || t1 != lastT1 || t2 != lastT2 || render_needToUpdateOscillogram)
                {
                    render_needToUpdateOscillogram = false;
                    lastWp = widthPx, lastT1 = t1, lastT2 = t2;
                    t1 = std::max(t1, 0.0), t2 = std::min(t2, audioLengthInSeconds);
                    byLow.resize(widthPx), byHigh.resize(widthPx), bTime.resize(widthPx);
                    // clang-format off
                    const double frames      = floor((t2 - t1) * soundBuffer->getSampleRate());
                    const double framesPerPx = frames / widthPx;
                    const int    binLength   = std::max(1.0, floor(framesPerPx + 0.5));
                    const int    framesTotal = soundBuffer->getSampleCount() / soundBuffer->getChannelCount();
                    for (int i = 0; i < widthPx; ++i)
                    {
                        const int binBegin_ = floor(t1 * soundBuffer->getSampleRate() + i * framesPerPx);
                        const int binBegin  = std::max(0, std::min(framesTotal - 1, binBegin_));
                        const int binEnd    = std::max(0, std::min(framesTotal - 1, binBegin_ + binLength - 1));
                        bTime[i]  =  t1 + i * (t2 - t1) / static_cast<double>(widthPx - 1);
                        byLow[i]  =  std::numeric_limits<double>::infinity();
                        byHigh[i] = -std::numeric_limits<double>::infinity();
                        for (int j = binBegin; j <= binEnd; ++j)
                        {
                            const int j1 = j * soundBuffer->getChannelCount();
                            double value = samples->at(j1);
                            byLow[i]     = std::min(byLow[i], value);
                            byHigh[i]    = std::max(byHigh[i], value);
                        }
                    }
                    // clang-format on
                }
                ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, 0.25f);
                ImPlot::PlotShaded("##L", bTime.data(), byLow.data(), byHigh.data(), widthPx);
                ImPlot::PopStyleVar();
                ImPlot::PlotLine("##L", bTime.data(), byLow.data(), widthPx);
                ImPlot::PlotLine("##L", bTime.data(), byHigh.data(), widthPx);

                // clang-format off
                const     double dummyX[] = {0, audioLengthInSeconds};
                constexpr double dummyY[] = {1,                   -1};
                // clang-format on
                ImPlot::SetNextLineStyle(ImVec4(0, 0, 0, 0));
                ImPlot::PlotLine("##DummyPointsForFitting", dummyX, dummyY, 4);
            }

            const auto& tiles = game->level.tiles;
            for (size_t i = 0; i < tiles.size(); ++i)
            {
                const auto& tile = tiles[i];
                if (game->activeTileIndex && *game->activeTileIndex == i)
                {
                    ImPlot::TagX(tile.seconds, ImVec4(0, 1, 0, 1));
                    ImPlot::SetNextLineStyle(ImVec4(0, 1, 0, 1));
                }
                else
                    ImPlot::SetNextLineStyle(ImVec4(1, 1, 0, 1));
                ImPlot::PlotInfLines("##TileSecond", &tile.seconds, 1);
            }
            ImPlot::DragLineX(114514, &seconds, ImVec4(1, 0, 0, 1));
            ImPlot::TagX(seconds, ImVec4(1, 0, 0, 1));

            ImPlot::EndPlot();
        }
        static bool play = false, musicPlayed = false;
        if (ImGui::Button(play ? " " ICON_FA_PAUSE " Pause" : " " ICON_FA_PLAY " Play"))
        {
            play = !play, musicPlayed = false;
            if (play)
            {
                spareClock.restart();
            }
            else if (music)
            {
                music->stop();
            }
        }
        if (play)
        {
            seconds += spareClock.restart().asSeconds();
        }
        if (play && music && !musicPlayed)
        {
            musicPlayed = true;
            if (seconds > 0)
                music->setPlayingOffset(sf::seconds(seconds));
            music->play();
        }
    }
    ImGui::End();
}
void LiveCharting::renderLevelSettings() const
{
    constexpr ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
    const float width = ImGui::GetFontSize() * 20, height = ImGui::GetFontSize() * 20;
    ImGui::SetNextWindowPos({0, static_cast<float>(game->windowSize.y)}, 0, {0, 1});
    ImGui::SetNextWindowSize({width, height});
    if (ImGui::Begin("Settings", nullptr, flags))
    {
        static constexpr std::array<const char* const, 7> tabBarTitles = {
            "Song", "Level", "Track", "Background", "Camera", "Miscellaneous", "Decorations"};
        static constexpr std::array<const char* const, 7> titles = {
            "Song Settings",   "Level Settings",         "Track Settings", "Background Settings",
            "Camera Settings", "Miscellaneous Settings", "Decorations"};
        static constexpr std::array funcs = {&LiveCharting::renderSSong,       &LiveCharting::renderSLevel,
                                             &LiveCharting::renderSTrack,      &LiveCharting::renderSBackground,
                                             &LiveCharting::renderSCamera,     &LiveCharting::renderSMiscellaneous,
                                             &LiveCharting::renderSDecorations};
        if (ImGui::BeginTabBar("EventTabBar", ImGuiTabBarFlags_FittingPolicyScroll))
        {
            for (int i = 0; i < 7; ++i)
            {
                if (ImGui::BeginTabItem(tabBarTitles[i]))
                {
                    ImGui::SetCursorPosX(ImGui::GetWindowSize().x / 2 - ImGui::CalcTextSize(titles[i]).x / 2);
                    ImGui::Text(titles[i]);
                    (this->*funcs[i])();
                    ImGui::EndTabItem();
                }
            }
            ImGui::EndTabBar();
        }
    }
    ImGui::End();
}
void LiveCharting::renderEventBar() const
{
    constexpr ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
    const float barWidth = ImGui::GetFontSize() * 20, barHeight = ImGui::GetFontSize() * 5;
    ImGui::SetNextWindowPos({game->windowSize.x / 2.f, static_cast<float>(game->windowSize.y)}, 0, {0.5, 1});
    ImGui::SetNextWindowSize({barWidth, barHeight});
    if (game->activeTileIndex)
    {
        if (ImGui::Begin("EventBar", nullptr, flags))
        {
            if (ImGui::BeginTabBar("EventTabBar"))
            {
                using namespace AdoCpp::Event;
                if (ImGui::BeginTabItem("GamePlay"))
                {
                    using namespace GamePlay;
                    if (ImGui::Button("Set Speed"))
                    {
                        const auto e = std::make_shared<SetSpeed>();
                        e->floor = *game->activeTileIndex;
                        game->level.tiles[e->floor].events.push_back(e), parseUpdateLevel(e->floor);
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Twirl"))
                    {
                        const auto e = std::make_shared<Twirl>();
                        e->floor = *game->activeTileIndex;
                        game->level.tiles[e->floor].events.push_back(e), parseUpdateLevel(e->floor);
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Pause"))
                    {
                        const auto e = std::make_shared<Pause>();
                        e->floor = *game->activeTileIndex;
                        game->level.tiles[e->floor].events.push_back(e), parseUpdateLevel(e->floor);
                    }
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Track"))
                {
                    using namespace Track;
                    if (ImGui::Button("Position Track"))
                    {
                        const auto e = std::make_shared<PositionTrack>();
                        e->floor = *game->activeTileIndex;
                        game->level.tiles[e->floor].events.push_back(e), parseUpdateLevel(e->floor);
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Set Track Color"))
                    {
                        const auto e = std::make_shared<ColorTrack>();
                        e->floor = *game->activeTileIndex;
                        game->level.tiles[e->floor].events.push_back(e), parseUpdateLevel(e->floor);
                    }
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
        }
        ImGui::End();
    }
}
void LiveCharting::renderEventSettings() const
{
    if (game->activeTileIndex)
    {
        auto& tile = game->level.tiles[*game->activeTileIndex];
        constexpr ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
        const float width = ImGui::GetFontSize() * 20, height = ImGui::GetFontSize() * 20;
        ImGui::SetNextWindowPos({static_cast<float>(game->windowSize.x), static_cast<float>(game->windowSize.y)}, 0,
                                {1, 1});
        ImGui::SetNextWindowSize({width, height});
        if (ImGui::Begin("Event", nullptr, flags)) // FIXME
        {
            if (ImGui::BeginTabBar("EventTabBar",
                                   ImGuiTabBarFlags_FittingPolicyScroll | ImGuiTabBarFlags_AutoSelectNewTabs))
            {
                for (size_t i = 0; i < tile.events.size(); i++)
                {
                    char buffer[114]{};
                    sprintf_s(buffer, "%s##EventTabBar[%llu]", tile.events[i]->name(), i);
                    if (ImGui::BeginTabItem(buffer))
                    {
                        const char* title = tile.events[i]->name();
                        ImGui::Text(title);
                        ImGui::SameLine();
                        if (ImGui::Button("Delete"))
                        {
                            tile.events.erase(tile.events.begin() + i);
                            parseUpdateLevel(*game->activeTileIndex);
                            ImGui::EndTabItem();
                            i--;
                            continue;
                        }
                        const std::shared_ptr<Event> event = tile.events[i];

                        if (ImGui::Checkbox("Active", &event->active))
                            parseUpdateLevel(*game->activeTileIndex);

                        using namespace AdoCpp::Event::GamePlay;
                        if (const auto setSpeed = std::dynamic_pointer_cast<SetSpeed>(event))
                            renderEventSetSpeed(setSpeed.get());
                        if (const auto pause = std::dynamic_pointer_cast<Pause>(event))
                        {
                            if (ImGui::InputDouble("Duration##Pause", &pause->duration, 0, 0, "%g"))
                                parseUpdateLevel(*game->activeTileIndex);
                            if (ImGui::InputDouble("Countdown Ticks", &pause->countdownTicks, 0, 0, "%g"))
                                parseUpdateLevel(*game->activeTileIndex);
                        }
                        using namespace AdoCpp::Event::Track;
                        if (const auto pt = std::dynamic_pointer_cast<PositionTrack>(event))
                            renderEventPositionTrack(pt.get());
                        if (const auto ct = std::dynamic_pointer_cast<ColorTrack>(event))
                            renderEventColorTrack(ct.get());
                        ImGui::EndTabItem();
                    }
                }
                ImGui::EndTabBar();
            }
        }
        ImGui::End();
    }
}
void LiveCharting::renderEventSetSpeed(GamePlay::SetSpeed* setSpeed) const
{
    ImGui::Text("SpeedType");
    ImGui::SameLine();
    if (ImGui::RadioButton("BPM", setSpeed->speedType == GamePlay::SetSpeed::SpeedType::Bpm))
    {
        setSpeed->speedType = GamePlay::SetSpeed::SpeedType::Bpm;
        parseUpdateLevel(*game->activeTileIndex);
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Multiplier", setSpeed->speedType == GamePlay::SetSpeed::SpeedType::Multiplier))
    {
        setSpeed->speedType = GamePlay::SetSpeed::SpeedType::Multiplier;
        parseUpdateLevel(*game->activeTileIndex);
    }
    if (setSpeed->speedType == GamePlay::SetSpeed::SpeedType::Multiplier)
        ImGui::BeginDisabled();
    if (ImGui::InputDouble("Beats Per Minute##SetSpeed", &setSpeed->beatsPerMinute, 0, 0, "%g"))
        parseUpdateLevel(*game->activeTileIndex);
    if (setSpeed->speedType == GamePlay::SetSpeed::SpeedType::Multiplier)
        ImGui::EndDisabled();
    if (setSpeed->speedType == GamePlay::SetSpeed::SpeedType::Bpm)
        ImGui::BeginDisabled();
    if (ImGui::InputDouble("BPM Multiplier##SetSpeed", &setSpeed->bpmMultiplier, 0, 0, "%g"))
        parseUpdateLevel(*game->activeTileIndex);
    if (setSpeed->speedType == GamePlay::SetSpeed::SpeedType::Bpm)
        ImGui::EndDisabled();
}
void LiveCharting::renderEventPositionTrack(Track::PositionTrack* pt) const
{
    if (ImGui::InputDouble("X", &pt->positionOffset.x, 0, 0, "%g"))
        parseUpdateLevel(*game->activeTileIndex);
    if (ImGui::InputDouble("Y", &pt->positionOffset.y, 0, 0, "%g"))
        parseUpdateLevel(*game->activeTileIndex);
    {
        static int selected;
        bool changed = false;
        selected = static_cast<int>(pt->relativeTo.relativeTo);
        ImGui::Text("Relative to");
        if (ImGui::BeginCombo("##relTo", AdoCpp::cstrRelativeToTile[selected]))
        {
            for (int n = 0; n < IM_ARRAYSIZE(AdoCpp::cstrRelativeToTile); n++)
            {
                const bool is_selected = selected == n;
                if (ImGui::Selectable(AdoCpp::cstrRelativeToTile[n], is_selected))
                    selected = n, changed = true;
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        if (changed)
            pt->relativeTo.relativeTo = static_cast<AdoCpp::RelativeToTile>(selected),
            parseUpdateLevel(*game->activeTileIndex);
    }
    if (ImGui::InputScalar("tiles", ImGuiDataType_U64, &pt->relativeTo.index, nullptr, nullptr, "%llu"))
        parseUpdateLevel(*game->activeTileIndex);
    if (ImGui::InputDouble("Rotation", &pt->rotation, 0, 0, "%g"))
        parseUpdateLevel(*game->activeTileIndex);
    if (ImGui::InputDouble("Scale", &pt->scale, 0, 0, "%g"))
        parseUpdateLevel(*game->activeTileIndex);
    if (ImGui::InputDouble("Opacity", &pt->opacity, 0, 0, "%g"))
        parseUpdateLevel(*game->activeTileIndex);
    if (ImGui::Checkbox("Editor Only", &pt->editorOnly))
        parseUpdateLevel(*game->activeTileIndex);
    if (ImGui::Checkbox("Just This Tile", &pt->justThisTile))
        parseUpdateLevel(*game->activeTileIndex);
}
void LiveCharting::renderEventColorTrack(Track::ColorTrack* ct) const
{
    {
        static int selected;
        bool changed = false;
        selected = static_cast<int>(ct->trackColorType);
        if (ImGui::BeginCombo("Track Color Type", AdoCpp::cstrTrackColorType[selected]))
        {
            for (int n = 0; n < IM_ARRAYSIZE(AdoCpp::cstrTrackColorType); n++)
            {
                const bool is_selected = selected == n;
                if (ImGui::Selectable(AdoCpp::cstrTrackColorType[n], is_selected))
                    selected = n, changed = true;
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        if (changed)
            ct->trackColorType = static_cast<AdoCpp::TrackColorType>(selected),
            parseUpdateLevel(*game->activeTileIndex);
    }
    if (ImGuiInputColor("Track Color##ColorTrack", &ct->trackColor))
        parseUpdateLevel(*game->activeTileIndex);
    if (ImGuiInputColor("Secondary Track Color##ColorTrack", &ct->secondaryTrackColor))
        parseUpdateLevel(*game->activeTileIndex);
    if (ImGui::InputDouble("Track Color Animation Duration##ColorTrack", &ct->trackColorAnimDuration, 0, 0, "%g"))
        parseUpdateLevel(*game->activeTileIndex);
    {
        static int selected;
        bool changed = false;
        selected = static_cast<int>(ct->trackColorPulse) + 1;
        if (ImGui::BeginCombo("Track Color Pulse##ColorTrack", AdoCpp::cstrTrackColorPulse[selected]))
        {
            for (int n = 0; n < IM_ARRAYSIZE(AdoCpp::cstrTrackColorPulse); n++)
            {
                const bool is_selected = selected == n;
                if (ImGui::Selectable(AdoCpp::cstrTrackColorPulse[n], is_selected))
                    selected = n, changed = true;
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        if (changed)
            ct->trackColorPulse = static_cast<AdoCpp::TrackColorPulse>(selected - 1),
            parseUpdateLevel(*game->activeTileIndex);
    }
    ImGui::InputScalar("Track Pulse Length##ColorTrack", ImGuiDataType_U32, &ct->trackPulseLength);
    {
        static int selected;
        bool changed = false;
        selected = static_cast<int>(ct->trackStyle);
        if (ImGui::BeginCombo("Track Style##ColorTrack", AdoCpp::cstrTrackStyle[selected]))
        {
            for (int n = 0; n < IM_ARRAYSIZE(AdoCpp::cstrTrackStyle); n++)
            {
                const bool is_selected = selected == n;
                if (ImGui::Selectable(AdoCpp::cstrTrackStyle[n], is_selected))
                    selected = n, changed = true;
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        if (changed)
            ct->trackStyle = static_cast<AdoCpp::TrackStyle>(selected), parseUpdateLevel(*game->activeTileIndex);
    }
}
void LiveCharting::parseUpdateLevel(const size_t floor) const
{
    game->level.parse(floor, true, true);
    game->level.update();
    game->tileSystem.parse();
    game->tileSystem.update();
}
void LiveCharting::renderSSong() const
{
    auto& settings = game->level.settings;
    IGFD::FileDialogConfig cfg;
    cfg.path = game->levelPath.parent_path().string();
    cfg.flags = ImGuiFileDialogFlags_Modal;
    if (ImGuiInputFilename(cfg, "Select a file", ".ogg", "Song Filename", "No files selected", &settings.songFilename))
        parseUpdateLevel(0);
    if (ImGui::InputDouble("BPM##SongSettings", &settings.bpm, 0, 0, "%g"))
        parseUpdateLevel(0);
    if (ImGui::InputDouble("Volume##SongSettings", &settings.volume, 0, 0, "%g"))
        parseUpdateLevel(0);
    if (ImGui::InputDouble("Offset##SongSettings", &settings.offset, 0, 0, "%g"))
        parseUpdateLevel(0);
    if (ImGui::InputDouble("Pitch##SongSettings", &settings.pitch, 0, 0, "%g"))
        parseUpdateLevel(0);
    {
        static int selected;
        selected = static_cast<int>(settings.hitsound);
        bool changed = false;
        if (ImGui::BeginCombo("Hitsound", AdoCpp::cstrHitsound[selected]))
        {
            for (int n = 0; n < IM_ARRAYSIZE(AdoCpp::cstrHitsound); n++)
            {
                const bool is_selected = selected == n;
                if (ImGui::Selectable(AdoCpp::cstrHitsound[n], is_selected))
                    selected = n, changed = true;
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        if (changed)
            settings.hitsound = static_cast<AdoCpp::Hitsound>(selected), parseUpdateLevel(0);
    }
    if (ImGui::InputDouble("Hitsound Volume##SongSettings", &settings.hitsoundVolume, 0, 0, "%g"))
        parseUpdateLevel(0);
    if (ImGui::InputDouble("Countdown Ticks##SongSettings", &settings.countdownTicks, 0, 0, "%g"))
        parseUpdateLevel(0);
}
void LiveCharting::renderSLevel() const
{
    auto& settings = game->level.settings;
    if (ImGui::InputText("Artist##LevelSettings", &settings.artist))
        parseUpdateLevel(0);
    if (ImGui::InputText("Song##LevelSettings", &settings.song))
        parseUpdateLevel(0);
    if (ImGui::InputText("Author##LevelSettings", &settings.author))
        parseUpdateLevel(0);
    if (ImGui::Checkbox("Separate Countdown Time##LevelSettings", &settings.separateCountdownTime))
        parseUpdateLevel(0);
}
void LiveCharting::renderSTrack() const
{
    auto& settings = game->level.settings;
    {
        static int selected;
        selected = static_cast<int>(settings.trackColorType);
        bool changed = false;
        if (ImGui::BeginCombo("Track Color Type##TrackSettings", AdoCpp::cstrTrackColorType[selected]))
        {
            for (int n = 0; n < IM_ARRAYSIZE(AdoCpp::cstrTrackColorType); n++)
            {
                const bool is_selected = selected == n;
                if (ImGui::Selectable(AdoCpp::cstrTrackColorType[n], is_selected))
                    selected = n, changed = true;
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        if (changed)
            settings.trackColorType = static_cast<AdoCpp::TrackColorType>(selected), parseUpdateLevel(0);
    }

    if (ImGuiInputColor("Track Color##TrackSettings", &settings.trackColor))
        parseUpdateLevel(0);
    if (ImGuiInputColor("Secondary Track Color##TrackSettings", &settings.trackColor))
        parseUpdateLevel(0);
    if (ImGui::InputDouble("Track Color Animation Duration##TrackSettings", &settings.trackColorAnimDuration, 0, 0,
                           "%g"))
        parseUpdateLevel(0);
    {
        static int selected;
        selected = static_cast<int>(settings.trackColorPulse) + 1;
        bool changed = false;
        if (ImGui::BeginCombo("Track Color Pulse##TrackSettings", AdoCpp::cstrTrackColorPulse[selected]))
        {
            for (int n = 0; n < IM_ARRAYSIZE(AdoCpp::cstrTrackColorPulse); n++)
            {
                const bool is_selected = selected == n;
                if (ImGui::Selectable(AdoCpp::cstrTrackColorPulse[n], is_selected))
                    selected = n, changed = true;
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        if (changed)
            settings.trackColorPulse = static_cast<AdoCpp::TrackColorPulse>(selected - 1), parseUpdateLevel(0);
    }
    if (ImGui::InputScalar("Track Pulse Length##TrackSettings", ImGuiDataType_U32, &settings.trackPulseLength))
        parseUpdateLevel(0);
    {
        static int selected;
        selected = static_cast<int>(settings.trackStyle);
        bool changed = false;
        if (ImGui::BeginCombo("Track Style##TrackSettings", AdoCpp::cstrTrackStyle[selected]))
        {
            for (int n = 0; n < IM_ARRAYSIZE(AdoCpp::cstrTrackStyle); n++)
            {
                const bool is_selected = selected == n;
                if (ImGui::Selectable(AdoCpp::cstrTrackStyle[n], is_selected))
                    selected = n, changed = true;
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        if (changed)
            settings.trackStyle = static_cast<AdoCpp::TrackStyle>(selected), parseUpdateLevel(0);
    }
    {
        static int selected;
        selected = static_cast<int>(settings.trackAnimation);
        if (ImGui::BeginCombo("Track Animation##TrackSettings", AdoCpp::cstrTrackAnimation[selected]))
        {
            for (int n = 0; n < IM_ARRAYSIZE(AdoCpp::cstrTrackAnimation); n++)
            {
                const bool is_selected = selected == n;
                if (ImGui::Selectable(AdoCpp::cstrTrackAnimation[n], is_selected))
                    selected = n;
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        settings.trackAnimation = static_cast<AdoCpp::TrackAnimation>(selected);
    }
    if (ImGui::InputDouble("Beats ahead##TrackSettings", &settings.beatsAhead, 0, 0, "%g"))
        parseUpdateLevel(0);
    {
        static int selected;
        selected = static_cast<int>(settings.trackDisappearAnimation);
        bool changed = false;
        if (ImGui::BeginCombo("Track Disappear Animation##TrackSettings",
                              AdoCpp::cstrTrackDisappearAnimation[selected]))
        {
            for (int n = 0; n < IM_ARRAYSIZE(AdoCpp::cstrTrackDisappearAnimation); n++)
            {
                const bool is_selected = selected == n;
                if (ImGui::Selectable(AdoCpp::cstrTrackDisappearAnimation[n], is_selected))
                    selected = n, changed = true;
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        if (changed)
            settings.trackDisappearAnimation = static_cast<AdoCpp::TrackDisappearAnimation>(selected),
            parseUpdateLevel(0);
    }
    if (ImGui::InputDouble("Beats behind##TrackSettings", &settings.beatsBehind, 0, 0, "%g"))
        parseUpdateLevel(0);
}
void LiveCharting::renderSBackground() const
{
    auto& settings = game->level.settings;
    if (ImGuiInputColor("Background color##BackgroundSettings", &settings.backgroundColor))
        parseUpdateLevel(0);
}
void LiveCharting::renderSCamera() const
{
    auto& settings = game->level.settings;
    {
        static int selected;
        selected = static_cast<int>(settings.relativeTo);
        bool changed = false;
        if (ImGui::BeginCombo("Relative To##CameraSettings", AdoCpp::cstrRelativeToCamera[selected]))
        {
            for (int n = 0; n < IM_ARRAYSIZE(AdoCpp::cstrRelativeToCamera); n++)
            {
                const bool is_selected = selected == n;
                if (ImGui::Selectable(AdoCpp::cstrRelativeToCamera[n], is_selected))
                    selected = n, changed = true;
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        if (changed)
            settings.relativeTo = static_cast<AdoCpp::RelativeToCamera>(selected), parseUpdateLevel(0);
    }
    // TODO
}
void LiveCharting::renderSMiscellaneous() const
{
    auto& settings = game->level.settings;
    ImGui::Checkbox("Stick to floors##MiscSettings", &settings.stickToFloors);
}
// ReSharper disable once CppMemberFunctionMayBeStatic
void LiveCharting::renderSDecorations() const
{
    // auto& settings = game->level.settings; // TODO MAYBE I WILL NEVER DO THIS owo
}
