import subprocess
import sys
from pathlib import Path

# ---------------------------------------------------------------------------

def run(cmd, cwd=None, silent=False):
    print("> " + " ".join(map(str, cmd)))
    try:
        if silent:
            return subprocess.call(
                list(map(str, cmd)),
                cwd=cwd,
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
            )
        return subprocess.check_call(list(map(str, cmd)), cwd=cwd)
    except subprocess.CalledProcessError:
        print("> Command failed, aborting!")
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
