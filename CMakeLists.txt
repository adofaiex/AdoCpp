cmake_minimum_required(VERSION 3.31)
project(AdoCpp)

set(CMAKE_CXX_STANDARD 20)

option(USE_MIRROR "Use mirror to git clone faster" ON)

include(${PROJECT_SOURCE_DIR}/GetRapidJSON.cmake)

add_subdirectory(AdoCpp)
add_subdirectory(AdoCppGame)
add_subdirectory(test)
