#!/usr/bin/env python3

import argparse
from pathlib import Path

from ProjectDefines import Paths, run

# ---------------------------------------------------------------------------

RADBIN = Path(r"C:\Dev\RAD_Debugger\radbin.exe")

# ---------------------------------------------------------------------------

def generate_rdi(build_dir: Path):

    if not build_dir.exists():
        print(f"Build directory '{build_dir}' does not exist ...")
        return

    pdb_files = list(build_dir.rglob("*.pdb"))

    if not pdb_files:
        print("Found no '.pdb' files ...\n")
        return

    print(f"Found {len(pdb_files)} '.pdb' files ...")

    for pdb in sorted(pdb_files):
        rdi = pdb.with_suffix(".rdi")

        run([
            str(RADBIN),
            str(pdb),
            f"--out:{rdi}"
        ])

        print(f"Generated '{rdi.name}' from '{pdb.name}' ...")

# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(description="[VK_Endeavour] RAD Debugger helper")

    parser.add_argument(
        "-d",
        "--debug",
        action="store_true",
        help="Generate RDI files for Debug"
    )

    parser.add_argument(
        "-r",
        "--release",
        action="store_true",
        help="Generate RDI files for Release"
    )

    args = parser.parse_args()

    if not RADBIN.exists():
        print(f"Failed to find RAD CLI Binary Utility: '{RADBIN}'")
        return

    if not (args.debug or args.release):
        print("> No action specified. Use '-h' or '--help' for usage information.")
        return

    if args.debug:
        print("\n============ Generating Debug RDI Files ============\n")
        generate_rdi(Paths.DEBUG)

    if args.release:
        print("\n============ Generating Release RDI Files ============\n")
        generate_rdi(Paths.RELEASE)

# ---------------------------------------------------------------------------

if __name__ == "__main__":
    main()
