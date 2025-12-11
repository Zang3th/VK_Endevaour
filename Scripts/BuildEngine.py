#!/usr/bin/env python3

import subprocess
from pathlib import Path
import sys


class Paths:
    SCRIPTS = Path(__file__).resolve().parent
    PROJECT_ROOT = SCRIPTS.parent
    BUILD = PROJECT_ROOT / "Build"
    DEBUG = BUILD / "Debug"
    APP_SRC = PROJECT_ROOT / "Applications"
    APP_BUILD = DEBUG / "Applications" / "Sandbox"
    APP_EXE = APP_BUILD / "Sandbox"


def run(cmd, cwd=None):
    print("> " + " ".join(cmd))
    try:
        subprocess.check_call(cmd, cwd=cwd)
    except subprocess.CalledProcessError:
        print("Command failed, aborting.")
        sys.exit(1)


def main():
    # Create build directory
    Paths.DEBUG.mkdir(parents=True, exist_ok=True)

    # Remove old executable
    if Paths.APP_EXE.exists():
        Paths.APP_EXE.unlink()

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

    # Copy shaders

    # Launch application


if __name__ == "__main__":
    main()
