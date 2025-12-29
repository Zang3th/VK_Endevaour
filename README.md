# Vulkan Endevaour

Accommodating repository while learning Vulkan. This includes:

- Some very barebones engine code
- A sandbox application to test engine features

## Building and compiling

### What you need

- Modern version of Clang
  - It is technically possible to compile the project via GCC with relative minimal effort
  - I just didn't bother integrating compiler specific pragmas for now
- CMake 3.25 or above
- Vulkan 1.4 compatible graphics card, driver and an installation of the SDK
- An installation of all external dependencies

### Notes

- This project is currently only tested on Linux (Wayland) with Clang 20.x
- Please note the path from which you start up the application, because resource loading is relative for now

### Packages (only used for development)

    sudo dnf install vulkan-tools
    sudo dnf install vulkan-loader-devel
    sudo dnf install vulkan-validation-layers-devel

### Integrated libraries

**Thanks to all the creators and contributors of these projects!**

| **Library**                                                              | **Version** | **Updated in Engine** | **Functionality**           | **Type**       |
|:------------------------------------------------------------------------:|:-----------:|:---------------------:|:---------------------------:|:--------------:|
| [glm](https://github.com/g-truc/glm)                                     | 1.0.2       | 29.12.2025            | Mathematics                 | Source include |
| [imgui](https://github.com/ocornut/imgui)                                | 1.92.5      | 29.12.2025            | GUI                         | Source include |
| [vma](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator) | 3.3.0       | 29.12.2025            | Memory allocation           | Source include |
| [fmt](https://github.com/fmtlib/fmt)                                     | 12.1.0      | 29.12.2025            | Formatting and logging      | Static library |
| [glfw](https://github.com/glfw/glfw)                                     | 3.4.0       | 11.12.2025            | Window and input            | Static library |
| [tinyobjloader](https://github.com/tinyobjloader/tinyobjloader)          | 2.0.0       | 24.10.2024            | OBJ file loading            | Source include |
| [stb_image](https://github.com/nothings/stb/blob/master/stb_image.h)     | 2.30        | 21.10.2024            | Image loading               | Source include |
