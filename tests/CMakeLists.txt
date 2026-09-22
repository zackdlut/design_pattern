include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.15.2
    GIT_SHALLOW    TRUE
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

file(GLOB_RECURSE DESIGN_PATTERN_TEST_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_executable(design_pattern_tests ${DESIGN_PATTERN_TEST_SOURCES})

target_link_libraries(design_pattern_tests
    PRIVATE
        design_pattern::design_pattern
        GTest::gtest_main
)

target_compile_features(design_pattern_tests PRIVATE cxx_std_20)

if(MSVC)
    target_compile_options(design_pattern_tests PRIVATE /W4)
else()
    target_compile_options(design_pattern_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

include(GoogleTest)
gtest_discover_tests(design_pattern_tests)
