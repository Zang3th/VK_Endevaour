#!/usr/bin/env python3

from pathlib import Path
from ProjectDefines import EXPANDED_DIR_GROUPS, EXPANDED_EXTENSIONS, Paths, banner, create_parser, info, iter_sources, run_script

# ---------------------------------------------------------------------------

def count_lines(directory: Path) -> int:
    total = 0

    for path in iter_sources([directory], EXPANDED_EXTENSIONS):
        with path.open("r", encoding="utf-8", errors="ignore") as f:
            total += sum(1 for _ in f)

    return total

# ---------------------------------------------------------------------------

def main() -> int:
    parser = create_parser("Count the lines of code per directory")
    parser.parse_args()

    banner("Code statistics (Lines of code)")

    directories = [directory for _, dirs in EXPANDED_DIR_GROUPS for directory in dirs]
    width = max(len(str(directory.relative_to(Paths.PROJECT_ROOT))) for directory in directories) + 2
    total = 0

    for group_name, dirs in EXPANDED_DIR_GROUPS:
        info(f"{group_name}:")

        for directory in dirs:
            name = str(directory.relative_to(Paths.PROJECT_ROOT))

            if not directory.is_dir():
                info(f"  {name:<{width}} missing")
                continue

            lines = count_lines(directory)
            total += lines
            info(f"  {name:<{width}} {lines:>7}")

        info("")

    info(f"  {'Total':<{width}} {total:>7}")
    return 0

# ---------------------------------------------------------------------------

if __name__ == "__main__":
    run_script(main)
