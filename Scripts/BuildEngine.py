#!/usr/bin/env python3

import argparse
import shutil
from pathlib import Path
from ProjectDefines import Paths, run

# ---------------------------------------------------------------------------

def configure_and_build(build_dir: Path, build_type: str, clean_build: bool):
    if clean_build:
        print("Clean rebuild ...")
        # Delete specific build subdirectory if it exists
        if build_dir.exists():
            shutil.rmtree(build_dir)
            print(f"Removed directory '{build_dir}'...")

        build_dir.mkdir(parents=True, exist_ok=True)
        print(f"Created directory '{build_dir}' ...")

        cmd = [
            "cmake",
            "../..",
            "-G", "Ninja",
            "-DCMAKE_CXX_COMPILER=clang++",
            "-DCMAKE_C_COMPILER=clang",
            f"-DCMAKE_BUILD_TYPE={build_type}",
            "-DCMAKE_MESSAGE_LOG_LEVEL=WARNING",
        ]
        run(cmd, cwd=build_dir)
    else:
        print("Fast build ...")
    run(["ninja"], cwd=build_dir)

# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(description="[VK_Endeavour] build helper")
    parser.add_argument("-d", "--debug", action="store_true", help="Build Debug")
    parser.add_argument("-r", "--release", action="store_true", help="Build Release")
    parser.add_argument("-c", "--clean", action="store_true", help="Fast Rebuild")

    args = parser.parse_args()

    if not (args.debug or args.release):
        print("> No action specified. Use '-h' or '--help' for usage information.")
        return

    if args.debug:
        print("\n============ Building Debug ============\n")
        configure_and_build(Paths.DEBUG, "Debug", args.clean)

    if args.release:
        print("\n============ Building Release ============\n")
        configure_and_build(Paths.RELEASE, "Release", args.clean)

    print("")

# ---------------------------------------------------------------------------

if __name__ == "__main__":
    main()
