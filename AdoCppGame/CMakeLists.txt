#include(${PROJECT_SOURCE_DIR}/GetRapidJSON.cmake)
include(${PROJECT_SOURCE_DIR}/GetSFML.cmake)
include(${PROJECT_SOURCE_DIR}/GetImGui.cmake)
include(${PROJECT_SOURCE_DIR}/GetExprtk.cmake)

find_package(SFML REQUIRED COMPONENTS System Window Graphics Audio)

add_executable(
        AdoCppGame

        thirdparty/IconsFontAwesome6.h
        thirdparty/ImGuiFileDialog.h
        thirdparty/ImGuiFileDialog.cpp
        thirdparty/ImGuiFileDialogConfig.h
        thirdparty/implot-master/implot.h
        thirdparty/implot-master/implot.cpp
        thirdparty/implot-master/implot_internal.h
        thirdparty/implot-master/implot_demo.cpp
        thirdparty/implot-master/implot_items.cpp

        src/main.cpp
        src/Game.h src/Game.cpp
        src/State.h
        src/State/Charting.h src/State/Charting.cpp
        src/State/Playing.h src/State/Playing.cpp
        src/State/LiveCharting.h src/State/LiveCharting.cpp
        src/Tile.h src/Tile.cpp
        src/HitErrorMeter.h
        src/HitText.h
        src/KeyViewer.h src/KeyViewer.cpp
        src/AudioProcessing.h
        src/ImGuiConfig.h

        src/resource.rc
        src/Config.cpp
        src/Config.h
        src/CountDownSystem.h
)

set_property(TARGET AdoCppGame PROPERTY CXX_STANDARD 20)

if (CMAKE_COMPILER_IS_GNUCXX)
    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag("-Wa,-mbig-obj" GNU_BIG_OBJ_FLAG_ENABLE)
endif ()
target_compile_options(AdoCppGame
        PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/bigobj>
        $<$<AND:$<CXX_COMPILER_ID:GNU>,$<BOOL:${GNU_BIG_OBJ_FLAG_ENABLE}>>:-Os -Wa,-mbig-obj>
)

# copy assets
add_custom_target(copy_assets
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/assets
)
add_dependencies(AdoCppGame copy_assets)

target_include_directories(
        AdoCppGame PRIVATE
        src
        thirdparty
        thirdparty/implot-master
        ${PROJECT_SOURCE_DIR}/AdoCpp/src
        ${exprtk_SOURCE_DIR}
)

add_dependencies(AdoCppGame AdoCpp)
target_link_libraries(
        AdoCppGame PRIVATE

        rapidjson::rapidjson

        AdoCpp

        SFML::System
        SFML::Window
        SFML::Graphics
        SFML::Audio

        ImGui-SFML::ImGui-SFML
)
target_include_directories(
        AdoCppGame PRIVATE
        ${igfd_SOURCE_DIR}
)

target_compile_definitions(AdoCppGame PRIVATE NOMINMAX)
