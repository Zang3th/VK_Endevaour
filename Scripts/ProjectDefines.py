import argparse
import functools
import os
import re
import subprocess
import sys
from collections.abc import Callable, Iterable, Iterator
from dataclasses import dataclass
from enum import Enum, auto
from pathlib import Path

# ---------------------------------------------------------------------------

class Paths:
    # Roots
    SCRIPTS = Path(__file__).resolve().parent
    SCRIPTS_VENDOR = SCRIPTS / "Vendor"
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

FORMAT_DIRS = [
    *STANDARD_DIRS,
    *TEST_DIRS,
]

STANDARD_EXTENSIONS = {
    ".cpp",
    ".hpp",
}

# ---------------------------------------------------------------------------
# Regex filters generated from the shared directory groundtruth
# ---------------------------------------------------------------------------

PATH_SEPARATOR = r"[\\/]"

def dir_to_pattern(directory: Path) -> str:
    parts = directory.relative_to(Paths.PROJECT_ROOT).parts
    path_pattern = PATH_SEPARATOR.join(map(re.escape, parts))
    return f".*{PATH_SEPARATOR}{path_pattern}{PATH_SEPARATOR}.*"

def dirs_to_pattern(dirs: list[Path]) -> str:
    patterns = [dir_to_pattern(directory) for directory in dirs]
    return "(?:" + "|".join(patterns) + ")" if patterns else r"(?!)"

STANDARD_FILTER = dirs_to_pattern(STANDARD_DIRS)
STANDARD_FILTER_RE = re.compile(STANDARD_FILTER)

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

# ---------------------------------------------------------------------------

def iter_sources(dirs: Iterable[Path], extensions: set[str]) -> Iterator[Path]:
    for directory in dirs:
        if not directory.is_dir():
            warn(f"Missing directory '{directory}'.")
            continue

        yield from sorted(p for p in directory.rglob("*") if p.suffix in extensions and p.is_file())

# ---------------------------------------------------------------------------
# Build configurations
# ---------------------------------------------------------------------------

@dataclass(frozen=True)
class BuildConfig:
    name: str
    directory: Path
    preset: str

BUILD_CONFIGS = [
    BuildConfig("Debug", Paths.DEBUG, "clang-debug"),
    BuildConfig("Release", Paths.RELEASE, "clang-release"),
]

def add_config_args(parser: argparse.ArgumentParser, help_format: str) -> None:
    for config in BUILD_CONFIGS:
        name = config.name.lower()
        parser.add_argument(f"-{name[0]}", f"--{name}", action="store_true", help=help_format.format(config.name))

def selected_configs(parser: argparse.ArgumentParser, args: argparse.Namespace) -> list[BuildConfig]:
    configs = [config for config in BUILD_CONFIGS if getattr(args, config.name.lower())]

    if not configs:
        parser.error("no build configuration specified, use -d and/or -r")

    return configs

# ---------------------------------------------------------------------------
# Script frame
# ---------------------------------------------------------------------------

def create_parser(description: str) -> argparse.ArgumentParser:
    return argparse.ArgumentParser(description=f"[VK_Endevaour] {description}")

def run_script(main: Callable[[], int]) -> None:
    exit_code = main()

    if not _last_line_blank:
        print("")

    sys.exit(exit_code)

# ---------------------------------------------------------------------------
# Output
# ---------------------------------------------------------------------------

_last_line_blank = False

def _write(text: str) -> None:
    global _last_line_blank
    print(text, flush=True)
    _last_line_blank = text == "" or text.endswith("\n")

def dump(text: str) -> None:
    global _last_line_blank
    if text:
        print(text, end="", flush=True)
        _last_line_blank = text.endswith("\n\n")

def banner(title: str) -> None:
    if not _last_line_blank:
        print("")

    _write(f"============ {title} ============\n")

def info(message: str) -> None:
    _write(message)

def warn(message: str) -> None:
    _write(f"Warning: {message}")

def error(message: str) -> None:
    _write(f"Error: {message}")

@functools.cache
def _colors_enabled() -> bool:
    if "NO_COLOR" in os.environ or not sys.stdout.isatty():
        return False

    if os.name != "nt":
        return True

    import ctypes

    kernel32 = ctypes.windll.kernel32
    handle = kernel32.GetStdHandle(-11)
    mode = ctypes.c_uint32()
    enable_virtual_terminal_processing = 0x0004

    if not kernel32.GetConsoleMode(handle, ctypes.byref(mode)):
        return False

    return bool(kernel32.SetConsoleMode(handle, mode.value | enable_virtual_terminal_processing))

def status_tag(passed: bool) -> str:
    text, color = (" OK ", "32") if passed else ("FAIL", "31")
    return f"[\033[1;{color}m{text}\033[0m]" if _colors_enabled() else f"[{text}]"

# ---------------------------------------------------------------------------
# Command execution
# ---------------------------------------------------------------------------

class Output(Enum):
    SHOW = auto()
    ON_FAILURE = auto()
    CAPTURE = auto()

COMMAND_NOT_STARTED = 127

def format_command(cmd: list) -> str:
    lines = [str(cmd[0])]
    neighbours = zip(cmd, cmd[1:], [*cmd[2:], None])

    for previous, arg, following in neighbours:
        in_path_list = isinstance(arg, Path) and (isinstance(previous, Path) or isinstance(following, Path))

        if str(arg).startswith("-") or in_path_list:
            lines.append(str(arg))
        else:
            lines[-1] += f" {arg}"

    return "\n  ".join(lines)

def run(cmd: list, cwd: Path | None = None, output: Output = Output.SHOW) -> subprocess.CompletedProcess:
    global _last_line_blank
    info(f"> {format_command(cmd)}\n")
    args = list(map(str, cmd))

    try:
        result = subprocess.run(
            args,
            cwd=cwd,
            capture_output=output is not Output.SHOW,
            encoding="utf-8",
            errors="replace",
        )
    except OSError as exception:
        error(f"Could not start '{args[0]}': {exception.strerror}. Run CheckDependencies.py to verify the toolchain.")
        return subprocess.CompletedProcess(args, COMMAND_NOT_STARTED, "", "")

    if output is Output.SHOW:
        _last_line_blank = False

    if output is Output.ON_FAILURE and result.returncode != 0:
        dump(result.stdout)
        dump(result.stderr)

    return result
