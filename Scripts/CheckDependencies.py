#!/usr/bin/env python3

import os
import ctypes
import shutil
import subprocess
import re
from pathlib import Path
from dataclasses import dataclass

# ---------------------------------------------------------------------------

@dataclass
class ExecutableInfo:
    path: str
    version: str

# ---------------------------------------------------------------------------
# Vulkan SDK
# ---------------------------------------------------------------------------

def check_vk_loader():
    print(f"[VK_LOADER] ", end="")

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
            print("✅")
            print(f"  Lib: {lib}\n")
            return
        except OSError:
            continue

    print("❌\n")

# ---------------------------------------------------------------------------

def check_vk_info():
    print("[VK_INFO] ", end="")

    exe = shutil.which("vulkaninfo")

    if not exe:
        print("❌\n")
        return

    output = subprocess.check_output(
        ["vulkaninfo", "--summary"], stderr=subprocess.STDOUT, text=True
    )

    print("✅")

    instance_version = ""
    gpu = {}

    re_instance_version = re.compile(r"Vulkan Instance Version:\s+(.*)")
    re_device_start = re.compile(r"^GPU\d+:")
    re_field = re.compile(r"^\s*([a-zA-Z]+)\s*=\s*(.*)$")

    in_gpu = False

    for line in output.splitlines():
        line = line.strip()

        # Instance version
        m = re_instance_version.search(line)
        if m:
            instance_version = m.group(1)
            continue

        # GPU start
        if re_device_start.match(line):
            in_gpu = True
            continue

        # GPU fields
        if in_gpu:
            m = re_field.match(line)
            if m:
                key, value = m.groups()
                gpu[key] = value

                if all(k in gpu for k in ("deviceName", "deviceType", "driverVersion")):
                    break

    print(f"  Instance Version: {instance_version}")
    print(f"  Device Name: {gpu.get('deviceName')}")
    print(f"  Device Type: {gpu.get('deviceType')}")
    print(f"  Driver Version: {gpu.get('driverVersion')}\n")

# ---------------------------------------------------------------------------
# Utility functions
# ---------------------------------------------------------------------------

def find_executable(name) -> ExecutableInfo | None:
    exe = shutil.which(name)

    if exe:
        exePath = Path(exe)
        version_raw = subprocess.check_output([exe, "--version"], text=True)
        version = version_raw.splitlines()[0]

        return ExecutableInfo(path=str(exePath), version=version)

    return None

# ---------------------------------------------------------------------------

def check_existence(name: str):
    print(f"[{name.upper()}] ", end="")
    exe = find_executable(name)

    if not exe:
        print("❌\n")
        return

    print("✅")
    print(f"  Path: {exe.path}")
    print(f"  Version: {exe.version}\n")

# ---------------------------------------------------------------------------

def main():
    print("\n====== Building ======\n")
    check_existence("cmake")
    check_existence("clang++")
    check_existence("ninja")

    print("====== Vulkan ======\n")
    check_vk_loader()
    check_vk_info()
    check_existence("glslc")

# ---------------------------------------------------------------------------

if __name__ == "__main__":
    main()
