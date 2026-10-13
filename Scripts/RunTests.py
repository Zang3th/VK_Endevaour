#!/usr/bin/env python3

import os
import xml.etree.ElementTree as ET
from collections import defaultdict
from pathlib import Path
from ProjectDefines import Output, add_config_args, banner, create_parser, dump, error, info, run, run_script, selected_configs

# ---------------------------------------------------------------------------

TEST_EXECUTABLE = "EngineTests"
TEST_RESULTS_XML = "test-results.xml"

# ---------------------------------------------------------------------------

def executable_path(build_dir: Path) -> Path:
    return build_dir / "Tests" / (TEST_EXECUTABLE + (".exe" if os.name == "nt" else ""))

# ---------------------------------------------------------------------------

# Kept inside the build directory: it is a per-configuration artifact, gets wiped by a clean rebuild
def results_path(build_dir: Path) -> Path:
    return build_dir / "Tests" / TEST_RESULTS_XML

# ---------------------------------------------------------------------------

def run_tests(build_dir: Path, verbose: bool) -> int:
    executable = executable_path(build_dir)

    if not executable.exists():
        error(f"Test executable '{executable}' does not exist.")
        return 1

    results = results_path(build_dir)

    if results.exists():
        results.unlink()

    cmd = [
        executable,
        "--reporters=xml",
        f"--out={results}",
    ]

    # The engine logs to stdout and would bury the report, so its output is captured instead of inherited.
    # In verbose mode the streams are inherited, which keeps the output live and colored.
    process = run(cmd, cwd=build_dir, output=Output.SHOW if verbose else Output.CAPTURE)

    if not results.exists():
        if not verbose:
            dump(process.stdout)
            dump(process.stderr)

        error(f"Test run produced no report (exit code {process.returncode}).")
        return 1

    info(f"Wrote report to '{results}'.")

    if process.returncode != 0:
        error(f"Tests failed (exit code {process.returncode}).")

    return process.returncode

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

    try:
        root = ET.parse(results).getroot()
    except ET.ParseError:
        error(f"Test report '{results}' is incomplete, the run probably crashed.")
        return

    grouped = collect_failures(root)

    for file in sorted(grouped.keys()):
        for line, name, expression, expanded in sorted(grouped[file]):
            info(f"{file}:{line}")
            info(f"  [{name}]")
            info(f"    {expression}")
            info(f"    -> {expanded}\n")

    if not grouped:
        info("Found no failing assertions.\n")

    skipped = [t.get("name") or "" for t in root.iter("TestCase") if t.get("skipped") == "true"]

    if skipped:
        info("Skipped:")
        for name in sorted(skipped):
            info(f"  {name}")
        info("")

    cases = root.find("OverallResultsTestCases")
    asserts = root.find("OverallResultsAsserts")

    if cases is not None:
        info(f"Test cases: {cases.get('successes')} passed, "
             f"{cases.get('failures')} failed, "
             f"{cases.get('skipped')} skipped")

    if asserts is not None:
        info(f"Assertions: {asserts.get('successes')} passed, "
             f"{asserts.get('failures')} failed")

# ---------------------------------------------------------------------------

def main() -> int:
    parser = create_parser("Run the engine tests")
    add_config_args(parser, "Run the tests of the {} build")
    parser.add_argument("-p", "--print", action="store_true", help="Print results")
    parser.add_argument("-v", "--verbose", action="store_true", help="Show the full engine output")

    args = parser.parse_args()
    exit_code = 0

    for config in selected_configs(parser, args):
        banner(f"Running tests ({config.name})")

        if not config.directory.exists():
            error(f"Build directory '{config.directory}' does not exist.")
            exit_code = 1
            continue

        if run_tests(config.directory, args.verbose) != 0:
            exit_code = 1

        if args.print and results_path(config.directory).exists():
            banner(f"Printing results ({config.name})")
            print_test_results(config.directory)

    return exit_code

# ---------------------------------------------------------------------------

if __name__ == "__main__":
    run_script(main)
