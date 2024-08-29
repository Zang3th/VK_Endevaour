# VK Endevaour

Accommodating repository while learning Vulkan. This includes:

- Some basic engine code
- Learning materials and documentation (some of it may be in German)

## Building and compiling

### What you need

- C++20 compiler
- CMake 3.25 or above
- Vulkan compatible graphics card, driver and the SDK
- An installation of all external dependencies

The project should build cross-platform but is currently only tested on Linux.

### External dependencies

- **GLFW**

        sudo apt install libglfw3-dev

### Packages (only used for development)

        sudo apt install vulkan-tools
        sudo apt install libvulkan-dev
        sudo apt install vulkan-validationlayers-dev spirv-tools

### Integrated libraries

**Thanks to all the creators and contributors of these projects!**

| **Library**                                                          | **Version** | **Updated in Engine** | **Functionality**           | **Type**       |
|:--------------------------------------------------------------------:|:-----------:|:---------------------:|:---------------------------:|:--------------:|
| [GLM](https://github.com/g-truc/glm)                                 | 1.0.1       | 29.08.2024            | Mathematics                 | Header only    |
| [fmt](https://github.com/fmtlib/fmt)                                 | 11.0.2      | 28.08.2024            | Formatting and logging      | Static library |
