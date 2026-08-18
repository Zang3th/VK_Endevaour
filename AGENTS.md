# AGENTS.md — VK_Endevaour

## Project Identity

`VK_Endevaour` is a C++20 Vulkan rendering engine prototype with a programming-driven architecture, built as a static library and consumed by applications such as `Sandbox`.

The goal is not a full game engine, but an incrementally developed, clean, explicit, low-level Vulkan architecture suitable for renderer architecture design, real-time rendering experiments, GPU compute workflows, and Vulkan resource ownership experiments.

Favor correctness, explicitness, architectural clarity, and Vulkan validity over convenient abstractions.

---

## Core Technical Stack

Use and assume:

* C++20, no exceptions
* CMake + Ninja, Clang / MSVC
* Vulkan SDK via Vulkan-Hpp, Dynamic Rendering, VMA
* GLFW
* Validation Layers enabled in debug builds
* Static engine library + application executables
* Cross platform development environment
* Python script workflow for build/run/test/format
* Neovim / clangd-compatible codebase

Do not introduce:

* Render Pass or Framebuffer based architecture
* exception-based error handling
* hidden global engine state
* unrelated third-party abstractions
* broad ECS/game-engine abstractions
* unnecessary template metaprogramming

---

## Repository Layout

The directory tree is documented in `README.md` under "Project structure" — treat it as the single source of truth.

Two directories carry rules beyond that layout:

* `Scripts/` is a first-class part of the project pipeline. The Python scripts define build, run, test, formatting, analysis, shader compilation, dependency-check, and automation workflows. Inspect them before making assumptions about any of that behavior, and prefer them over manual CMake/build/run/format commands. Do not bypass, rewrite, or ignore them unless explicitly requested.
* `Vendor/` is third-party code: read-only and out-of-scope. Never analyze, patch, refactor, format, or modify its internals, and never include it in edits. Only inspect it if absolutely necessary to understand public API usage.

---

## Global Response Rules

When answering:

* Be concise, technical, and implementation-aware.
* Prefer architectural reasoning before code.
* Prefer headers, APIs, interfaces, structs, signatures, and ownership diagrams.
* Do not generate implementations unless explicitly requested; if code is requested, keep it minimal and local.
* Do not refactor unrelated code or rename public APIs without a strong architectural reason.
* Do not silently change project conventions.
* Point out Vulkan lifetime, synchronization, ownership, and validation-layer issues aggressively.
* Prefer minimal viable Vulkan infrastructure over speculative engine systems.
* Distinguish clearly between: current architecture, recommended next step, optional future extension, dangerous design smell.

If a request is ambiguous, make the best reasonable assumption and continue. Avoid blocking on clarification unless the ambiguity could cause destructive edits.

---

## Default Editing Policy

Default mode is **analysis only**. Unless the user explicitly says to edit files, do not edit anything.

Allowed by default: inspect project-owned files and `Scripts/`, explain architecture, identify problems, and propose API changes, header-only designs, file layout, Vulkan object ownership, build commands, and validation/debug strategy.

Not allowed unless explicitly requested:

* patch, reformat, rename, or move files
* generate implementation-heavy code
* execute destructive commands
* change CMake targets or `.clang-format`
* add dependencies
* rewrite large subsystems

When file edits are requested:

* keep edits minimal and prefer one focused patch
* show intent before touching multiple files
* preserve existing style
* do not mix formatting-only changes with logic changes

---

## Code Style

Respect the repository `.clang-format` and do not introduce formatting churn. Never reformat the whole repository unless explicitly requested, and keep formatting-only commits separate from logic commits.

General style:

* explicit ownership, clear lifetime boundaries, RAII where appropriate
* avoid hidden side effects, macro-heavy design, excessive abstraction, premature generalization
* prefer simple structs for specifications and aggregate initialization where practical
* prefer strongly typed IDs / handles where useful

```cpp
// Preferred
struct PipelineSpecification
{
    // explicit fields
};

// Avoid
class OverAbstractedBuilderFactoryManager;
```

Error handling:

* No exceptions. Assertions for fatal programmer/configuration errors.
* Check runtime Vulkan errors explicitly; never silently ignore `vk::Result`.
* Keep failure modes visible.

---

## Build and Launch Assumptions

Launch from the repository root — resource paths, shader paths, and debugger working directories depend on the current working directory. Do not assume executable-relative resource loading unless explicitly implemented.

```powershell
cd C:\Dev\VK_Endevaour
.\Build\Debug\Applications\Sandbox\Sandbox.exe
```
