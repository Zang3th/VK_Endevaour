#!/usr/bin/env python3

from pathlib import Path

TARGET_DIRS = [
    "Engine/Core",
    "Engine/Debug",
    "Engine/Graphics",
    "Engine/Vendor",
    "Applications/Sandbox",
]

EXTENSIONS = [".cpp", ".hpp"]


# ---------------------------------------------------------------------------


def count_lines_in_dir(directory: Path) -> int:
    total = 0
    if not directory.exists():
        return 0

    for path in directory.rglob("*"):
        if path.suffix in EXTENSIONS and path.is_file():
            try:
                with path.open("r", encoding="utf-8", errors="ignore") as f:
                    total += sum(1 for _ in f)
            except Exception:
                pass
    return total


# ---------------------------------------------------------------------------


def main():
    project_root = Path(__file__).resolve().parent.parent

    print("\n====== Code statistics (Lines of code) ======\n")

    for dir in TARGET_DIRS:
        abs_path = project_root / dir
        loc = count_lines_in_dir(abs_path)
        print(f"  {dir:<25} {loc}")

    print("")


if __name__ == "__main__":
    main()
