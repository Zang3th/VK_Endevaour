#!/usr/bin/env python3

import json
import os
import re
import shutil
import sys
from collections import defaultdict
from dataclasses import dataclass
from pathlib import Path

import yaml

from ProjectDefines import COMMAND_NOT_STARTED, Output, Paths, STANDARD_DIRS, STANDARD_FILTER, STANDARD_FILTER_RE, banner, create_parser, dump, error, info, run, run_script, warn

# ---------------------------------------------------------------------------

# Same directory groundtruth as clang-format, without the tests.
SOURCE_FILTER = STANDARD_FILTER
HEADER_FILTER = STANDARD_FILTER
COMPILE_COMMANDS = Paths.DEBUG / "compile_commands.json"
TIDY_CONFIG = Paths.PROJECT_ROOT / ".clang-tidy"
TIDY_FIXES_YAML = Paths.DEBUG / "tidy-fixes.yaml"
PVS_LOG = Paths.DEBUG / "PVS-Studio.log"
PVS_REPORT = Paths.DEBUG / "PVS-Studio.json"
PVS_RULES_CONFIG = Paths.PROJECT_ROOT / "PVS-Studio.pvsconfig"
PVS_SUPPRESS_FILE = Paths.PROJECT_ROOT / "suppress_file.suppress.json"

# ---------------------------------------------------------------------------

def require_compile_commands() -> bool:
    if COMPILE_COMMANDS.exists():
        return True

    error(f"Compilation database '{COMPILE_COMMANDS}' does not exist. Build Debug first.")
    return False

# ---------------------------------------------------------------------------

def clang_tidy() -> int:
    if not require_compile_commands():
        return 1

    if TIDY_FIXES_YAML.exists():
        TIDY_FIXES_YAML.unlink()

    cmd = [
        sys.executable, Paths.SCRIPTS_VENDOR / "RunClangTidy.py",
        "-p", Paths.DEBUG,
        "-config-file", TIDY_CONFIG,
        "-quiet",
        "-source-filter", SOURCE_FILTER,
        "-header-filter", HEADER_FILTER,
        "-export-fixes", TIDY_FIXES_YAML,
    ]

    result = run(cmd, output=Output.CAPTURE)

    if result.returncode == COMMAND_NOT_STARTED:
        return 1

    if not TIDY_FIXES_YAML.exists():
        dump(result.stdout)
        dump(result.stderr)
        error(f"clang-tidy produced no report (exit code {result.returncode}).")
        return 1

    output = result.stdout + result.stderr
    unprocessed = sorted(set(re.findall(r"^Error while processing (.+)\.$", output, re.MULTILINE)))

    if unprocessed:
        warn(f"clang-tidy could not compile {len(unprocessed)} files, their results are incomplete:")
        for file in unprocessed:
            info(f"  {file}")
        info("")

    info(f"Wrote report to '{TIDY_FIXES_YAML}'.")
    return 0

# ---------------------------------------------------------------------------

def find_pvs_tool(names: tuple[str, ...]) -> Path | None:
    for name in names:
        executable = shutil.which(name)
        if executable:
            return Path(executable)

    if os.name != "nt":
        return None

    for environment_variable in ("ProgramFiles(x86)", "ProgramFiles"):
        install_root = os.environ.get(environment_variable)
        if not install_root:
            continue

        for name in names:
            candidate = Path(install_root) / "PVS-Studio" / name
            if candidate.is_file():
                return candidate

    return None

# ---------------------------------------------------------------------------

@dataclass
class PvsSetup:
    analyzer: Path | None
    converter: Path | None

    def prerequisites(self) -> list[tuple[str, str, bool]]:
        return [
            ("Analyzer", str(self.analyzer or "CompilerCommandsAnalyzer.exe / pvs-studio-analyzer"), self.analyzer is not None),
            ("Report converter", str(self.converter or "PlogConverter.exe / plog-converter"), self.converter is not None),
            ("Compilation database", str(COMPILE_COMMANDS), COMPILE_COMMANDS.exists()),
            ("Rules configuration", str(PVS_RULES_CONFIG), PVS_RULES_CONFIG.exists()),
        ]

def locate_pvs() -> PvsSetup:
    return PvsSetup(
        analyzer=find_pvs_tool(("CompilerCommandsAnalyzer.exe", "pvs-studio-analyzer")),
        converter=find_pvs_tool(("PlogConverter.exe", "plog-converter")),
    )

# ---------------------------------------------------------------------------

def check_pvs_license(analyzer: Path, output: Output) -> bool:
    result = run([analyzer, "lic-info"], output=output)

    if result.returncode == 2:
        warn("PVS-Studio license expires in less than one month.")

    return result.returncode in (0, 2)

# ---------------------------------------------------------------------------

def verify_pvs() -> int:
    setup = locate_pvs()
    valid = True

    for label, location, found in setup.prerequisites():
        info(f"{label + ':':<22}{location if found else f'not found ({location})'}")
        valid = valid and found

    info("")

    if not setup.analyzer:
        error("PVS-Studio license not checked, the analyzer is missing.")
        return 1

    if not check_pvs_license(setup.analyzer, Output.SHOW):
        error("PVS-Studio license is missing or no longer valid.")
        valid = False

    return 0 if valid else 1

# ---------------------------------------------------------------------------

def remove_pvs_reports() -> None:
    for report in (PVS_LOG, PVS_REPORT):
        if report.exists():
            report.unlink()

def remove_pvs_leftovers() -> None:
    leftovers = [path for directory in STANDARD_DIRS for path in directory.rglob("*.PVS-Studio.cfg")]

    for leftover in leftovers:
        leftover.unlink()

    if leftovers:
        info(f"Removed {len(leftovers)} temporary PVS-Studio files from the source tree.\n")

# ---------------------------------------------------------------------------

def pvs_studio() -> int:
    remove_pvs_reports()

    setup = locate_pvs()
    missing = [label for label, _, found in setup.prerequisites() if not found]

    if missing or not setup.analyzer:
        error(f"PVS-Studio setup is incomplete, missing: {', '.join(missing)}. Run with '--verify' for details.")
        return 1

    if not check_pvs_license(setup.analyzer, Output.ON_FAILURE):
        error("PVS-Studio license is missing or no longer valid.")
        return 1

    cmd = [
        setup.analyzer, "analyze",
        "-f", COMPILE_COMMANDS,
        "-o", PVS_LOG,
        "-e", Paths.ENGINE_VENDOR,
        "-e", Paths.TESTS,
        "-a", "GA;64;OP",
        "-R", PVS_RULES_CONFIG,
        "-j", "0",
        "-q",
        "--intermodular",
        "--project-root", Paths.PROJECT_ROOT,
    ]

    if PVS_SUPPRESS_FILE.exists():
        cmd.extend(["-s", PVS_SUPPRESS_FILE])

    analysis_result = run(cmd, cwd=Paths.DEBUG, output=Output.ON_FAILURE).returncode
    remove_pvs_leftovers()

    if analysis_result == 5:
        error("PVS-Studio license is no longer valid.")
        return 1

    if analysis_result not in (0, 2):
        error(f"PVS-Studio analysis failed with exit code {analysis_result}.")
        return 1

    if analysis_result == 2:
        warn("PVS-Studio license expires in less than one month.")

    if os.name == "nt":
        cmd = [
            setup.converter, PVS_LOG,
            "-t", "JSON",
            "-o", Paths.DEBUG,
            "-n", PVS_REPORT.stem,
            "-r", Paths.PROJECT_ROOT,
        ]
    else:
        cmd = [
            setup.converter, PVS_LOG,
            "-t", "json",
            "-a", "ALL",
            "-o", PVS_REPORT,
        ]

    conversion_result = run(cmd, cwd=Paths.DEBUG, output=Output.ON_FAILURE).returncode
    if conversion_result != 0 or not PVS_REPORT.exists():
        error(f"PVS-Studio report conversion failed with exit code {conversion_result}.")
        return 1

    return 0

# ---------------------------------------------------------------------------

def verify_config() -> int:
    cmd = [
        "clang-tidy",
        "--verify-config",
        "-config-file", TIDY_CONFIG,
    ]

    returncode = run(cmd).returncode

    if returncode == COMMAND_NOT_STARTED:
        return 1

    if returncode != 0:
        error("clang-tidy config is invalid.")
        return 1

    return 0

# ---------------------------------------------------------------------------

def offset_to_line(path: Path, offset: int) -> int:
    with path.open("rb") as f:
        return f.read(offset).count(b"\n") + 1

# ---------------------------------------------------------------------------

def print_tidy_fixes() -> None:
    if not TIDY_FIXES_YAML.exists():
        info("No clang-tidy report found. Run with '--tidy' first.")
        return

    data = yaml.safe_load(TIDY_FIXES_YAML.read_text(encoding="utf-8"))
    diags = (data or {}).get("Diagnostics") or []
    grouped: dict[str, list[tuple[int, str, str]]] = defaultdict(list)

    for d in diags:
        msg = d.get("DiagnosticMessage") or {}
        file = msg.get("FilePath")
        offset = msg.get("FileOffset")
        text = msg.get("Message") or ""
        name = d.get("DiagnosticName") or ""

        if not file or offset is None:
            continue

        normalized_file = Path(os.path.normpath(file)).resolve()
        if not STANDARD_FILTER_RE.match(str(normalized_file)):
            continue

        line = offset_to_line(normalized_file, int(offset))
        grouped[str(normalized_file)].append((line, name, text))

    if not grouped:
        info("Found no clang-tidy diagnostics in the project sources.")
        return

    for file in sorted(grouped.keys()):
        for line, name, text in sorted(grouped[file]):
            info(f"{file}:{line}")
            info(f"  [{name}]")
            info(f"    {text}\n")

    info(f"clang-tidy diagnostics: {sum(map(len, grouped.values()))}")

# ---------------------------------------------------------------------------

def print_pvs_report() -> None:
    if not PVS_REPORT.exists():
        info("No PVS-Studio report found. Run with '--pvs' first.")
        return

    try:
        data = json.loads(PVS_REPORT.read_text(encoding="utf-8-sig"))
    except (json.JSONDecodeError, OSError) as exception:
        error(f"Could not read PVS-Studio report: {exception}")
        return

    warnings = data.get("warnings", []) if isinstance(data, dict) else []
    grouped: dict[str, list[tuple[int, int, str, int, str]]] = defaultdict(list)

    for warning in warnings:
        positions = warning.get("positions") or warning.get("Positions") or []
        if not positions:
            continue

        position = positions[0]
        file = position.get("file") or position.get("File")
        if not file:
            continue

        normalized_file = Path(os.path.normpath(file))
        if not normalized_file.is_absolute():
            normalized_file = Paths.PROJECT_ROOT / normalized_file
        normalized_file = normalized_file.resolve()

        if not STANDARD_FILTER_RE.match(str(normalized_file)):
            continue

        line = int(position.get("line") or position.get("Line") or 0)
        column = int(position.get("column") or position.get("Column") or 0)
        code = str(warning.get("code") or warning.get("Code") or "PVS-Studio")
        level = int(warning.get("level") or warning.get("Level") or 0)
        text = str(warning.get("message") or warning.get("Message") or "")

        grouped[str(normalized_file)].append((line, column, code, level, text))

    if not grouped:
        info("Found no PVS-Studio diagnostics in the project sources.")
        return

    totals: dict[int, int] = defaultdict(int)

    for file in sorted(grouped.keys()):
        for line, column, code, level, text in sorted(grouped[file]):
            location = f"{file}:{line}"
            if column:
                location += f":{column}"

            certainty = f", level {level}" if level else ""
            info(location)
            info(f"  [{code}{certainty}]")
            info(f"    {text}\n")
            totals[level] += 1

    summary = ", ".join(
        f"level {level}: {count}" if level else f"unspecified: {count}"
        for level, count in sorted(totals.items())
    )
    info(f"PVS-Studio diagnostics: {sum(totals.values())} ({summary})")

# ---------------------------------------------------------------------------

def main() -> int:
    parser = create_parser("Analyze the sources with clang-tidy and PVS-Studio")
    parser.add_argument("-t", "--tidy", action="store_true", help="Run clang-tidy")
    parser.add_argument("--pvs", action="store_true", help="Run PVS-Studio")
    parser.add_argument("-p", "--print", action="store_true", help="Print results")
    parser.add_argument("-v", "--verify", action="store_true", help="Verify config")

    args = parser.parse_args()

    if not (args.tidy or args.pvs or args.print or args.verify):
        parser.error("no action specified, use -t, --pvs, -p and/or -v")

    exit_code = 0

    if args.verify:
        banner("Verifying clang-tidy config")
        if verify_config() != 0:
            exit_code = 1

        banner("Verifying PVS-Studio setup")
        if verify_pvs() != 0:
            exit_code = 1

    if args.tidy:
        banner("Running clang-tidy")
        if clang_tidy() != 0:
            exit_code = 1

    if args.pvs:
        banner("Running PVS-Studio")
        if pvs_studio() != 0:
            exit_code = 1

    if args.print:
        print_all = not args.tidy and not args.pvs
        print_tidy = args.tidy or print_all
        print_pvs = args.pvs or print_all

        if print_tidy:
            banner("clang-tidy results")
            print_tidy_fixes()

        if print_pvs:
            banner("PVS-Studio results")
            print_pvs_report()

    return exit_code

# ---------------------------------------------------------------------------

if __name__ == "__main__":
    run_script(main)
