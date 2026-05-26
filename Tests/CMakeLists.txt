cmake_minimum_required(VERSION 3.25)

project(EngineTests LANGUAGES CXX)

# Get all source files
file(
    GLOB_RECURSE
    APP_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

# Compile to an executable
add_executable(EngineTests ${APP_SOURCES})

# Link application against the engine
target_link_libraries(EngineTests PRIVATE Engine)

add_test(NAME EngineTests COMMAND EngineTests)
