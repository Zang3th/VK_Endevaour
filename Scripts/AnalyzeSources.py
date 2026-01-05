#!/usr/bin/env python3

import argparse
from ProjectDefines import Paths, run

# ---------------------------------------------------------------------------

SOURCE_FILTER = r".*[\\/]Engine[\\/](Core|Debug|Graphics|Platform)[\\/].*|.*[\\/]Applications[\\/].*"
HEADER_FILTER = SOURCE_FILTER

# ---------------------------------------------------------------------------

def clang_tidy():
    cmd = [
        "python", str(Paths.SCRIPTS / "RunClangTidy.py"),
        "-p", str(Paths.DEBUG),
        "-config-file", str(Paths.PROJECT_ROOT / ".clang-tidy"),
        "-quiet",
        "-source-filter", SOURCE_FILTER,
        "-header-filter", HEADER_FILTER,
        "-export-fixes", str(Paths.SCRIPTS / "tidy-fixes.yaml"),
    ]
    return run(cmd, silent=True)

# ---------------------------------------------------------------------------

def verify_config():
    cmd = [
        "clang-tidy",
        "--verify-config",
        "-config-file", str(Paths.PROJECT_ROOT / ".clang-tidy"),
    ]
    return run(cmd)

# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(description="[VK_Endeavour] clang-tidy helper")
    parser.add_argument("-t", "--tidy", action="store_true", help="Run clang-tidy")
    parser.add_argument("-v", "--verify", action="store_true", help="Verify config")

    args = parser.parse_args()

    if args.tidy:
        print("\n====== Running clang-tidy ======\n")
        clang_tidy()
    elif args.verify:
        print("\n====== Verifying config ======\n")
        verify_config()

# ---------------------------------------------------------------------------

if __name__ == "__main__":
    main()
