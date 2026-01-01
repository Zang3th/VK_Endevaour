# VK_Endevaour

## Introduction

**VK_Endevaour** is an educational project focused on learning modern Vulkan (1.4+) by building a small, cleanly structured rendering engine and a sandbox application on top of it.

The primary goals are:

- Learning modern Vulkan usage patterns
- Experimenting with engine architecture
- Maintaining a clean, programming-driven API design

## Key technical goals

- Vulkan 1.4 with promoted modern features:
  - No legacy render passes (**Dynamic Rendering**)
  - Explicit synchronization (**Synchronization2**)
  - Extensive use of **dynamic pipeline state**
- Clean ownership and lifetime management
- Modern C++20 (no exceptions, assert-based error handling)
- Engine-driven command buffer recording
- Minimal application-side Vulkan exposure

## Project structure

```text
VK_Endevaour/
├── Applications/
│   └── Sandbox/                # Test application
├── CMake/                      # Global CMake configuration
├── Engine/
│   ├── Core/                   # Core utilities and engine base
│   ├── Debug/                  # Logging, validation, diagnostics
│   ├── Graphics/
│   │   └── Vulkan/             # Vulkan backend
│   └── Vendor/                 # Vendored third-party libraries
├── Scripts/                    # Helper scripts (build, checks, stats)
```

## Building and compiling

### Requirements

- C++20 compiler
  - Tested with Clang 20.x
- CMake 3.25 or newer
- Vulkan 1.4 compatible graphics card and driver
- Vulkan SDK installation (platform dependent)
  - Installation of the Vulkan SDK, validation layers, glslc, etc. is intentionally not automated, as this strongly depends on the platform and distribution.

For one of my fedora machines these installs were sufficient:

```bash
sudo dnf install vulkan-tools
sudo dnf install vulkan-loader-devel
sudo dnf install vulkan-validation-layers-devel
```

### Platform support

- Linux (Wayland)
- Windows
- Regularly tested on both platforms

### Dependency handling

All third-party libraries are fully vendored, either header-only or built as static libraries via sub-CMake.

No system-wide installation of engine dependencies is required.

A helper script exists to verify basic build requirements:

```bash
python Scripts/CheckDependencies.py
```

This script provides a best-effort assessment of whether the system is able to build the project.

### Build process

Recommended workflow via the provided build script:

```bash
python Scripts/BuildEngine.py -d    # Debug
python Scripts/BuildEngine.py -r    # Release
python Scripts/BuildEngine.py -c    # Clean
```
Manual CMake usage is also possible.

### Notes

- Resource paths are currently relative to the working directory
- Validation layers are expected to be available during development
- APIs may change frequently while the architecture is being refined

### Integrated libraries

**Thanks to all the creators and contributors of these projects!**

| **Library**                                                              | **Version** | **Updated in Engine** | **Functionality**           | **Type**       |
|:------------------------------------------------------------------------:|:-----------:|:---------------------:|:---------------------------:|:--------------:|
| [glm](https://github.com/g-truc/glm)                                     | 1.0.3       | 01.01.2026            | Mathematics                 | Source include |
| [imgui](https://github.com/ocornut/imgui)                                | 1.92.5      | 29.12.2025            | GUI                         | Source include |
| [vma](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator) | 3.3.0       | 29.12.2025            | Memory allocation           | Source include |
| [fmt](https://github.com/fmtlib/fmt)                                     | 12.1.0      | 29.12.2025            | Formatting and logging      | Static library |
| [glfw](https://github.com/glfw/glfw)                                     | 3.4.0       | 11.12.2025            | Window and input            | Static library |
| [tinyobjloader](https://github.com/tinyobjloader/tinyobjloader)          | 2.0.0       | 24.10.2024            | OBJ file loading            | Source include |
| [stb_image](https://github.com/nothings/stb/blob/master/stb_image.h)     | 2.30        | 21.10.2024            | Image loading               | Source include |

## License

VK_Endevaour is licensed under the [MIT LICENSE](https://github.com/Zang3th/VK_Endevaour/blob/main/LICENSE).
