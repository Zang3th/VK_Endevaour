cmake_minimum_required(VERSION 3.25)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(
    VK_Endevaour
    VERSION 0.0.1
    LANGUAGES CXX
)

# Add compile options
include("CMake/AddCompileOptions.cmake")

# Add global include directory
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Engine)

# Add subprojects
add_subdirectory(Engine)
add_subdirectory(Applications)
