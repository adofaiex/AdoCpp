add_executable(test test.cpp)

target_include_directories(
        test PRIVATE
        ${jsoncpp_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/AdoCpp/include
        ${PROJECT_SOURCE_DIR}/AdoCpp/src
)

add_dependencies (test AdoCpp)
target_link_libraries (
        test PRIVATE
        jsoncpp::jsoncpp
        AdoCpp
)