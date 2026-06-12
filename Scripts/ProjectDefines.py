import subprocess
import sys
from pathlib import Path

# ---------------------------------------------------------------------------

# Utility function to format the executed commands on every given argument
def format_command(cmd, delimiters=("-",)):
    result = []
    first = True

    for arg in map(str, cmd):
        if not first and any(arg.startswith(d) for d in delimiters):
            result.append("\n  ")

        result.append(arg)
        result.append(" ")

        first = False

    return "".join(result).rstrip()

# ---------------------------------------------------------------------------

def run(cmd, cwd=None, silent=False, delimiters=("-",)):
    print("\n> " + format_command(cmd, delimiters) + "\n")

    try:
        if silent:
            return subprocess.call(
                list(map(str, cmd)),
                cwd=cwd,
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
            )

        return subprocess.check_call(
            list(map(str, cmd)),
            cwd=cwd
        )

    except subprocess.CalledProcessError:
        print("> Command failed, aborting!\n")
        sys.exit(1)

# ---------------------------------------------------------------------------

class Paths:
    # Roots
    SCRIPTS = Path(__file__).resolve().parent
    PROJECT_ROOT = SCRIPTS.parent

    # Applications
    APPS = PROJECT_ROOT / "Applications"
    SANDBOX_SRC = APPS / "Sandbox"

    # Engine
    ENGINE = PROJECT_ROOT / "Engine"

    # Top-level
    ENGINE_CORE = ENGINE / "Core"
    ENGINE_DEBUG = ENGINE / "Debug"
    ENGINE_GRAPHICS = ENGINE / "Graphics"
    ENGINE_MATH = ENGINE / "Math"
    ENGINE_PLATFORM = ENGINE / "Platform"
    ENGINE_VENDOR = ENGINE / "Vendor"

    # Engine / Graphics
    ENGINE_GRAPHICS_IMPORT = ENGINE_GRAPHICS / "Import"
    ENGINE_GRAPHICS_RESOURCES = ENGINE_GRAPHICS / "Resources"
    ENGINE_GRAPHICS_UI = ENGINE_GRAPHICS / "UI"
    ENGINE_GRAPHICS_VULKAN = ENGINE_GRAPHICS / "Vulkan"

    # Engine / Vendor
    ENGINE_VENDOR_DOCTEST = ENGINE_VENDOR / "doctest"
    ENGINE_VENDOR_FMT = ENGINE_VENDOR / "fmt"
    ENGINE_VENDOR_GLFW = ENGINE_VENDOR / "glfw"
    ENGINE_VENDOR_GLM = ENGINE_VENDOR / "glm"
    ENGINE_VENDOR_IMGUI = ENGINE_VENDOR / "imgui"
    ENGINE_VENDOR_STBIMAGE = ENGINE_VENDOR / "stb_image"
    ENGINE_VENDOR_TINYOBJLOADER = ENGINE_VENDOR / "tinyobjloader"
    ENGINE_VENDOR_VMA = ENGINE_VENDOR / "VulkanMemoryAllocator"

    # Build
    BUILD = PROJECT_ROOT / "Build"
    DEBUG = BUILD / "Debug"
    RELEASE = BUILD / "Release"

    # Tests
    TESTS = PROJECT_ROOT / "Tests"

# ---------------------------------------------------------------------------

APP_DIRS = [
    Paths.SANDBOX_SRC,
]

CORE_DIRS = [
    Paths.ENGINE_CORE,
]

DEBUG_DIRS = [
    Paths.ENGINE_DEBUG,
]

GRAPHICS_DIRS = [
    Paths.ENGINE_GRAPHICS_IMPORT,
    Paths.ENGINE_GRAPHICS_RESOURCES,
    Paths.ENGINE_GRAPHICS_UI,
    Paths.ENGINE_GRAPHICS_VULKAN,
]

MATH_DIRS = [
    Paths.ENGINE_MATH,
]

PLATFORM_DIRS = [
    Paths.ENGINE_PLATFORM,
]

VENDOR_DIRS = [
    Paths.ENGINE_VENDOR_DOCTEST,
    Paths.ENGINE_VENDOR_FMT,
    Paths.ENGINE_VENDOR_GLFW,
    Paths.ENGINE_VENDOR_GLM,
    Paths.ENGINE_VENDOR_IMGUI,
    Paths.ENGINE_VENDOR_STBIMAGE,
    Paths.ENGINE_VENDOR_TINYOBJLOADER,
    Paths.ENGINE_VENDOR_VMA,
]

SCRIPT_DIRS = [
    Paths.SCRIPTS,
]

TEST_DIRS = [
    Paths.TESTS,
]

STANDARD_DIRS = [
    *APP_DIRS,
    *CORE_DIRS,
    *DEBUG_DIRS,
    *GRAPHICS_DIRS,
    *MATH_DIRS,
    *PLATFORM_DIRS,
]

STANDARD_EXTENSIONS = {
    ".cpp",
    ".hpp",
}

EXPANDED_DIR_GROUPS = [
    ("Applications", APP_DIRS),
    ("Core", CORE_DIRS),
    ("Debug", DEBUG_DIRS),
    ("Graphics", GRAPHICS_DIRS),
    ("Math", MATH_DIRS),
    ("Platform", PLATFORM_DIRS),
    ("Vendor", VENDOR_DIRS),
    ("Scripts", SCRIPT_DIRS),
    ("Tests", TEST_DIRS),
]

EXPANDED_EXTENSIONS = {
    *STANDARD_EXTENSIONS,
    ".py",
    ".c",
    ".cc",
    ".h",
}
