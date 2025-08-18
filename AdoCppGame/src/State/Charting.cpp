#include "Charting.h"
#include "LiveCharting.h"
#include "Playing.h"

#include <AudioProcessing.h>
#include <ImGuiFileDialog.h>
#include <exprtk.hpp>
#include <future>
#include <imgui-SFML.h>
#include <imgui.h>
#include <iostream>
#include <misc/cpp/imgui_stdlib.h>

#include <implot.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>

constexpr ImGuiWindowFlags ADOCPPGAME_FLAGS =
    ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;

static std::map<std::string, char*> buffers;
static bool ImGuiInputFilename(const char* text, const char* hint, std::string* pathPtr)
{
    if (ImGui::Button(" " ICON_FA_FOLDER " "))
    {
        IGFD::FileDialogConfig config;
        config.path = ".";
        config.flags = ImGuiFileDialogFlags_Modal;
        ImGuiFileDialog::Instance()->OpenDialog("ImGuiInputFilename", "Choose an file", ".adofai", config);
    }
    ImGui::SameLine();
    bool val = ImGui::InputTextWithHint(text, hint, pathPtr, ImGuiInputTextFlags_ElideLeft);
    if (ImGuiFileDialog::Instance()->Display("ImGuiInputFilename"))
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

static void ImGuiInputDouble(const char* text, double* doublePtr)
{
    if (!buffers.contains(text))
        buffers[text] = new char[1145]{}, sprintf_s(buffers[text], 1145, "%g", *doublePtr);

    ImGui::InputText(text, buffers[text], 1145, ImGuiInputTextFlags_CharsDecimal);
    if (ImGui::IsItemDeactivatedAfterEdit())
    {
        exprtk::expression<double> expression;
        exprtk::parser<double> parser;
        parser.compile(buffers[text], expression);
        *doublePtr = expression.value();
        sprintf_s(buffers[text], 1145, "%g", *doublePtr);
    }
    if (!ImGui::IsItemActive())
        sprintf_s(buffers[text], 1145, "%g", *doublePtr);
}


static void ImGuiInputFloat(const char* text, float* floatPtr)
{
    if (!buffers.contains(text))
        buffers[text] = new char[1145]{}, sprintf_s(buffers[text], 1145, "%g", *floatPtr);

    ImGui::InputText(text, buffers[text], 1145, ImGuiInputTextFlags_CharsDecimal);
    if (ImGui::IsItemDeactivatedAfterEdit())
    {
        exprtk::expression<float> expression;
        exprtk::parser<float> parser;
        parser.compile(buffers[text], expression);
        *floatPtr = expression.value();
        sprintf_s(buffers[text], 1145, "%g", *floatPtr);
    }
    if (!ImGui::IsItemActive())
        sprintf_s(buffers[text], 1145, "%g", *floatPtr);
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


StateCharting StateCharting::m_stateCharting;

void StateCharting::init(Game* l_game)
{
    game = l_game;
    newLevel();
    game->tileSystem.setTilePlaceMode(0);
}

void StateCharting::cleanup() {}

void StateCharting::pause() {}

void StateCharting::resume()
{
    game->tileSystem.setTilePlaceMode(0);
    game->activeTileIndex = std::nullopt;
    game->level.update();
    game->tileSystem.update();
    game->view.setRotation(sf::degrees(0));
}

void StateCharting::handleEvent(const sf::Event event)
{
    using enum sf::Keyboard::Key;
    if (!ImGui::GetIO().WantCaptureKeyboard)
    {
        if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>())
        {
            if (keyPressed->code == F12)
                game->autoplay = !game->autoplay;
            else if (keyPressed->code == Space)
                game->level.initCamera(), game->pushState(StatePlaying::instance());
            else if (keyPressed->code == Grave)
                game->pushState(LiveCharting::instance());
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

void StateCharting::update()
{
    const auto w = static_cast<float>(game->windowSize.x), h = static_cast<float>(game->windowSize.y);
    game->view.setSize({w / (w + h) * 16 * game->zoom.x, -h / (w + h) * 16 * game->zoom.y});
    game->tileSystem.setActiveTileIndex(game->activeTileIndex);
    game->tileSystem.update();

    if (!sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
        dragging = false;
}

void StateCharting::render()
{
    // render the world
    game->window.setView(game->view);
    game->window.draw(game->tileSystem);

    // render the GUI
    sf::View defaultView = game->window.getDefaultView();
    defaultView.setSize(sf::Vector2f(game->windowSize));
    defaultView.setCenter(sf::Vector2f(game->windowSize) / 2.f);
    game->window.setView(defaultView);

#ifndef NDEBUG
    ImGui::ShowDemoWindow();
    ImPlot::ShowDemoWindow();
#endif // NDEBUG

    renderFilenameBar();
    renderLevelSettings();
    renderEventSettings();
    renderControlPad();
}
void StateCharting::renderFilenameBar()
{
    const float barWidth = ImGui::GetFontSize() * 15;
    ImGui::SetNextWindowSize(ImVec2(barWidth, 0));
    ImGui::SetNextWindowPos(ImVec2(static_cast<float>(game->windowSize.x) / 2.f, 0), 0, ImVec2(0.5, 0));
    if (ImGui::Begin("FilenameBar", nullptr, ADOCPPGAME_FLAGS))
    {
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        static std::future<std::filesystem::path> future;
        static float progress;
        static std::optional<std::exception> exception;
        std::string filename = game->levelPath.filename().string();
        if (filename.empty())
            filename = "Untitled";
        if (ImGui::TreeNodeEx("FilenameBarTreeNode", ImGuiTreeNodeFlags_SpanAvailWidth, filename.c_str()))
        {
            if (ImGui::Button("Open ...", ImVec2(-1, 0)))
            {
                IGFD::FileDialogConfig config;
                config.path = ".";
                config.flags = ImGuiFileDialogFlags_Modal;
                ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose an ADOFAI file", ".adofai", config);
            }
            if (ImGui::Button("Save as ...", ImVec2(-1, 0)))
            {
                IGFD::FileDialogConfig config;
                config.path = game->origMusicPath.parent_path().string();
                config.flags = ImGuiFileDialogFlags_Modal;
                ImGuiFileDialog::Instance()->OpenDialog("SaveFileDlgKey", "Save the ADOFAI file", ".adofai", config);
            }
            if (ImGui::Button(addedHitsound ? "Re-add hitsound" : "Add hitsound", ImVec2(-1, 0)))
            {
                future = std::async(std::launch::async, addHitsound, game->origMusicPath, game->level.tiles, &progress);
                ImGui::OpenPopup("Adding hitsound...");
            }
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            if (ImGui::BeginPopupModal("Adding hitsound...", nullptr))
            {
                if (game->musicPath.empty())
                {
                    ImGui::Text("AdoCpp cannot add hitsound\n because the music file name is empty.");
                    if (ImGui::Button("Close"))
                        ImGui::CloseCurrentPopup();
                }
                else
                {
                    if (!exception)
                    {
                        if (progress == -1)
                            ImGui::Text("Loading...");
                        else if (progress == 2)
                            ImGui::Text("Saving...");
                        else
                            ImGui::Text("Adding hitsound...");
                        ImGui::ProgressBar(std::max(0.f, std::min(1.f, progress)), ImVec2(-1, 0));
                    }
                    else
                    {
                        ImGui::Text("An error occurred. %s", exception->what());
                        if (ImGui::Button("Close"))
                            ImGui::CloseCurrentPopup();
                    }

                    if (future.valid() && future.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready)
                    {
                        try
                        {
                            game->musicPath = future.get();
                            addedHitsound = true;
                            if (!game->musicPath.empty() && !game->music.openFromFile(game->musicPath))
                                throw std::runtime_error("Failed to load music");
                            ImGui::CloseCurrentPopup();
                        }
                        catch (const std::exception& ex)
                        {
                            exception = ex;
                        }
                    }
                }
                ImGui::EndPopup();
            }
            ImGui::TreePop();
        }
        if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                game->levelPath = ImGuiFileDialog::Instance()->GetFilePathName();
                try
                {
                    game->level.fromFile(game->levelPath);
                }
                catch (const AdoCpp::LevelJsonException&)
                {
                    game->levelPath.clear();
                    game->level.defaultLevel();
                    ImGui::OpenPopup("Error!##AdoCpp::LevelJsonHasParseErrorException");
                }
                newLevel();
            }
            ImGuiFileDialog::Instance()->Close();
        }
        if (ImGuiFileDialog::Instance()->Display("SaveFileDlgKey"))
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                const auto path = ImGuiFileDialog::Instance()->GetFilePathName();
                std::ofstream ofs(path, std::ios::binary);
                std::unique_ptr<rapidjson::Document> doc = game->level.intoJson();
                rapidjson::OStreamWrapper osw(ofs);
                rapidjson::EncodedOutputStream<rapidjson::UTF8<>, rapidjson::OStreamWrapper> eos(osw, true);
                rapidjson::PrettyWriter writer(eos);
                doc->Accept(writer);
            }
            ImGuiFileDialog::Instance()->Close();
        }
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        if (ImGui::BeginPopupModal("Error!##AdoCpp::LevelJsonHasParseErrorException", nullptr,
                                   ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Error when parsing json.");
            if (ImGui::Button("Close"))
                ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }
    }
    ImGui::End();
}
void StateCharting::renderLevelSettings() const
{
    const float width = ImGui::GetFontSize() * 15, height = ImGui::GetFontSize() * 30;
    ImGui::SetNextWindowSize(ImVec2(width, height));
    ImGui::SetNextWindowPos(ImVec2(0, static_cast<float>(game->windowSize.y) / 2.f), 0, ImVec2(0, 0.5));
    if (ImGui::Begin("Settings", nullptr, ADOCPPGAME_FLAGS))
    {
        const float settingsTabContentWidth = width / 5 * 4;
        static size_t selectedTab = 0;
        static const std::array<std::string, 7> titles = {
            "Song Settings",   "Level Settings",         "Track Settings", "Background Settings",
            "Camera Settings", "Miscellaneous Settings", "Decorations"};
        static constexpr std::array funcs = {&StateCharting::renderSSong,       &StateCharting::renderSLevel,
                                             &StateCharting::renderSTrack,      &StateCharting::renderSBackground,
                                             &StateCharting::renderSCamera,     &StateCharting::renderSMiscellaneous,
                                             &StateCharting::renderSDecorations};
        if (ImGui::BeginChild("Settings/TabContent", ImVec2(settingsTabContentWidth, 0)))
        {
            ImGui::SetCursorPosX(settingsTabContentWidth / 2 - ImGui::CalcTextSize(titles[selectedTab].c_str()).x / 2);
            ImGui::Text(titles[selectedTab].c_str());
            (this->*funcs[selectedTab])();
        }
        ImGui::EndChild();
        ImGui::SameLine();
        if (ImGui::BeginChild("Settings/TabBtns"))
        {
            if (ImGui::BeginTable("Settings/TabBtns/Table", 1))
            {
                for (size_t i = 0; i < 7; i++)
                {
                    ImGui::TableNextRow(), ImGui::TableNextColumn();
                    ImGui::PushID(("Settings/TabBtns/Table/TabBtn" + std::to_string(i)).c_str());
                    if (ImGui::Button(titles[i].c_str(), ImVec2(-1, 0)))
                        selectedTab = i;
                    ImGui::PopID();
                }
                ImGui::EndTable();
            }
        }
        ImGui::EndChild();
    }
    ImGui::End();
}
void StateCharting::renderEventSettings() const
{
    const bool windowOpen = game->activeTileIndex.has_value();
    const float width = ImGui::GetFontSize() * 15, height = ImGui::GetFontSize() * 30;
    if (windowOpen)
    {
        const auto& tile = game->level.tiles[*game->activeTileIndex];
        ImGui::SetNextWindowSize(ImVec2(width, height));
        ImGui::SetNextWindowPos(
            ImVec2(static_cast<float>(game->windowSize.x), static_cast<float>(game->windowSize.y) / 2.f), 0,
            ImVec2(1, 0.5));
        if (ImGui::Begin("EventSettings", nullptr, ADOCPPGAME_FLAGS))
        {
            const float tabButtonsWidth = width / 5, rightSettingsTabContentWidth = width - tabButtonsWidth;
            static std::optional<size_t> tileIndex;
            static size_t selectedTab = 0;
            if (game->activeTileIndex != tileIndex)
                tileIndex = game->activeTileIndex, selectedTab = 0;
            if (ImGui::BeginChild("EventSettings/TabBtns", ImVec2(tabButtonsWidth, 0)))
            {
                if (ImGui::BeginTable("EventSettings/TabBtns/Table", 1))
                {
                    for (size_t i = 0; i < tile.events.size(); i++)
                    {
                        ImGui::TableNextRow(), ImGui::TableNextColumn();
                        ImGui::PushID(("EventSettings/TabBtns/Table/TabBtn" + std::to_string(i)).c_str());
                        if (ImGui::Button(tile.events[i]->name(), ImVec2(-1, 0)))
                            selectedTab = i;
                        ImGui::PopID();
                    }
                    ImGui::EndTable();
                }
            }
            ImGui::EndChild();
            ImGui::SameLine();
            if (ImGui::BeginChild("EventSettings/TabContent"))
            {
                if (!tile.events.empty())
                {
                    const char* title = tile.events[selectedTab]->name();
                    ImGui::SetCursorPosX(rightSettingsTabContentWidth / 2 - ImGui::CalcTextSize(title).x / 2);
                    ImGui::Text(title);
                    auto event = tile.events[selectedTab];
                    if (const auto twirl = std::dynamic_pointer_cast<AdoCpp::Event::GamePlay::Twirl>(event))
                    {
                    }
                    if (const auto setSpeed = std::dynamic_pointer_cast<AdoCpp::Event::GamePlay::SetSpeed>(event))
                    {
                    }
                }
            }
            ImGui::EndChild();
        }
        ImGui::End();
    }
}
void StateCharting::renderControlPad() const
{
    if (ImGui::Begin("ControlPad"))
    {
        if (ImGui::TreeNode("Select Difficulty"))
        {
            using AdoCpp::Difficulty;

            if (ImGui::Selectable("Lenient", game->config.difficulty == Difficulty::Lenient))
                game->config.difficulty = Difficulty::Lenient;
            if (ImGui::Selectable("Normal", game->config.difficulty == Difficulty::Normal))
                game->config.difficulty = Difficulty::Normal;
            if (ImGui::Selectable("Strict", game->config.difficulty == Difficulty::Strict))
                game->config.difficulty = Difficulty::Strict;

            ImGui::TreePop();
        }
        ImGui::InputFloat("Input Offset", &game->config.inputOffset, 1, 10, "%0.f");
        {
            constexpr uint32_t step = 1, stepFast = 10;
            ImGui::InputScalar("FPS Limit", ImGuiDataType_U32, &game->config.fpsLimit, &step, &stepFast);
            if (ImGui::IsItemDeactivatedAfterEdit())
                game->window.setFramerateLimit(game->config.fpsLimit);
        }
        ImGui::Checkbox("Block Keyboard Chatter", &game->config.blockKeyboardChatter);
        ImGui::Checkbox("Hide Perfects", &game->config.hidePerfects);
        ImGui::Checkbox("Timer Sync With Music", &game->config.syncWithMusic);
        if (ImGui::TreeNode("Performance"))
        {
            if (static bool disableAnimationTrack = game->level.disableAnimateTrack();
                ImGui::Checkbox("Disable AnimationTrack", &disableAnimationTrack))
            {
                game->level.disableAnimateTrack(disableAnimationTrack);
                parseUpdateLevel(0);
            }
            ImGui::TreePop();
        }
        if (static size_t index; ImGui::TreeNode("Keystroke Settings"))
        {
            for (size_t j, i = j = 0; i < game->config.keyLimiter.size(); i++, j++)
            {
                ImGui::PushID(j);
                if (ImGui::Button(" " ICON_FA_TRASH_CAN " ##Keystroke"))
                {
                    game->config.keyLimiter.erase(game->config.keyLimiter.begin() + i);
                    i--;
                }
                ImGui::SameLine();
                std::string desc = sf::Keyboard::getDescription(game->config.keyLimiter[i]).toAnsiString();
                if (ImGui::Button(desc.c_str(), {ImGui::GetFontSize() * 10, 0}))
                {
                    index = i;
                    ImGui::OpenPopup("Change hotkey##aaa");
                }
                if (ImGui::BeginPopupModal("Change hotkey##aaa", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
                {
                    ImGui::Text("Press any key...");
                    for (size_t k = 0; k < sf::Keyboard::ScancodeCount; k++)
                    {
                        const auto scan = static_cast<sf::Keyboard::Scan>(k);
                        if (sf::Keyboard::isKeyPressed(scan))
                        {
                            game->config.keyLimiter[index] = scan;
                            ImGui::CloseCurrentPopup();
                        }
                    }
                    ImGui::EndPopup();
                }
                ImGui::PopID();
            }
            if (ImGui::Button(" " ICON_FA_PLUS " ##KeystrokeAddKey"))
            {
                game->config.keyLimiter.push_back(sf::Keyboard::Scan::A);
            }
            const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            ImGui::TreePop();
        }
    }
    ImGui::End();
}
void StateCharting::renderSSong() const
{
    auto& settings = game->level.settings;
    if (ImGuiInputFilename("Song Filename", "No files selected", &settings.songFilename))
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
void StateCharting::renderSLevel() const
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
void StateCharting::renderSTrack() const
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
void StateCharting::renderSBackground() const
{
    auto& settings = game->level.settings;
    if (ImGuiInputColor("Background color##BackgroundSettings", &settings.backgroundColor))
        parseUpdateLevel(0);
}
void StateCharting::renderSCamera() const
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
void StateCharting::renderSMiscellaneous() const
{
    auto& settings = game->level.settings;
    ImGui::Checkbox("Stick to floors##MiscSettings", &settings.stickToFloors);
}
void StateCharting::renderSDecorations() const
{
    // auto& settings = game->level.settings; // TODO MAYBE I WILL NEVER DO THIS owo
}

void StateCharting::newLevel()
{
    addedHitsound = false;
    game->activeTileIndex = std::nullopt;
    game->level.parse();
    game->level.update();
    game->tileSystem.parse();
    game->tileSystem.update();
    game->origMusicPath = game->musicPath = game->levelPath.parent_path().append(game->level.settings.songFilename);
    if (!game->musicPath.empty())
    {
        if (!game->music.openFromFile(game->musicPath))
        {
            std::cerr << "Warning: Failed to load music from file \"" << game->musicPath
                      << "\". Maybe the file does not exist or it is not a music file.";
        }
    }
    game->view.setCenter({0.f, 0.f});
    game->view.setRotation(sf::degrees(0));
    game->zoom = {1.f, 1.f};
}
void StateCharting::parseUpdateLevel(const size_t floor) const
{
    game->level.parse(floor, false, true);
    game->level.update();
    game->tileSystem.parse();
    game->tileSystem.update();
}
