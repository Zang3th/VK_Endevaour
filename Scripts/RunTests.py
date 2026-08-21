#!/usr/bin/env python3

import argparse
import subprocess
import sys
import xml.etree.ElementTree as ET
from ProjectDefines import Paths, format_command
from pathlib import Path
from collections import defaultdict

# ---------------------------------------------------------------------------

TEST_EXECUTABLE = "EngineTests"
TEST_RESULTS_XML = "test-results.xml"

# ---------------------------------------------------------------------------

def executable_path(build_dir: Path) -> Path:
    executable = build_dir / "Tests" / (TEST_EXECUTABLE + ".exe")

    return executable if executable.exists() else build_dir / "Tests" / TEST_EXECUTABLE

# ---------------------------------------------------------------------------

# Kept inside the build directory: it is a per-configuration artifact, gets wiped by a clean rebuild
def results_path(build_dir: Path) -> Path:
    return build_dir / "Tests" / TEST_RESULTS_XML

# ---------------------------------------------------------------------------

def run_tests(build_dir: Path):
    executable = executable_path(build_dir)

    if not executable.exists():
        print(f"Test executable '{executable}' does not exist ...")
        return

    results = results_path(build_dir)

    if results.exists():
        results.unlink()

    cmd = [
        str(executable),
        "--reporters=xml",
        f"--out={results}",
    ]

    print("\n> " + format_command(cmd) + "\n")

    # The engine logs to stdout and would bury the report, so its output is captured instead of inherited.
    process = subprocess.run(
        cmd,
        cwd=build_dir,
        capture_output=True,
        encoding="utf-8",
        errors="replace",
    )

    if not results.exists():
        print(process.stdout, end="")
        print(process.stderr, end="")
        print(f"> Test run produced no report (exit code {process.returncode}), aborting!\n")
        sys.exit(1)

    print(f"Wrote report to '{results}' ...")

# ---------------------------------------------------------------------------

def collect_failures(root: ET.Element) -> dict[str, list[tuple[int, str, str, str]]]:
    grouped: dict[str, list[tuple[int, str, str, str]]] = defaultdict(list)

    for test_case in root.iter("TestCase"):
        name = test_case.get("name") or ""

        for expression in test_case.iter("Expression"):
            if expression.get("success") == "true":
                continue

            file = expression.get("filename") or ""
            line = int(expression.get("line") or 0)
            kind = expression.get("type") or ""
            original = (expression.findtext("Original") or "").strip()
            expanded = (expression.findtext("Expanded") or "").strip()

            grouped[file].append((line, name, f"{kind}( {original} )", expanded))

        # An unexpected throw or a crash is reported separately from the assertions
        for exception in test_case.iter("Exception"):
            file = test_case.get("filename") or ""
            line = int(test_case.get("line") or 0)
            kind = "CRASH" if exception.get("crash") == "true" else "EXCEPTION"

            grouped[file].append((line, name, kind, (exception.text or "").strip()))

    return grouped

# ---------------------------------------------------------------------------

def print_test_results(build_dir: Path) -> None:
    results = results_path(build_dir)

    if not results.exists():
        print("No test results found. Run with '--test' first ...\n")
        return

    try:
        root = ET.parse(results).getroot()
    except ET.ParseError:
        print(f"Test report '{results}' is incomplete ... (The run probably crashed)\n")
        return

    grouped = collect_failures(root)

    for file in sorted(grouped.keys()):
        for line, name, expression, expanded in sorted(grouped[file]):
            print(f"{file}:{line}")
            print(f"  [{name}]")
            print(f"    {expression}")
            print(f"    -> {expanded}\n")

    if not grouped:
        print("Found no failing assertions.\n")

    # Skipped cases are deliberate gaps, so they are listed instead of being reduced to a number
    skipped = [t.get("name") or "" for t in root.iter("TestCase") if t.get("skipped") == "true"]

    if skipped:
        print("Skipped:")
        for name in sorted(skipped):
            print(f"  {name}")
        print("")

    cases = root.find("OverallResultsTestCases")
    asserts = root.find("OverallResultsAsserts")

    if cases is not None:
        print(f"Test cases: {cases.get('successes')} passed, "
              f"{cases.get('failures')} failed, "
              f"{cases.get('skipped')} skipped")

    if asserts is not None:
        print(f"Assertions: {asserts.get('successes')} passed, "
              f"{asserts.get('failures')} failed")

# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(description="[VK_Endeavour] test helper")
    parser.add_argument("-d", "--debug", action="store_true", help="Target the Debug build")
    parser.add_argument("-r", "--release", action="store_true", help="Target the Release build")
    parser.add_argument("-p", "--print", action="store_true", help="Print results")

    args = parser.parse_args()

    if not (args.debug or args.release):
        print("> No build specified. Use '-h' or '--help' for usage information.")
        return

    print("")

    build_dirs = []

    if args.debug:
        build_dirs.append(Paths.DEBUG)

    if args.release:
        build_dirs.append(Paths.RELEASE)

    for build_dir in build_dirs:
        if not build_dir.exists():
            print(f"Build directory '{build_dir}' does not exist ...")
            continue

        print(f"============ Running tests ({build_dir.name}) ============")
        run_tests(build_dir)

        if args.print:
            print(f"\n============ Printing results ({build_dir.name}) ============\n")
            print_test_results(build_dir)

        print("")

# ---------------------------------------------------------------------------

if __name__ == "__main__":
    main()
