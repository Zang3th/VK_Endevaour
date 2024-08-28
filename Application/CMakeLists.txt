cmake_minimum_required(VERSION 3.25)

project(
    Application
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
add_executable(Application ${APP_SOURCES})

# Link application against the engine
target_link_libraries(Application PRIVATE Engine)
