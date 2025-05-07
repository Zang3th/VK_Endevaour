# Vulkan Endevaour

Accommodating repository while learning Vulkan. This includes:

- Some very barebones engine code
- A sandbox application to test engine features

## Building and compiling

### What you need

- C++20 compiler
- CMake 3.25 or above
- Vulkan 1.4 compatible graphics card, driver and an installation of the SDK
- An installation of all external dependencies

### Notes

- This project is currently only tested on Linux with Wayland
- Please note the path from which you start up the application, because resource loading is relative for now

### External dependencies

- [glfw](https://github.com/glfw/glfw)
- [glslc](https://github.com/google/shaderc?tab=readme-ov-file#downloads)

### Packages (only used for development)

    sudo dnf install vulkan-tools
    sudo dnf install vulkan-loader-devel
    sudo dnf install vulkan-validation-layers-devel

### Integrated libraries

**Thanks to all the creators and contributors of these projects!**

| **Library**                                                          | **Version** | **Updated in Engine** | **Functionality**           | **Type**       |
|:--------------------------------------------------------------------:|:-----------:|:---------------------:|:---------------------------:|:--------------:|
| [imgui](https://github.com/ocornut/imgui)                            | 1.91.9b     | 07.05.2025            | GUI                         | Source include |
| [fmt](https://github.com/fmtlib/fmt)                                 | 11.1.4      | 23.04.2025            | Formatting and logging      | Static library |
| [tinyobjloader](https://github.com/tinyobjloader/tinyobjloader)      | 2.0.0       | 24.10.2024            | OBJ file loading            | Source include |
| [stb_image](https://github.com/nothings/stb/blob/master/stb_image.h) | 2.30        | 21.10.2024            | Image loading               | Source include |
| [glm](https://github.com/g-truc/glm)                                 | 1.0.1       | 29.08.2024            | Mathematics                 | Source include |
