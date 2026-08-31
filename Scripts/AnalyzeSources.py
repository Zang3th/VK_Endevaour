#!/usr/bin/env python3

import argparse
import json
import os
import re
import shutil
import subprocess
import sys
from collections import defaultdict
from pathlib import Path

import yaml

from ProjectDefines import Paths, run, STANDARD_FILTER, format_command

# ---------------------------------------------------------------------------

# Keep clang-tidy and clang-format on the same directory groundtruth.
SOURCE_FILTER = STANDARD_FILTER
HEADER_FILTER = STANDARD_FILTER
TIDY_FIXES_YAML = Paths.PROJECT_ROOT / "tidy-fixes.yaml"
PVS_COMPILE_COMMANDS = Paths.DEBUG / "compile_commands.json"
PVS_LOG = Paths.DEBUG / "PVS-Studio.log"
PVS_REPORT = Paths.DEBUG / "PVS-Studio.json"
PVS_RULES_CONFIG = Paths.PROJECT_ROOT / "PVS-Studio.pvsconfig"

SOURCE_FILTER_RE = re.compile(SOURCE_FILTER, re.IGNORECASE)

# ---------------------------------------------------------------------------

def clang_tidy():
    if TIDY_FIXES_YAML.exists():
        TIDY_FIXES_YAML.unlink()

    cmd = [
        "python", str(Paths.SCRIPTS / "RunClangTidy.py"),
        "-p", str(Paths.DEBUG),
        "-config-file", str(Paths.PROJECT_ROOT / ".clang-tidy"),
        "-quiet",
        "-source-filter", SOURCE_FILTER,
        "-header-filter", HEADER_FILTER,
        "-export-fixes", TIDY_FIXES_YAML,
    ]
    return run(cmd, silent=True)

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

def run_pvs_command(cmd: list[str | Path], cwd: Path | None = None) -> int:
    print("> " + format_command(cmd) + "\n", flush=True)
    return subprocess.call(
        list(map(str, cmd)),
        cwd=cwd,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )

# ---------------------------------------------------------------------------

def verify_pvs() -> int:
    analyzer = find_pvs_tool(("CompilerCommandsAnalyzer.exe", "pvs-studio-analyzer"))
    converter = find_pvs_tool(("PlogConverter.exe", "plog-converter"))
    valid = True

    if analyzer:
        print(f"Analyzer:             {analyzer}")
    else:
        print("Analyzer:             not found")
        valid = False

    if converter:
        print(f"Report converter:     {converter}")
    else:
        print("Report converter:     not found")
        valid = False

    if PVS_COMPILE_COMMANDS.exists():
        print(f"Compilation database: {PVS_COMPILE_COMMANDS}")
    else:
        print(f"Compilation database: not found ({PVS_COMPILE_COMMANDS})")
        valid = False

    if PVS_RULES_CONFIG.exists():
        print(f"Rules configuration:  {PVS_RULES_CONFIG}")
    else:
        print(f"Rules configuration:  not found ({PVS_RULES_CONFIG})")
        valid = False

    if not analyzer:
        print("License:              not checked\n")
        return 1

    license_result = subprocess.run(
        [analyzer, "lic-info"],
        capture_output=True,
        text=True,
        encoding="utf-8",
        errors="replace",
    )
    license_info = (license_result.stdout or license_result.stderr).strip().splitlines()

    if license_result.returncode in (0, 2):
        print("License:              valid")
        for line in license_info:
            print(f"  {line}")

        if license_result.returncode == 2:
            print("  Warning: license expires in less than one month.")
    else:
        print("License:              missing or no longer valid")
        for line in license_info:
            print(f"  {line}")
        valid = False

    print("")
    return 0 if valid else 1

# ---------------------------------------------------------------------------

def remove_pvs_reports() -> None:
    for report in (PVS_LOG, PVS_REPORT):
        if report.exists():
            report.unlink()

# ---------------------------------------------------------------------------

def pvs_studio() -> int:
    remove_pvs_reports()

    analyzer = find_pvs_tool(("CompilerCommandsAnalyzer.exe", "pvs-studio-analyzer"))
    converter = find_pvs_tool(("PlogConverter.exe", "plog-converter"))

    if not analyzer:
        print("> PVS-Studio analyzer CLI was not found. Install it or add it to PATH.\n")
        return 1

    if not converter:
        print("> PVS-Studio report converter was not found. Install it or add it to PATH.\n")
        return 1

    if not PVS_COMPILE_COMMANDS.exists():
        print(f"> Compilation database '{PVS_COMPILE_COMMANDS}' does not exist. Build Debug first.\n")
        return 1

    if not PVS_RULES_CONFIG.exists():
        print(f"> PVS-Studio rules configuration '{PVS_RULES_CONFIG}' does not exist.\n")
        return 1

    license_result = run_pvs_command([analyzer, "lic-info"])
    if license_result not in (0, 2):
        print("> PVS-Studio license is missing or no longer valid.\n")
        return 1

    if license_result == 2:
        print("> PVS-Studio license expires in less than one month.\n")

    cmd = [
        analyzer, "analyze",
        "-f", PVS_COMPILE_COMMANDS,
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

    suppress_file = Paths.PROJECT_ROOT / "suppress_file.suppress.json"
    if suppress_file.exists():
        cmd.extend(["-s", suppress_file])

    analysis_result = run_pvs_command(cmd, cwd=Paths.DEBUG)
    if analysis_result == 5:
        print("> PVS-Studio license is no longer valid.\n")
        return 1

    if analysis_result not in (0, 2):
        print(f"> PVS-Studio analysis failed with exit code {analysis_result}.\n")
        return 1

    if analysis_result == 2:
        print("> PVS-Studio license expires in less than one month.\n")

    if os.name == "nt":
        cmd = [
            converter, PVS_LOG,
            "-t", "JSON",
            "-o", Paths.DEBUG,
            "-n", PVS_REPORT.stem,
            "-r", Paths.PROJECT_ROOT,
        ]
    else:
        cmd = [
            converter, PVS_LOG,
            "-t", "json",
            "-a", "ALL",
            "-o", PVS_REPORT,
        ]

    conversion_result = run_pvs_command(cmd, cwd=Paths.DEBUG)
    if conversion_result != 0 or not PVS_REPORT.exists():
        print(f"> PVS-Studio report conversion failed with exit code {conversion_result}.\n")
        return 1

    return 0

# ---------------------------------------------------------------------------

def verify_config():
    cmd = [
        "clang-tidy",
        "--verify-config",
        "-config-file", str(Paths.PROJECT_ROOT / ".clang-tidy"),
    ]
    print("> " + format_command(cmd) + "\n", flush=True)
    return subprocess.call(cmd)

# ---------------------------------------------------------------------------

def offset_to_line(path: Path, offset: int) -> int:
    with path.open("rb") as f:
        return f.read(offset).count(b"\n") + 1

# ---------------------------------------------------------------------------

def print_tidy_fixes() -> None:
    if not TIDY_FIXES_YAML.exists():
        print("No clang-tidy report found. Run with '--tidy' first.\n")
        return

    data = yaml.safe_load(TIDY_FIXES_YAML.read_text(encoding="utf-8"))

    if not data or not data.get("Diagnostics"):
        print("No clang-tidy diagnostics found.\n")
        return

    diags = data.get("Diagnostics", []) or []
    grouped: dict[str, list[tuple[int, str, str]]] = defaultdict(list)
    validResults = False

    for d in diags:
        msg = d.get("DiagnosticMessage") or {}
        file = msg.get("FilePath")
        offset = msg.get("FileOffset")
        text = msg.get("Message") or ""
        name = d.get("DiagnosticName") or ""

        if not file or offset is None:
            continue

        normalized_file = Path(os.path.normpath(file)).resolve()
        if not SOURCE_FILTER_RE.match(str(normalized_file)):
            continue

        validResults = True

        line = offset_to_line(normalized_file, int(offset))
        grouped[str(normalized_file)].append((line, name, text))

    for file in sorted(grouped.keys()):
        for line, name, text in grouped[file]:
            print(f"{file}:{line}")
            print(f"  [{name}]")
            print(f"    {text}\n")

    if not validResults:
        print("Found no clang-tidy diagnostics in the project sources.\n")

# ---------------------------------------------------------------------------

def print_pvs_report() -> None:
    if not PVS_REPORT.exists():
        print("No PVS-Studio report found. Run with '--pvs' first.\n")
        return

    try:
        data = json.loads(PVS_REPORT.read_text(encoding="utf-8-sig"))
    except (json.JSONDecodeError, OSError) as error:
        print(f"Could not read PVS-Studio report: {error}\n")
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

        if not SOURCE_FILTER_RE.match(str(normalized_file)):
            continue

        line = int(position.get("line") or position.get("Line") or 0)
        column = int(position.get("column") or position.get("Column") or 0)
        code = str(warning.get("code") or warning.get("Code") or "PVS-Studio")
        level = int(warning.get("level") or warning.get("Level") or 0)
        text = str(warning.get("message") or warning.get("Message") or "")

        grouped[str(normalized_file)].append((line, column, code, level, text))

    if not grouped:
        print("Found no PVS-Studio diagnostics in the project sources.\n")
        return

    totals: dict[int, int] = defaultdict(int)

    for file in sorted(grouped.keys()):
        for line, column, code, level, text in sorted(grouped[file]):
            location = f"{file}:{line}"
            if column:
                location += f":{column}"

            certainty = f", level {level}" if level else ""
            print(location)
            print(f"  [{code}{certainty}]")
            print(f"    {text}\n")
            totals[level] += 1

    summary = ", ".join(
        f"level {level}: {count}" if level else f"unspecified: {count}"
        for level, count in sorted(totals.items())
    )
    print(f"PVS-Studio diagnostics: {sum(totals.values())} ({summary})\n")

# ---------------------------------------------------------------------------

def main() -> int:
    parser = argparse.ArgumentParser(description="[VK_Endeavour] Analyze Sources")
    parser.add_argument("-t", "--tidy", action="store_true", help="Run clang-tidy")
    parser.add_argument("--pvs", action="store_true", help="Run PVS-Studio")
    parser.add_argument("-p", "--print", action="store_true", help="Print results")
    parser.add_argument("-v", "--verify", action="store_true", help="Verify config")

    args = parser.parse_args()

    if not (args.tidy or args.pvs or args.print or args.verify):
        print("> No action specified. Use '-h' or '--help' for usage information.")
        return 1

    print("")
    exit_code = 0

    if args.verify:
        print("============ Verifying clang-tidy config ============\n", flush=True)
        if verify_config() != 0:
            exit_code = 1

        print("")
        print("============ Verifying PVS-Studio setup ============\n")
        if verify_pvs() != 0:
            exit_code = 1

    if args.tidy:
        print("============ Running clang-tidy ============")
        if clang_tidy() != 0:
            exit_code = 1

    if args.pvs:
        print("============ Running PVS-Studio ============\n")
        if pvs_studio() != 0:
            exit_code = 1

    if args.print:
        print_all = not args.tidy and not args.pvs
        print_tidy = args.tidy or print_all
        print_pvs = args.pvs or print_all

        if print_tidy:
            print("============ clang-tidy results ============\n")
            print_tidy_fixes()

        if print_pvs:
            print("============ PVS-Studio results ============\n")
            print_pvs_report()

    return exit_code

# ---------------------------------------------------------------------------

if __name__ == "__main__":
    sys.exit(main())
