cmake_minimum_required(VERSION 3.20)

project(design_pattern
    VERSION 0.1.0
    DESCRIPTION "C++ design patterns learning project"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(DESIGN_PATTERN_BUILD_TESTS "Build GoogleTest unit tests" ON)

file(GLOB_RECURSE DESIGN_PATTERN_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library(design_pattern ${DESIGN_PATTERN_SOURCES})
add_library(design_pattern::design_pattern ALIAS design_pattern)

target_include_directories(design_pattern
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_compile_features(design_pattern PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(design_pattern PRIVATE /W4)
else()
    target_compile_options(design_pattern PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(DESIGN_PATTERN_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
