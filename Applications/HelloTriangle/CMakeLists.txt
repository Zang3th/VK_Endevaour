cmake_minimum_required(VERSION 3.25)

project(
    HelloTriangle
    LANGUAGES CXX
)

# Add source files
file(
    GLOB_RECURSE
    APP_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

# Compile to an executable
add_executable(HelloTriangle ${APP_SOURCES})

# Link application against the engine
target_link_libraries(HelloTriangle PRIVATE Engine)

# Compile shaders
add_custom_command(
    TARGET HelloTriangle
    POST_BUILD
    COMMAND glslc "${CMAKE_CURRENT_SOURCE_DIR}/Shaders/Frag.glsl" -o "${CMAKE_CURRENT_SOURCE_DIR}/Shaders/Frag.spv"
    COMMAND glslc "${CMAKE_CURRENT_SOURCE_DIR}/Shaders/Vert.glsl" -o "${CMAKE_CURRENT_SOURCE_DIR}/Shaders/Vert.spv"
)

message(STATUS "Compiled shaders: HelloTriangle")
