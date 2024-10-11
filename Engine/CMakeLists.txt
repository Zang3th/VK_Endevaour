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
add_library(Engine STATIC ${ENGINE_SOURCES})

# Add external libraries
add_subdirectory("Vendor/fmt")

# Check for GLFW
find_package(glfw3 REQUIRED)
message(STATUS "{glfw} version: ${glfw3_VERSION}")

# Check for Vulkan
find_package(Vulkan REQUIRED)
message(STATUS "Vulkan SDK version: ${Vulkan_VERSION}")

# Link against everything
target_link_libraries( Engine
    PRIVATE
        fmt::fmt
    PUBLIC
        glfw
        Vulkan::Vulkan
)

# Add linker flags
set(ENGINE_LINKER_FLAGS "-ldl -lpthread")
target_link_libraries(Engine PRIVATE ${ENGINE_LINKER_FLAGS})
