include(FetchContent)
message(STATUS "Getting JsonCpp...")
if (USE_MIRROR)
    FetchContent_Declare(jsoncpp
            GIT_REPOSITORY https://gh.xmly.dev/github.com/open-source-parsers/jsoncpp
            GIT_SHALLOW ON
            GIT_TAG 1.9.6
            SYSTEM
    )
else ()
    FetchContent_Declare(jsoncpp
            GIT_REPOSITORY https://github.com/open-source-parsers/jsoncpp
            GIT_SHALLOW ON
            GIT_TAG 1.9.6
            SYSTEM
    )
endif ()

FetchContent_GetProperties(jsoncpp)
if(NOT jsoncpp_POPULATED)
  set(JSONCPP_WITH_TESTS
      OFF
      CACHE BOOL "")
  set(JSONCPP_WITH_POST_BUILD_UNITTEST
      OFF
      CACHE BOOL "")

  set(BUILD_OBJECT_LIBS
      OFF
      CACHE BOOL "")

  if(BUILD_SHARED_LIBS)
    set(BUILD_SHARED_LIBS
        ON
        CACHE BOOL "")
    set(BUILD_STATIC_LIBS
        OFF
        CACHE BOOL "")
  else()
    set(BUILD_SHARED_LIBS
        OFF
        CACHE BOOL "")
    set(BUILD_STATIC_LIBS
        ON
        CACHE BOOL "")
  endif()

  FetchContent_MakeAvailable(jsoncpp)

  if(TARGET jsoncpp_static)
    add_library(jsoncpp::jsoncpp ALIAS jsoncpp_static)
  elseif(TARGET jsoncpp_lib)
    add_library(jsoncpp::jsoncpp ALIAS jsoncpp_lib)
  endif()
endif()