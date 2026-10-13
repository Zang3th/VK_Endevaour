#!/usr/bin/env python3

import os
import shutil
from pathlib import Path
from ProjectDefines import BuildConfig, add_config_args, banner, create_parser, error, info, run, run_script, selected_configs

# ---------------------------------------------------------------------------

RADBIN_DEFAULT = Path(r"C:\Dev\RAD_Debugger\radbin.exe")

def find_radbin() -> Path | None:
    if "RADBIN" in os.environ:
        candidates = [os.environ["RADBIN"]]
    else:
        candidates = [shutil.which("radbin"), RADBIN_DEFAULT]

    return next((Path(c) for c in candidates if c and Path(c).is_file()), None)

# ---------------------------------------------------------------------------

def generate_rdi(radbin: Path, config: BuildConfig) -> bool:
    if not config.directory.exists():
        error(f"Build directory '{config.directory}' does not exist.")
        return False

    pdb_files = sorted(config.directory.rglob("*.pdb"))

    if not pdb_files:
        info("Found no '.pdb' files.")
        return True

    info(f"Found {len(pdb_files)} '.pdb' files.\n")

    for index, pdb in enumerate(pdb_files):
        rdi = pdb.with_suffix(".rdi")

        if index:
            info("")

        if run([radbin, pdb, f"--out:{rdi}"]).returncode != 0:
            error(f"Generating '{rdi.name}' failed.")
            return False

        info(f"Generated '{rdi.name}' from '{pdb.name}'.")

    return True

# ---------------------------------------------------------------------------

def main() -> int:
    parser = create_parser("Generate RAD Debugger info from the '.pdb' files")
    add_config_args(parser, "Generate RDI files for {}")

    args = parser.parse_args()
    configs = selected_configs(parser, args)
    radbin = find_radbin()

    if not radbin and "RADBIN" in os.environ:
        error(f"Found no RAD CLI Binary Utility at RADBIN='{os.environ['RADBIN']}'.")
        return 1

    if not radbin:
        error(f"Found no RAD CLI Binary Utility. Set RADBIN, add radbin to PATH or install it to '{RADBIN_DEFAULT}'.")
        return 1

    exit_code = 0

    for config in configs:
        banner(f"Generating {config.name} RDI files")
        if not generate_rdi(radbin, config):
            exit_code = 1

    return exit_code

# ---------------------------------------------------------------------------

if __name__ == "__main__":
    run_script(main)
