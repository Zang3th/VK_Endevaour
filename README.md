# VK_Endevaour

## Introduction

**VK_Endevaour** is an educational project focused on learning modern Vulkan (1.4+) by building a small, cleanly structured rendering engine and a sandbox application on top of it.

The primary goals are:

- Learning modern Vulkan usage patterns
- Experimenting with engine architecture
- Maintaining a clean, programming-driven API design

## Key technical goals

- Vulkan 1.4 with modern features
  - No legacy render passes (**Dynamic Rendering**)
  - Explicit synchronization (**Synchronization2**)
  - Usage of **VULKAN_HPP_NO_CONSTRUCTORS** and **VULKAN_HPP_NO_EXCEPTIONS**
    - Explicit create/destroy calls with modern return value types
- C++20
  - Assert-based error handling

## Project structure

```text
VK_Endevaour/
├── Applications/
│   └── Sandbox/                # Test application
├── CMake/                      # Global CMake configuration
├── Docs/                       # Documentation (architecture, usage, roadmap)
├── Engine/
│   ├── Core/                   # Fundamental engine utilities (types, memory, helpers)
│   ├── Debug/                  # Logging and diagnostics
│   ├── Graphics/
│   │   ├── Import/             # CPU-side asset import (e.g., OBJ)
│   │   ├── Resources/          # Graphics resources (e.g., Mesh)
│   │   ├── UI/                 # ImGui integration and UI tooling
│   │   └── Vulkan/             # Vulkan backend
│   ├── Math/                   # Mathematical foundations
│   ├── Platform/               # Platform abstraction (window, input)
│   └── Vendor/                 # Third-party libraries
├── Scripts/                    # Helper scripts (format, build, analyze)
└── Tests/                      # Tests
```

## Building and compiling

### Requirements

- C++20 compiler
  - Tested with Clang 20.x
- CMake 3.25 or newer
- Vulkan 1.4 compatible graphics card and driver
- Vulkan SDK installation (platform dependent)
  - Installation of the SDK, validation layers, glslc, etc. is intentionally not automated, as this strongly depends on the platform and distribution.

For one Fedora-based system, the following packages were sufficient:

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

- No system-wide installation of engine dependencies is required.

A helper script exists to verify basic build requirements:

```bash
python .\Scripts\CheckDependencies.py
```

This script provides a best-effort assessment of whether the system is able to build the project.

### Build process

Manual CMake usage or via the provided build script:

```bash
python .\Scripts\BuildEngine.py -dr    # Debug + Release
```

### Launching

Applications should be launched from the project root directory:

```bash
...\VK_Endevaour> .\Build\Debug\Applications\Sandbox\Sandbox.exe
```

This ensures that all relative resource paths resolve correctly (e.g. shaders,
models, textures, configuration files).

### Integrated libraries

**Thanks to all the creators and contributors of these projects!**

| **Library**                                                              | **Version** | **Updated in Engine** | **Functionality**           | **Type**       |
|:------------------------------------------------------------------------:|:-----------:|:---------------------:|:---------------------------:|:--------------:|
| [vma](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator) | 3.4.0       | 08.06.2026            | Memory allocation           | Source include |
| [imgui](https://github.com/ocornut/imgui)                                | 1.92.8      | 08.06.2026            | GUI                         | Source include |
| [doctest](https://github.com/doctest/doctest)                            | 2.5.0       | 26.05.2026            | Testing                     | Source include |
| [glm](https://github.com/g-truc/glm)                                     | 1.0.3       | 01.01.2026            | Mathematics                 | Source include |
| [fmt](https://github.com/fmtlib/fmt)                                     | 12.1.0      | 29.12.2025            | Formatting and logging      | Static library |
| [glfw](https://github.com/glfw/glfw)                                     | 3.4.0       | 11.12.2025            | Window and input            | Static library |
| [tinyobjloader](https://github.com/tinyobjloader/tinyobjloader)          | 2.0.0       | 24.10.2024            | OBJ file loading            | Source include |
| [stb_image](https://github.com/nothings/stb/blob/master/stb_image.h)     | 2.30        | 21.10.2024            | Image loading               | Source include |

## License

VK_Endevaour is licensed under the [MIT LICENSE](https://github.com/Zang3th/VK_Endevaour/blob/main/LICENSE).
