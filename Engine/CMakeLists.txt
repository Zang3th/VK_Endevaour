cmake_minimum_required(VERSION 3.25)

project(
    Engine
    LANGUAGES CXX
)

# Add source files
file(
    GLOB_RECURSE
    ENGINE_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

# Compile to a static library
add_library(
    Engine
    STATIC
    ${ENGINE_SOURCES}
)

# Add external libraries
add_subdirectory("Vendor/fmt")
target_link_libraries(
    Engine
    fmt::fmt
)

# TODO: Check for Vulkan

# TODO: Check for GLFW

# TODO: Link against libraries
