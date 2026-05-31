#!/usr/bin/env python3

import argparse
from pathlib import Path
from ProjectDefines import Paths, run

# ---------------------------------------------------------------------------

def run_tests(build_dir: Path):
    if not build_dir.exists():
        print(f"Build directory '{build_dir}' does not exist ...")
        return

    print(f"\n====== Running Tests ({build_dir.name}) ======")

    run(
        [str(build_dir / "Tests" / "EngineTests.exe")],
        cwd=build_dir
    )

# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(description="[VK_Endeavour] test helper")

    parser.add_argument(
        "-d",
        "--debug",
        action="store_true",
        help="Run Debug tests"
    )

    parser.add_argument(
        "-r",
        "--release",
        action="store_true",
        help="Run Release tests"
    )

    args = parser.parse_args()

    if not (args.debug or args.release):
        print("> No action specified. Use '-h' or '--help' for usage information.")
        return

    if args.debug:
        run_tests(Paths.DEBUG)

    if args.release:
        run_tests(Paths.RELEASE)

# ---------------------------------------------------------------------------

if __name__ == "__main__":
    main()
