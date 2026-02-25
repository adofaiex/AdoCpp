#include <AdoCpp.h>
#include <iostream>

const std::filesystem::path PATH = "F:/Levels/adofaigg2191 [18] The Limit Does Not Exist (By  -K & BamgoeSN)"
                      "_1751702585602/The_limit_does_not_exist.adofai";

void test()
{
    const AdoCpp::Level level{PATH};
    const Json::Value doc = level.intoJson();
    Json::StreamWriterBuilder builder;
    std::cout << Json::writeString(builder, doc) << std::endl;
}

int main()
{
    // 1. Import an ADOFAI file to the level.
    const std::filesystem::path PATH = "...";
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
    level.tiles[2].events.push_back(twirl); // Add an event to the tile.

    // 8. Export the level as JSON (needn't parse).
    Json::Value doc = level.intoJson();

    return 0;
}