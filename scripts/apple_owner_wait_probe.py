# SPDX-License-Identifier: MIT
"""Bounded source-only owner-wait validation; never builds/uploads a game."""
import argparse
import hashlib
import json
from pathlib import Path
import platform
import shutil
import struct
import subprocess
import sys
import tempfile
import time

ROOT = Path(__file__).resolve().parents[1]

def checked(args, deadline, timeout=120):
    remaining = deadline - time.monotonic()
    if remaining <= 0:
        raise ValueError("Validation deadline exhausted")
    result = subprocess.run(args, capture_output=True, timeout=min(timeout, remaining))
    if result.returncode or max(len(result.stdout), len(result.stderr)) > 1048576:
        raise ValueError("Validation command failed or exceeded output bound")
    return result.stdout

def execute(local=False):
    deadline = time.monotonic() + 240
    if local:
        if sys.platform != "linux":
            raise ValueError("Local safety check requires Linux")
        compiler = shutil.which("clang++")
        if not compiler:
            raise ValueError("Existing compiler unavailable")
        flags = ["-std=c++14", "-O1", "-g", "-fsanitize=address,undefined",
                 "-fno-sanitize-recover=all", "-fno-omit-frame-pointer"]
        arch = platform.machine()
    else:
        if sys.platform != "darwin" or platform.machine() not in ("arm64", "x86_64"):
            raise ValueError("Actual Apple host required")
        if checked(["xcodebuild", "-version"], deadline).splitlines()[0] != b"Xcode 16.4":
            raise ValueError("Pinned Xcode required")
        compiler = checked(["xcrun", "--sdk", "macosx", "--find", "clang++"], deadline).decode().strip()
        sdk = checked(["xcrun", "--sdk", "macosx", "--show-sdk-path"], deadline).decode().strip()
        arch = platform.machine()
        flags = ["-std=c++14", "-O1", "-g", "-arch", arch, "-isysroot", sdk,
                 "-fsanitize=address,undefined", "-fno-sanitize-recover=all", "-fno-omit-frame-pointer"]
    report = {"schema": 1, "scope": "owner-wait correctness, not game performance or energy",
              "local_safety_only": local, "architecture": arch, "tests": [],
              "ios_executed": False, "app_built": False, "artifacts_uploaded": False}
    sources = ["tests/diagnostic_session_tests.cpp", "src/DiagnosticSession.h", "src/MainWaitTiming.h"]
    if not local:
        sources += ["tests/apple_main_thread_tests.cpp", "src/MainThreadBridge.cpp",
                    "src/MainThreadBridge.h", "src/WorkerControl.h", "src/AppleMainThread.mm"]
    report["source_sha256"] = {name: hashlib.sha256((ROOT / name).read_bytes()).hexdigest() for name in sources}
    with tempfile.TemporaryDirectory(prefix="nfs-owner-wait-") as folder:
        def compile_run(name, units, extra=()):
            binary = Path(folder) / name
            checked([compiler, *flags, "-I" + str(ROOT / "src"),
                     *[str(ROOT / unit) for unit in units], *extra, "-o", str(binary)], deadline)
            if not local:
                magic, cpu, _, kind = struct.unpack_from("<IIII", binary.read_bytes())
                expected = 0x0100000c if arch == "arm64" else 0x01000007
                if (magic, cpu, kind) != (0xfeedfacf, expected, 2):
                    raise ValueError("Native host executable identity mismatch")
            checked([str(binary)], deadline, timeout=30)
            report["tests"].append({"name": name, "exit_code": 0, "sanitizers": True})
        compile_run("diagnostic-session", ["tests/diagnostic_session_tests.cpp"])
        if not local:
            compile_run("actual-apple-owner", ["tests/apple_main_thread_tests.cpp",
                        "src/MainThreadBridge.cpp", "src/AppleMainThread.mm"],
                        ["-pthread", "-framework", "CoreFoundation"])
            ios_sdk = checked(["xcrun", "--sdk", "iphoneos", "--show-sdk-path"], deadline).decode().strip()
            checked([compiler, "-std=c++14", "-target", "arm64-apple-ios16.0", "-isysroot", ios_sdk,
                     "-I" + str(ROOT / "src"), "-fsyntax-only", str(ROOT / "src/AppleMainThread.mm")], deadline)
            report["ios_owner_source_syntax_passed"] = True
    report["complete"] = True
    return report

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--local-safety-check", action="store_true")
    args = parser.parse_args()
    try:
        print(json.dumps(execute(args.local_safety_check), sort_keys=True, indent=2))
    except (OSError, ValueError, struct.error, subprocess.SubprocessError):
        parser.exit(1, "Owner-wait validation failed its platform, compile, runtime or time bound.\n")

if __name__ == "__main__":
    main()
