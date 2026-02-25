include(FetchContent)
message(STATUS "Getting RapidJSON...")
set(RAPIDJSON_BUILD_EXAMPLES OFF)
set(RAPIDJSON_BUILD_TESTS OFF)
set(RAPIDJSON_HAS_STDSTRING ON)
if (USE_MIRROR)
    FetchContent_Declare(RapidJSON
            GIT_REPOSITORY https://gh.xmly.dev/github.com/Tencent/rapidjson.git
            GIT_SHALLOW ON
            SYSTEM
            #        FIND_PACKAGE_ARGS NAMES RapidJSON
    )
else ()
    FetchContent_Declare(RapidJSON
            GIT_REPOSITORY https://github.com/Tencent/rapidjson.git
            GIT_SHALLOW ON
            SYSTEM
            #        FIND_PACKAGE_ARGS NAMES RapidJSON
    )
endif ()
FetchContent_MakeAvailable(RapidJSON)
add_library(rapidjson INTERFACE)
add_library(rapidjson::rapidjson ALIAS rapidjson)
target_include_directories(rapidjson INTERFACE ${RapidJSON_SOURCE_DIR}/include)
