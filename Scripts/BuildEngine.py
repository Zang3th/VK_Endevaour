#!/usr/bin/env python3

import subprocess
from pathlib import Path
import sys
import shutil

# ---------------------------------------------------------------------------

class Paths:
    SCRIPTS = Path(__file__).resolve().parent
    PROJECT_ROOT = SCRIPTS.parent
    BUILD = PROJECT_ROOT / "Build"
    DEBUG = BUILD / "Debug"
    APP_SRC = PROJECT_ROOT / "Applications" / "Sandbox"
    APP_SRC_SHADERS = APP_SRC / "Shaders"
    APP_BUILD = DEBUG / "Applications" / "Sandbox"
    APP_BUILD_SHADERS = APP_BUILD / "Shaders"

# ---------------------------------------------------------------------------

def run(cmd, cwd=None):
    print("> " + " ".join(cmd))
    try:
        subprocess.check_call(cmd, cwd=cwd)
    except subprocess.CalledProcessError:
        print("Command failed, aborting.")
        sys.exit(1)

# ---------------------------------------------------------------------------

def main():
    # Clean build directory (Debug only)
    if Paths.DEBUG.exists():
        shutil.rmtree(Paths.DEBUG)
        print(f"> Deleted '{Paths.DEBUG}'")

    # Create build directory
    Paths.DEBUG.mkdir(parents=True, exist_ok=True)
    print(f"> Created '{Paths.DEBUG}'")

    # Configure CMake
    run(
        [
            "cmake",
            "../..",
            "-G",
            "Ninja",
            "-DCMAKE_CXX_COMPILER=clang++",
            "-DCMAKE_C_COMPILER=clang",
            "-DCMAKE_BUILD_TYPE=Debug",
        ],
        cwd=Paths.DEBUG,
    )

    # Build everything via Ninja
    run(
        [
            "ninja",
        ],
        cwd=Paths.DEBUG,
    )

    # Copy shaders
    Paths.APP_BUILD_SHADERS.mkdir(parents=True, exist_ok=True)
    for spv in Paths.APP_SRC_SHADERS.rglob("*.spv"):
        shutil.copy2(spv, Paths.APP_BUILD_SHADERS / spv.name)
        print(f"> Copied shader '{spv.name}' to '{Paths.APP_BUILD_SHADERS}'")

# ---------------------------------------------------------------------------

if __name__ == "__main__":
    main()
