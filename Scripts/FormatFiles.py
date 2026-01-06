#!/usr/bin/env python3

from pathlib import Path
from ProjectDefines import Paths, run, TARGET_DIRS, EXTENSIONS

# ---------------------------------------------------------------------------

def iter_source_files(root: Path):
    for p in root.rglob("*"):
        if not p.is_file():
                continue
        if p.suffix not in EXTENSIONS:
                continue
        yield p

# ---------------------------------------------------------------------------

def main() -> int:
    files: list[Path] = []
    for d in TARGET_DIRS:
        if not d.exists():
            print(f"> Warning: missing directory: {d}")
            continue
        files.extend(iter_source_files(d))

        if not files:
            print("> No files found.")
            return 0

    run(["clang-format", "-i", *files], cwd=Paths.PROJECT_ROOT)
    print(f"> Formatted {len(files)} files.")
    return 0

# ---------------------------------------------------------------------------

if __name__ == "__main__":
    main()
