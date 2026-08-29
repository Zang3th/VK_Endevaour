#!/usr/bin/env python3

import argparse
import os
import re
import sys
from collections import defaultdict
from pathlib import Path

import yaml

from ProjectDefines import Paths, run, STANDARD_FILTER

# ---------------------------------------------------------------------------

# Keep clang-tidy and clang-format on the same directory groundtruth.
SOURCE_FILTER = STANDARD_FILTER
HEADER_FILTER = STANDARD_FILTER
TIDY_FIXES_YAML = Paths.PROJECT_ROOT / "tidy-fixes.yaml"

SOURCE_FILTER_RE = re.compile(SOURCE_FILTER, re.IGNORECASE)

# ---------------------------------------------------------------------------

def clang_tidy():
    if TIDY_FIXES_YAML.exists():
        TIDY_FIXES_YAML.unlink()

    cmd = [
        "python", str(Paths.SCRIPTS / "RunClangTidy.py"),
        "-p", str(Paths.DEBUG),
        "-config-file", str(Paths.PROJECT_ROOT / ".clang-tidy"),
        "-quiet",
        "-source-filter", SOURCE_FILTER,
        "-header-filter", HEADER_FILTER,
        "-export-fixes", TIDY_FIXES_YAML,
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

def offset_to_line(path: Path, offset: int) -> int:
    with path.open("rb") as f:
        return f.read(offset).count(b"\n") + 1

# ---------------------------------------------------------------------------

def print_tidy_fixes() -> None:
    data = yaml.safe_load(TIDY_FIXES_YAML.read_text(encoding="utf-8"))

    if not data or not data.get("Diagnostics"):
        print("No clang-tidy diagnostics found.")
        return

    diags = data.get("Diagnostics", []) or []
    grouped: dict[str, list[tuple[int, str, str]]] = defaultdict(list)
    validResults = False

    for d in diags:
        msg = d.get("DiagnosticMessage") or {}
        file = msg.get("FilePath")
        offset = msg.get("FileOffset")
        text = msg.get("Message") or ""
        name = d.get("DiagnosticName") or ""

        if not file or offset is None:
            continue

        normalized_file = Path(os.path.normpath(file)).resolve()
        if not SOURCE_FILTER_RE.match(str(normalized_file)):
            continue

        validResults = True

        line = offset_to_line(normalized_file, int(offset))
        grouped[str(normalized_file)].append((line, name, text))

    for file in sorted(grouped.keys()):
        for line, name, text in grouped[file]:
            print(f"{file}:{line}")
            print(f"  [{name}]")
            print(f"    {text}\n")

    if not validResults:
        print("Found no clang-tidy diagnostics in the project sources.\n")
# ---------------------------------------------------------------------------

def main() -> int:
    parser = argparse.ArgumentParser(description="[VK_Endeavour] clang-tidy helper")
    parser.add_argument("-t", "--tidy", action="store_true", help="Run clang-tidy")
    parser.add_argument("-p", "--print", action="store_true", help="Print results")
    parser.add_argument("-v", "--verify", action="store_true", help="Verify config")

    args = parser.parse_args()

    if not (args.tidy or args.print or args.verify):
        print("> No action specified. Use '-h' or '--help' for usage information.")
        return 1

    print("")
    exit_code = 0

    if args.verify:
        print("============ Verifying config ============")
        if verify_config() != 0:
            exit_code = 1

    if args.tidy:
        print("============ Running clang-tidy ============")
        if clang_tidy() != 0:
            exit_code = 1

    if args.print:
        print("============ Printing results ============\n")
        print_tidy_fixes()

    return exit_code

# ---------------------------------------------------------------------------

if __name__ == "__main__":
    sys.exit(main())
