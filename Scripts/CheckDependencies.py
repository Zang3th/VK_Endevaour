#!/usr/bin/env python3

import ctypes
import importlib.metadata
import importlib.util
import os
import re
import shutil
import subprocess
from ProjectDefines import banner, create_parser, info, run_script, status_tag

# ---------------------------------------------------------------------------

REQUIRED_VULKAN_VERSION = (1, 4)

# ---------------------------------------------------------------------------
# Utility functions
# ---------------------------------------------------------------------------

def report(name: str, passed: bool, details: list[tuple[str, str]]) -> bool:
    info(f"{status_tag(passed)} {name}")

    width = max((len(key) for key, _ in details), default=0) + 1
    for key, value in details:
        info(f"       {key + ':':<{width}} {value}")

    return passed

def capture(cmd: list[str]) -> subprocess.CompletedProcess | None:
    try:
        return subprocess.run(cmd, capture_output=True, text=True, encoding="utf-8", errors="replace")
    except OSError:
        return None

def first_line(text: str) -> str:
    lines = [line.strip() for line in text.splitlines() if line.strip()]
    return next((line for line in lines if "version" in line.lower()), lines[0] if lines else "")

def parse_version(text: str) -> tuple[int, ...]:
    match = re.match(r"(\d+)\.(\d+)", text)
    return tuple(map(int, match.groups())) if match else ()

# ---------------------------------------------------------------------------
# Tools
# ---------------------------------------------------------------------------

def check_executable(name: str) -> bool:
    exe = shutil.which(name)

    if not exe:
        return report(name, False, [("Path", "not found")])

    result = capture([exe, "--version"])
    version = first_line(result.stdout or result.stderr) if result else "unknown"

    return report(name, True, [("Path", exe), ("Version", version)])

# ---------------------------------------------------------------------------

def check_python_module(name: str, distribution: str) -> bool:
    spec = importlib.util.find_spec(name)

    if not spec:
        return report(distribution, False, [("Module", f"'{name}' not found, install with 'pip install {distribution}'")])

    return report(distribution, True, [("Path", str(spec.origin)), ("Version", importlib.metadata.version(distribution))])

# ---------------------------------------------------------------------------
# Vulkan SDK
# ---------------------------------------------------------------------------

def check_vk_loader() -> bool:
    # Windows
    if os.name == "nt":
        candidates = ["vulkan-1.dll"]

    # Linux
    else:
        candidates = [
            "libvulkan.so.1",
            "libvulkan.so",
            "/usr/lib/libvulkan.so.1",
            "/usr/lib64/libvulkan.so.1",
            "/usr/local/lib/libvulkan.so.1",
        ]

    for lib in candidates:
        try:
            ctypes.CDLL(lib)
            return report("Vulkan loader", True, [("Library", lib)])
        except OSError:
            continue

    return report("Vulkan loader", False, [("Library", "not found")])

# ---------------------------------------------------------------------------

def check_vk_info() -> bool:
    exe = shutil.which("vulkaninfo")

    if not exe:
        report("vulkaninfo", False, [("Path", "not found")])
        info("")
        return report("Vulkan 1.4", False, [("Version", "unknown, vulkaninfo is missing")])

    result = capture([exe, "--summary"])

    if not result or result.returncode != 0:
        reason = (first_line(result.stderr) or f"exit code {result.returncode}") if result else "could not be started"
        report("vulkaninfo", False, [("Path", exe), ("Error", reason)])
        info("")
        return report("Vulkan 1.4", False, [("Version", "unknown, vulkaninfo failed")])

    instance_version = ""
    gpus: list[dict[str, str]] = []

    re_instance_version = re.compile(r"Vulkan Instance Version:\s+(.*)")
    re_device_start = re.compile(r"^GPU\d+:")
    re_field = re.compile(r"^([a-zA-Z]+)\s*=\s*(.*)$")

    for line in result.stdout.splitlines():
        line = line.strip()

        m = re_instance_version.search(line)
        if m:
            instance_version = m.group(1)
            continue

        if re_device_start.match(line):
            gpus.append({})
            continue

        m = re_field.match(line)
        if gpus and m:
            key, value = m.groups()
            gpus[-1][key] = value

    details = [("Path", exe), ("Instance Version", instance_version)]
    for index, gpu in enumerate(gpus):
        details.append((f"GPU{index}", f"{gpu.get('deviceName')} ({gpu.get('deviceType')}), "
                                       f"API {gpu.get('apiVersion')}, driver {gpu.get('driverVersion')}"))

    vk_info_passed = report("vulkaninfo", True, details)
    info("")

    required = ".".join(map(str, REQUIRED_VULKAN_VERSION))
    instance_supported = parse_version(instance_version) >= REQUIRED_VULKAN_VERSION
    supported_gpus = [gpu.get("deviceName", "") for gpu in gpus
                      if parse_version(gpu.get("apiVersion", "")) >= REQUIRED_VULKAN_VERSION]

    version_passed = report(f"Vulkan {required}", instance_supported and bool(supported_gpus), [
        ("Instance", f"{instance_version or 'unknown'} ({'ok' if instance_supported else f'requires {required} or newer'})"),
        ("Devices", ", ".join(supported_gpus) if supported_gpus else f"no GPU supports {required} or newer"),
    ])

    return vk_info_passed and version_passed

# ---------------------------------------------------------------------------

def main() -> int:
    parser = create_parser("Check the build requirements")
    parser.parse_args()

    sections = [
        ("Building", [
            lambda: check_executable("cmake"),
            lambda: check_executable("clang++"),
            lambda: check_executable("ninja"),
        ]),
        ("Formatting and analysis", [
            lambda: check_executable("clang-format"),
            lambda: check_executable("clang-tidy"),
            lambda: check_python_module("yaml", "PyYAML"),
        ]),
        ("Vulkan", [
            check_vk_loader,
            check_vk_info,
            lambda: check_executable("glslc"),
        ]),
    ]

    results = []

    for title, checks in sections:
        banner(title)

        for index, check in enumerate(checks):
            if index:
                info("")
            results.append(check())

    return 0 if all(results) else 1

# ---------------------------------------------------------------------------

if __name__ == "__main__":
    run_script(main)
