cmake_minimum_required(VERSION 3.25)

project(Engine LANGUAGES CXX)

# Add source files
file(
    GLOB_RECURSE
    ENGINE_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

# Compile to a static library
add_library(Engine STATIC ${ENGINE_SOURCES})

# Configure fmt
set(FMT_TEST OFF CACHE BOOL "" FORCE)
set(FMT_DOC OFF CACHE BOOL "" FORCE)
set(FMT_INSTALL OFF CACHE BOOL "" FORCE)
set(FMT_FUZZ OFF CACHE BOOL "" FORCE)
set(FMT_CUDA_TEST OFF CACHE BOOL "" FORCE)
set(FMT_MODULE OFF CACHE BOOL "" FORCE)

# Add fmt
add_subdirectory("Vendor/fmt" EXCLUDE_FROM_ALL)

# Configure glfw
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(GLFW_LIBRARY_TYPE STATIC CACHE STRING "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

# Add glfw
add_subdirectory("Vendor/glfw" EXCLUDE_FROM_ALL)

# Check for Vulkan
find_package(Vulkan REQUIRED)
message(STATUS "Vulkan SDK version: ${Vulkan_VERSION}")

# Link against everything
target_link_libraries(Engine PRIVATE fmt::fmt glfw Vulkan::Vulkan)

# Add linker flags
target_link_libraries(Engine PRIVATE ${ENGINE_LINKER_FLAGS})
