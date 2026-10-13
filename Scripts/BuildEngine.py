#!/usr/bin/env python3

import shutil
from ProjectDefines import BuildConfig, Paths, add_config_args, banner, create_parser, error, info, run, run_script, selected_configs

# ---------------------------------------------------------------------------

def configure_and_build(config: BuildConfig, clean_build: bool) -> bool:
    if clean_build and config.directory.exists():
        shutil.rmtree(config.directory)
        info(f"Removed directory '{config.directory}'.")

    if not (config.directory / "CMakeCache.txt").exists():
        info("Configuring the build.")
        if run(["cmake", "--preset", config.preset], cwd=Paths.PROJECT_ROOT).returncode != 0:
            error(f"Configuring {config.name} failed.")
            return False

    if run(["cmake", "--build", "--preset", config.preset], cwd=Paths.PROJECT_ROOT).returncode != 0:
        error(f"Building {config.name} failed.")
        return False

    return True

# ---------------------------------------------------------------------------

def main() -> int:
    parser = create_parser("Configure and build the engine")
    add_config_args(parser, "Build {}")
    parser.add_argument("-c", "--clean", action="store_true", help="Delete the build directory first")

    args = parser.parse_args()
    exit_code = 0

    for config in selected_configs(parser, args):
        banner(f"Building {config.name}")
        if not configure_and_build(config, args.clean):
            exit_code = 1

    return exit_code

# ---------------------------------------------------------------------------

if __name__ == "__main__":
    run_script(main)
