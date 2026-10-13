#!/usr/bin/env python3

from ProjectDefines import COMMAND_NOT_STARTED, FORMAT_DIRS, Paths, STANDARD_EXTENSIONS, banner, create_parser, error, info, iter_sources, run, run_script

# ---------------------------------------------------------------------------

def main() -> int:
    parser = create_parser("Format the sources with clang-format")
    parser.add_argument("--check", action="store_true", help="Only report formatting violations, change nothing")

    args = parser.parse_args()

    banner("Checking formatting" if args.check else "Formatting files")

    files = [path.relative_to(Paths.PROJECT_ROOT) for path in iter_sources(FORMAT_DIRS, STANDARD_EXTENSIONS)]

    if not files:
        error("Found no files to format.")
        return 1

    mode = ["--dry-run", "-Werror"] if args.check else ["-i"]

    returncode = run(["clang-format", *mode, *files], cwd=Paths.PROJECT_ROOT).returncode

    if returncode == COMMAND_NOT_STARTED:
        return 1

    if returncode != 0:
        error("Found formatting violations." if args.check else "Formatting failed.")
        return 1

    info(f"Checked {len(files)} files." if args.check else f"Formatted {len(files)} files.")
    return 0

# ---------------------------------------------------------------------------

if __name__ == "__main__":
    run_script(main)
