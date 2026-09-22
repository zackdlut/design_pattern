# 每个模式一个可执行客户端：examples/<category>/<pattern>/main.cpp
# 目标名 example_<pattern>，输出到 ${CMAKE_BINARY_DIR}/examples/

file(GLOB_RECURSE DESIGN_PATTERN_EXAMPLE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*/main.cpp"
)

if(NOT DESIGN_PATTERN_EXAMPLE_SOURCES)
    message(WARNING "No example sources found under examples/")
    return()
endif()

set(DESIGN_PATTERN_EXAMPLE_TARGETS)

foreach(example_src IN LISTS DESIGN_PATTERN_EXAMPLE_SOURCES)
    get_filename_component(pattern_dir "${example_src}" DIRECTORY)
    get_filename_component(pattern_name "${pattern_dir}" NAME)
    set(example_target "example_${pattern_name}")

    add_executable(${example_target} "${example_src}")
    target_link_libraries(${example_target} PRIVATE design_pattern::design_pattern)
    target_compile_features(${example_target} PRIVATE cxx_std_20)

    if(MSVC)
        target_compile_options(${example_target} PRIVATE /W4)
    else()
        target_compile_options(${example_target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    set_target_properties(${example_target} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/examples"
    )

    list(APPEND DESIGN_PATTERN_EXAMPLE_TARGETS ${example_target})
endforeach()

add_custom_target(examples DEPENDS ${DESIGN_PATTERN_EXAMPLE_TARGETS})
