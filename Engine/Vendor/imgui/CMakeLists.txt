cmake_minimum_required(VERSION 3.25)

project(imgui LANGUAGES CXX)

# Compile to a static library
add_library(
    imgui STATIC
    "imgui.cpp"
    "imgui_demo.cpp"
    "imgui_draw.cpp"
    "imgui_tables.cpp"
    "imgui_widgets.cpp"
    "imgui_impl_glfw.cpp"
    "imgui_impl_vulkan.cpp"
)

# Check for Vulkan
find_package(Vulkan REQUIRED)

# The backends include GLFW and Vulkan themselves
target_compile_definitions(imgui PRIVATE GLFW_INCLUDE_NONE)
target_link_libraries(imgui PRIVATE glfw Vulkan::Vulkan)
