include(${PROJECT_SOURCE_DIR}/GetJsonCpp.cmake)

add_library(
        AdoCpp STATIC
        src/AdoCpp.h
        src/AdoCpp/Easing.h src/AdoCpp/Easing.cpp
        src/AdoCpp/Event.h src/AdoCpp/Event.cpp
        src/AdoCpp/Level.h src/AdoCpp/Level.cpp
        src/AdoCpp/Utils.h src/AdoCpp/Utils.cpp
        src/AdoCpp/Color.h src/AdoCpp/Color.inl
        src/AdoCpp/Events/GamePlay.h src/AdoCpp/Events/GamePlay.cpp
        src/AdoCpp/Events/Track.h src/AdoCpp/Events/Track.cpp
        src/AdoCpp/Events/Visual.h src/AdoCpp/Events/Visual.cpp
        src/AdoCpp/Events/Modifiers.h src/AdoCpp/Events/Modifiers.cpp
        src/AdoCpp/Events/Dlc.h src/AdoCpp/Events/Dlc.cpp
        src/AdoCpp/Events/Base.cpp
        src/AdoCpp/Events/Base.h
        src/AdoCpp/Tile.h
        src/AdoCpp/Tile.cpp
        src/AdoCpp/DynamicValue.cpp
        src/AdoCpp/DynamicValue.h
        src/AdoCpp/Math/Vector2.h
        src/AdoCpp/Math/Vector2.inl
        src/AdoCpp/Math/Vector2.cpp
        src/AdoCpp/Math/Angle.h
        src/AdoCpp/Math/Angle.inl
        src/AdoCpp/Camera.h
        src/AdoCpp/Camera.cpp

        include/json5cpp.h
)
target_include_directories(AdoCpp PRIVATE
        src/
        include/
)
target_link_libraries(AdoCpp PRIVATE jsoncpp::jsoncpp)