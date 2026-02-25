# AdoCpp

## AdoCpp Library

### Introduction

AdoCpp is an ADOFAI level parser and generator for C++.

AdoCpp uses following software as its dependencies:

- CMake as a general build tool
- JsonCpp & Json5Cpp to parse JSON files
- (optional) Doxygen to build documentation

### Usage at a glance

```c++
#include <iostream>
#include <AdoCpp.h>

int main()
{
    // 1. Import an ADOFAI file to the level.
    constexpr const char* PATH = "...";
    AdoCpp::Level level{PATH};
    // or "AdoCpp::Level level; level.fromFile(PATH);"
    
    // 2. Get some information of the level.
    std::cout << level.settings.artist << " - "
              << level.settings.song << std::endl;
    
    // 3. Parse the level.
    level.parse();
    
    // 4. Get some information of the tiles.
    for (const auto& tile : level.tiles)
        std::cout << tile.seconds << std::endl;
        
    // 5. Update the level.
    level.update(10); // the 10th second
    
    // 6. Get more information of the tiles.
    for (const auto& tile : level.tiles)
    {
        // ".o" means "original value".
        // Actually, you can get the original value
        //     before updating the level.
        auto [originalX, originalY] = tile.pos.o;
        // ".c" means "current value".
        auto [currentX, currentY] = tile.pos.c;
        printf("(%.2f, %.2f) (%.2f, %.2f)\n",
               originalX, originalY,
               currentX,  currentY);
    }
    
    // 7. Modify the level.
    level.tiles[2].angle = AdoCpp::degrees(114.514); // Change the angle of the tile.
    const auto twirl = std::make_shared<AdoCpp::Event::GamePlay::Twirl>();
    twirl->floor = 2;
    level.tiles[2].events.push_back(twirl); // Add an event to the tile.

    // 8. Export the level as JSON (needn't parse).
    Json::Value doc = level.intoJson();
    
    return 0;
}
```

Notice that AdoCpp is still under development
and it is a little buggy.

---

## AdoCppGame

### Introduction

AdoCppGame is an ADOFAI level editor (cannot edit though) and player.

AdoCppGame uses following software and libraries as its dependencies:

- CMake as a general build tool
- AdoCpp to parse ADOFAI files
- SFML as a game engine
- ImGui as a GUI library
- ImGui-SFML to use ImGui with SFML
- ImGuiFileDialog to select files
- ImPlot to show audio oscillogram
- Exprtk as a mathematical expression toolkit library

Notice that AdoCppGame is still under development
and it is a little buggy.

### Usage

When you open AdoCppGame, the game will be in charting state.

Keybind for changing the state:

```text
Charting --[`]-> LiveCharting
LiveCharting --[Esc]-> Charting
Charting --[Space]-> Playing
Playing --[Esc]--> Charting
```

`F11` is for toggling fullscreen.  
`F12` is for toggling autoplay.
