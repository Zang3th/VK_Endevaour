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
    ENGINE_CORE = ENGINE / "Core"
    ENGINE_DEBUG = ENGINE / "Debug"
    ENGINE_GRAPHICS = ENGINE / "Graphics"
    ENGINE_MATH = ENGINE / "Math"
    ENGINE_PLATFORM = ENGINE / "Platform"
    ENGINE_VENDOR = ENGINE / "Vendor"

    # Build
    BUILD = PROJECT_ROOT / "Build"
    DEBUG = BUILD / "Debug"
    RELEASE = BUILD / "Release"

# ---------------------------------------------------------------------------

STANDARD_DIRS = [
    Paths.SANDBOX_SRC,
    Paths.ENGINE_CORE,
    Paths.ENGINE_DEBUG,
    Paths.ENGINE_GRAPHICS,
    Paths.ENGINE_MATH,
    Paths.ENGINE_PLATFORM,
]

STANDARD_EXTENSIONS = {
    ".cpp",
    ".hpp",
}

EXPANDED_DIRS = [
    *STANDARD_DIRS,
    Paths.ENGINE_VENDOR,
    Paths.SCRIPTS,
]

EXPANDED_EXTENSIONS = {
    *STANDARD_EXTENSIONS,
    ".py",
}
