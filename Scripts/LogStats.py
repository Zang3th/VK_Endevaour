#!/usr/bin/env python3

from pathlib import Path
from ProjectDefines import EXPANDED_DIRS, EXPANDED_EXTENSIONS

# ---------------------------------------------------------------------------

def count_lines_in_dir(directory: Path) -> int:
    total = 0

    for path in directory.rglob("*"):
        if path.suffix in EXPANDED_EXTENSIONS and path.is_file():
            try:
                with path.open("r", encoding="utf-8", errors="ignore") as f:
                    total += sum(1 for _ in f)
            except Exception:
                pass
    return total

# ---------------------------------------------------------------------------

def main():
    print("\n====== Code statistics (Lines of code) ======\n")

    for dir in EXPANDED_DIRS:
        loc = count_lines_in_dir(dir)
        print(f"  {str(dir):<50} {loc}")

    print("")

# ---------------------------------------------------------------------------

if __name__ == "__main__":
    main()
