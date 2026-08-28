#!/usr/bin/env python3

import argparse
import shutil
from pathlib import Path
from ProjectDefines import Paths, run

# ---------------------------------------------------------------------------

def configure_and_build(build_dir: Path, preset: str, clean_build: bool):
    if clean_build:
        print("Clean build ...")
        # Delete specific build subdirectory if it exists
        if build_dir.exists():
            shutil.rmtree(build_dir)
            print(f"Removed directory '{build_dir}' ...")

    if not (build_dir / "CMakeCache.txt").exists():
        print("Configure build ...")
        run(["cmake", "--preset", preset], cwd=Paths.PROJECT_ROOT)
    else:
        print("Fast build ...")

    run(["cmake", "--build", "--preset", preset], cwd=Paths.PROJECT_ROOT)

# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(description="[VK_Endeavour] build helper")
    parser.add_argument("-d", "--debug", action="store_true", help="Build Debug")
    parser.add_argument("-r", "--release", action="store_true", help="Build Release")
    parser.add_argument("-c", "--clean", action="store_true", help="Clean build")

    args = parser.parse_args()

    if not (args.debug or args.release):
        print("> No action specified. Use '-h' or '--help' for usage information.")
        return

    if args.debug:
        print("\n============ Building Debug ============\n")
        configure_and_build(Paths.DEBUG, "clang-debug", args.clean)

    if args.release:
        print("\n============ Building Release ============\n")
        configure_and_build(Paths.RELEASE, "clang-release", args.clean)

    print("")

# ---------------------------------------------------------------------------

if __name__ == "__main__":
    main()
