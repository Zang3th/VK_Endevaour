#!/usr/bin/env python3

import argparse
from ProjectDefines import Paths, run
from pathlib import Path
from collections import defaultdict
import yaml

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

def offset_to_line(path: Path, offset: int) -> int:
    with path.open("rb") as f:
        return f.read(offset).count(b"\n") + 1

# ---------------------------------------------------------------------------

def print_tidy_fixes(path: Path) -> None:
    data = yaml.safe_load(path.read_text(encoding="utf-8"))

    if not data or not data.get("Diagnostics"):
        print("No clang-tidy diagnostics found.")
        return

    diags = data.get("Diagnostics", []) or []

    grouped: dict[str, list[tuple[int, str, str]]] = defaultdict(list)

    for d in diags:
        msg = d.get("DiagnosticMessage") or {}
        file = msg.get("FilePath")
        offset = msg.get("FileOffset")
        text = msg.get("Message") or ""
        name = d.get("DiagnosticName") or ""

        if not file or offset is None:
            continue

        line = offset_to_line(Path(file), int(offset))
        grouped[file].append((line, name, text))

    for file in sorted(grouped.keys()):
        for line, name, text in grouped[file]:
            print(f"{file}:{line}")
            print(f"  [{name}]")
            print(f"    {text}\n")

# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(description="[VK_Endeavour] clang-tidy helper")
    parser.add_argument("-t", "--tidy", action="store_true", help="Run clang-tidy")
    parser.add_argument("-v", "--verify", action="store_true", help="Verify config")

    args = parser.parse_args()

    if args.tidy:
        print("\n====== Running clang-tidy ======\n")
        clang_tidy()
        print("\n====== Printing results ======\n")
        print_tidy_fixes(Paths.SCRIPTS / "tidy-fixes.yaml")
    elif args.verify:
        print("\n====== Verifying config ======\n")
        verify_config()

# ---------------------------------------------------------------------------

if __name__ == "__main__":
    main()
