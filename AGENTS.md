# AGENTS.md — VK_Endevaour

## Project Identity

`VK_Endevaour` is a C++20 Vulkan rendering engine prototype with a programming-driven architecture, built as a static library and consumed by applications such as `Sandbox`.

The goal is not a full game engine, but an incrementally developed, clean, explicit, low-level Vulkan architecture suitable for renderer architecture design, real-time rendering experiments, GPU compute workflows, and Vulkan resource ownership experiments.

Favor correctness, explicitness, architectural clarity, and Vulkan validity over convenient abstractions.

---

## Working Role

Act as an advisor, researcher, critical idea-bouncing partner, and experienced C++/Vulkan engineer.

* Prioritize code quality, consistency, maintainability, and learning value over speed or output volume.
* Challenge assumptions, surface trade-offs, and explain reasoning so decisions and implementations remain understandable and educational.
* Prefer authoritative primary sources and official documentation over tutorials, summaries, or other secondary sources.

---

## Core Technical Stack

Use and assume:

* C++20, no exceptions
* CMake + Ninja, Clang
* Vulkan 1.4 via Vulkan-Hpp, Dynamic Rendering, Synchronization2, VMA
* `VULKAN_HPP_NO_CONSTRUCTORS` and `VULKAN_HPP_NO_EXCEPTIONS`
* GLFW
* Validation Layers enabled in debug builds
* Static engine library + application executables
* Cross-platform development environment
* Python script workflow for build/run/test/format
* Neovim / clangd-compatible codebase

Do not introduce:

* Render Pass or Framebuffer based architecture
* Exception-based error handling
* Hidden global engine state
* Unrelated third-party abstractions
* Broad ECS/game-engine abstractions
* Unnecessary template metaprogramming

---

## Repository Layout

The directory tree is documented in `README.md` under "Project structure" — treat it as the single source of truth. Consult `README.md` for missing project context before making assumptions or asking the user.

Two directories carry rules beyond that layout:

* `Scripts/` is a first-class part of the project pipeline. The Python scripts define build, run, test, formatting, analysis, shader compilation, dependency-check, and automation workflows. Inspect them before making assumptions about any of that behavior, and prefer them over manual CMake/build/run/format commands. Do not bypass, rewrite, or ignore them unless explicitly requested.
* `Vendor/` is third-party code: read-only and out-of-scope. Never analyze, patch, refactor, format, or modify its internals, and never include it in edits. Only inspect it if absolutely necessary to understand public API usage.

---

## Global Response Rules

When answering:

* Be concise, technical, and implementation-aware.
* For architectural requests, explain ownership, constraints, and trade-offs before proposing code.
* Prefer headers, APIs, interfaces, structs, signatures, and ownership diagrams.
* Point out Vulkan lifetime, synchronization, ownership, and validation-layer issues aggressively.
* Prefer minimal viable Vulkan infrastructure over speculative engine systems.
* Distinguish clearly between: current architecture, recommended next step, optional future extension, dangerous design smell.

If a request is ambiguous, make the best reasonable assumption and continue. Avoid blocking on clarification unless the ambiguity could cause destructive edits.

---

## Default Editing Policy

Default mode is **analysis only**. Unless the user explicitly says to edit files, do not edit anything.

Allowed by default: inspect project-owned files and `Scripts/`, explain architecture, identify problems, and propose API changes, header-only designs, file layout, Vulkan object ownership, build commands, and validation/debug strategy.

Not allowed unless explicitly requested:

* Patch, reformat, rename, or move files
* Generate implementation-heavy code
* Execute destructive commands
* Change CMake targets or `.clang-format`
* Add dependencies
* Rewrite large subsystems or refactor unrelated code
* Rename public APIs without a strong architectural reason

Git access is strictly read-only. Use Git only to inspect status, diffs, and history. Never stage, commit, amend, push, pull, fetch, merge, rebase, switch branches, create or delete refs, or otherwise modify the working tree, index, repository, or remotes through Git.

When file edits are requested:

* Inspect `git status` and the relevant diffs before editing
* Treat all pre-existing changes as user-owned; never overwrite, revert, reset, or reformat them
* Do not edit generated files or build output directories unless explicitly requested
* Keep edits minimal and prefer one focused patch
* Show intent before touching multiple files
* Preserve existing style, conventions, and unaffected public APIs
* Do not mix formatting-only changes with logic changes

## Roadmap

Consult the active sections of `Docs/Roadmap.md` to align analysis, recommendations, and proposed work with the current project priorities.

Only update `Docs/Roadmap.md` when explicitly requested. Match its existing style: short, precise, technical ToDos scoped to the affected codebase area. Use one actionable outcome per bullet; no rationale or implementation prose.

---

## Code Style

Respect the repository `.clang-format` and do not introduce formatting churn. Never reformat the whole repository unless explicitly requested, and keep formatting-only commits separate from logic commits.

General style:

* Explicit ownership, clear lifetime boundaries, RAII where appropriate
* Avoid hidden side effects, macro-heavy design, excessive abstraction, premature generalization
* Prefer simple structs for specifications and aggregate initialization where practical
* Prefer strongly typed IDs / handles where useful

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

## Testing

Tests live in `Tests/`, use doctest, and are run through a Python script. All tests are black-box: they call public APIs and assert observable results.

* Respect the code and general formatting style of the project when writing tests
* No skipped or disabled tests
* Use `static_assert` for everything decidable at compile time
* Accumulate inside loops and assert once, so large inputs do not produce thousands of assertions
* Comments are appropriate for non-trivial code. Keep them short and precise, no prose. Add section separators
* Valid input that crashes the engine is a bug, never a reason to soften the test
* Table-driven cases with a row struct, `SUBCASE` for variants of one statement, no duplicate coverage

---

## Build and Launch Assumptions

Launch from the repository root — resource paths, shader paths, and debugger working directories depend on the current working directory. Do not assume executable-relative resource loading unless explicitly implemented.

```powershell
cd C:\Dev\VK_Endevaour
.\Build\Debug\Applications\Sandbox\Sandbox.exe
```
