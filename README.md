# VK Endevaour

Accommodating repository while learning Vulkan. This includes:

- Some basic engine code
- Learning materials and documentation (some of it may be in German)

## Building and compiling

### What you need

- C++20 compiler
  - The provided build script uses Clang++-17
- CMake 3.25 or above
- Vulkan compatible graphics card, driver and the SDK
- An installation of all external dependencies

This project is currently only tested on Linux with Wayland.

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
| [tinyobjloader](https://github.com/tinyobjloader/tinyobjloader)      | 2.0.0       | 24.10.2024            | OBJ file loading            | Header only    |
| [stb_image](https://github.com/nothings/stb/blob/master/stb_image.h) | 2.30        | 21.10.2024            | Image loading               | Header only    |
| [glm](https://github.com/g-truc/glm)                                 | 1.0.1       | 29.08.2024            | Mathematics                 | Header only    |
| [fmt](https://github.com/fmtlib/fmt)                                 | 11.0.2      | 28.08.2024            | Formatting and logging      | Static library |
