cmake_minimum_required(VERSION 3.25)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(
    VK_Endevaour
    VERSION 0.0.1
    LANGUAGES CXX
)

include("CMake/AddCompileOptions.cmake")

# add_subdirectory(Engine)
add_subdirectory(Application)
