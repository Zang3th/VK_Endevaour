#!/usr/bin/env python3

import argparse
from ProjectDefines import Paths, run
from pathlib import Path
from collections import defaultdict
import yaml

# ---------------------------------------------------------------------------

SOURCE_FILTER = r".*[\\/]Engine[\\/](Core|Debug|Graphics|Math|Platform)[\\/].*|.*[\\/]Applications[\\/].*"
HEADER_FILTER = SOURCE_FILTER
TIDY_FIXES_YAML = Paths.PROJECT_ROOT / "tidy-fixes.yaml"

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

        # Filter unwanted warnings
        path_parts = file.lower().replace("\\", "/").split("/")
        if "vendor" in path_parts or "msvc" in path_parts :
            continue

        validResults = True

        line = offset_to_line(Path(file), int(offset))
        grouped[file].append((line, name, text))

    for file in sorted(grouped.keys()):
        for line, name, text in grouped[file]:
            print(f"{file}:{line}")
            print(f"  [{name}]")
            print(f"    {text}\n")

    # Print something if we had no valid results
    if not validResults:
        print("Found no clang-tidy diagnostics outside of 'Vendor/' and the STL.\n")
# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(description="[VK_Endeavour] clang-tidy helper")
    parser.add_argument("-t", "--tidy", action="store_true", help="Run clang-tidy")
    parser.add_argument("-p", "--print", action="store_true", help="Print results")
    parser.add_argument("-v", "--verify", action="store_true", help="Verify config")

    args = parser.parse_args()

    if not (args.tidy or args.print or args.verify):
        print("> No action specified. Use '-h' or '--help' for usage information.")
        return

    print("")

    if args.verify:
        print("============ Verifying config ============")
        verify_config()

    if args.tidy:
        print("============ Running clang-tidy ============")
        clang_tidy()

    if args.print:
        print("============ Printing results ============\n")
        print_tidy_fixes()

# ---------------------------------------------------------------------------

if __name__ == "__main__":
    main()
