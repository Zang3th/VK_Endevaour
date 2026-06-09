# AGENTS.md — VK_Endevaour

## Project Identity

This repository contains `VK_Endevaour`, a C++20 Vulkan rendering engine prototype with a programming-driven architecture.

The engine is built as a static library and consumed by applications such as `Sandbox`.

The project focus is not to build a full game engine, but to incrementally develop a clean, explicit, low-level Vulkan architecture suitable for:

* renderer architecture design
* real-time rendering experiments
* GPU compute workflows
* Vulkan resource ownership experiments

Favor correctness, explicitness, architectural clarity, and Vulkan validity over convenient abstractions.

---

## Core Technical Stack

Use and assume:

* C++20
* CMake + Ninja
* Clang / MSVC
* GLFW
* Vulkan SDK using Vulkan-Hpp
* VMA / Vulkan Memory Allocator
* Dynamic Rendering
* Static engine library + application executables
* Validation Layers enabled in debug builds
* Cross platform development environment
* Python script workflow for build/run/test/format
* Neovim / clangd-compatible codebase

Do not introduce:

* Render Pass based architecture
* Framebuffer based architecture
* exception-based error handling
* hidden global engine state
* unrelated third-party abstractions
* broad ECS/game-engine abstractions
* unnecessary template metaprogramming
* large refactors unless explicitly requested

---

## Project-Owned vs. Third-Party Code

`Scripts/` is a first-class part of the project pipeline.

Treat Python scripts as important project infrastructure. They define and support the intended build, run, test, formatting, analysis, shader compilation, dependency-check, and automation workflow.

Before making assumptions about build behavior, launch behavior, formatting behavior, dependency checks, shader compilation, or project automation, inspect the relevant Python scripts.

Do not bypass, rewrite, or ignore Python scripts unless explicitly requested.

`Vendor/` contains third-party code.

Do not analyze, patch, refactor, format, or modify third-party implementation internals under `Vendor/`. Treat it as read-only and out-of-scope. Only inspect it if absolutely necessary to understand public API usage.

---

## Global Response Rules for Codex

When answering:

* Be concise, technical, and implementation-aware.
* Prefer architectural reasoning before code.
* Prefer headers, APIs, interfaces, structs, signatures, and ownership diagrams.
* Do not generate implementations unless explicitly requested.
* If code is requested, keep it minimal and local.
* Do not modify files unless explicitly asked.
* Do not refactor unrelated code.
* Do not rename public APIs unless there is a strong architectural reason.
* Do not add dependencies without explicit justification.
* Do not silently change project conventions.
* Point out Vulkan lifetime, synchronization, ownership, and validation-layer issues aggressively.
* Prefer minimal viable Vulkan infrastructure over speculative engine systems.
* Distinguish clearly between:

  * current architecture
  * recommended next step
  * optional future extension
  * dangerous design smell

If the request is ambiguous, make the best reasonable assumption and continue. Avoid blocking on clarification unless the ambiguity could cause destructive edits.

---

## Default Editing Policy

Default mode:

```text
analysis only
```

Unless the user explicitly says to edit files, do not edit anything.

Allowed by default:

* inspect project-owned files
* inspect Python scripts in `Scripts/`
* explain architecture
* identify problems
* suggest API changes
* propose header-only designs
* propose file layout
* propose Vulkan object ownership
* propose build commands
* propose validation/debug strategy

Not allowed unless explicitly requested:

* patch files
* reformat files
* rename files
* move files
* generate implementation-heavy code
* execute destructive commands
* change CMake targets
* change `.clang-format`
* add dependencies
* rewrite large subsystems

Never allowed:

* patch third-party code
* refactor third-party code
* modify anything under `Vendor/`

When file edits are requested:

* keep edits minimal
* show intent before touching multiple files
* preserve existing style
* prefer one focused patch
* do not mix formatting-only changes with logic changes
* never include `Vendor/` in edits

---

## Code Style

General style:

* C++20
* explicit ownership
* RAII where appropriate
* no exceptions
* assertions for fatal programmer errors
* avoid hidden side effects
* avoid macro-heavy design
* avoid excessive abstraction
* avoid premature generalization
* prefer simple structs for specifications
* prefer strongly typed IDs / handles where useful
* prefer clear lifetime boundaries

Error handling:

* Do not use exceptions.
* Fatal programmer/configuration errors may use assertions.
* Runtime Vulkan errors should be checked explicitly.
* Keep failure modes visible.
* Do not silently ignore `vk::Result`.

Preferred style:

```cpp
struct PipelineSpecification
{
    // explicit fields
};
```

Prefer aggregate initialization where practical.

Avoid:

```cpp
class OverAbstractedBuilderFactoryManager;
```

---

## Formatting Rules

Respect the repository `.clang-format`.

Do not introduce formatting churn.

When suggesting formatting changes:

* separate formatting-only commits from logic commits
* do not mix `.clang-format` changes with code changes
* do not reformat the whole repository unless explicitly requested
* do not format files under `Vendor/`

---

## Build and Launch Assumptions

The project should usually be launched from the repository root because resource paths, shader paths, and debugger working directories depend on the current working directory.

Expected launch style:

```powershell
cd C:\Dev\VK_Endevaour
.\Build\Debug\Applications\Sandbox\Sandbox.exe
```

When suggesting debugger configuration, ensure the working directory is the project root:

```text
C:\Dev\VK_Endevaour
```

Do not assume executable-relative resource loading unless explicitly implemented.

Prefer existing Python scripts for project operations where available. Before suggesting manual CMake/build/run/format commands, check whether a corresponding Python script already exists in `Scripts/`.

---

## Repository Architecture

Expected high-level layout:

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
│   └── Vendor/                 # Third-party libraries; read-only/out-of-scope
├── Scripts/                    # First-class Python pipeline scripts
└── Tests/                      # Tests
```

---
