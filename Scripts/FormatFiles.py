#!/usr/bin/env python3

from pathlib import Path
from ProjectDefines import Paths, run, STANDARD_DIRS, STANDARD_EXTENSIONS

# ---------------------------------------------------------------------------

def iter_source_files(root: Path):
    for p in root.rglob("*"):
        if not p.is_file():
            continue
        if p.suffix not in STANDARD_EXTENSIONS:
            continue
        yield p

# ---------------------------------------------------------------------------

def main():
    print("\n====== Formatting files ======")

    files: list[Path] = []
    for d in STANDARD_DIRS:
        if not d.exists():
            print(f"Warning: missing directory: {d}")
            continue
        files.extend(iter_source_files(d))

        if not files:
            print("No files found.")
            return

    run(["clang-format", "-i", *files], cwd=Paths.PROJECT_ROOT, delimiters=("C:"))
    print(f"Formatted {len(files)} files.")

# ---------------------------------------------------------------------------

if __name__ == "__main__":
    main()
