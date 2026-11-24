# SPDX-License-Identifier: MIT
"""Bounded synthetic owned-chunk correctness and Apple SDK compilation probe.

No game assets, gameplay timing, energy measurements, IPA or uploads.
"""
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
FIXTURE = ROOT / "tests/fixtures/owned_chunk"


def checked(args, deadline, timeout=90):
    remaining = deadline - time.monotonic()
    if remaining <= 0:
        raise ValueError("Probe deadline exhausted")
    result = subprocess.run(args, capture_output=True, timeout=min(timeout, remaining))
    if result.returncode or max(len(result.stdout), len(result.stderr)) > 1048576:
        raise ValueError("Probe command failed or exceeded output bound")
    return result.stdout


def execute(local=False):
    deadline = time.monotonic() + 240
    arch = platform.machine()
    flags = ["-std=c++14", "-O1", "-g", "-fsanitize=address,undefined",
             "-fno-sanitize-recover=all", "-fno-omit-frame-pointer", "-pthread"]
    if local:
        if sys.platform != "linux":
            raise ValueError("Local safety check requires Linux")
        compiler = shutil.which("clang++")
        if not compiler:
            raise ValueError("Existing compiler unavailable")
    else:
        if sys.platform != "darwin" or arch != "arm64":
            raise ValueError("Native Apple ARM host required")
        if checked(["xcodebuild", "-version"], deadline).splitlines()[0] != b"Xcode 16.4":
            raise ValueError("Pinned Xcode required")
        compiler = checked(["xcrun", "--sdk", "macosx", "--find", "clang++"], deadline).decode().strip()
        sdk = checked(["xcrun", "--sdk", "macosx", "--show-sdk-path"], deadline).decode().strip()
        flags += ["-arch", "arm64", "-isysroot", sdk]
    sources = [Path(__file__).resolve(), ROOT / "src/EngineMemory.cpp", ROOT / "src/HeapArena.h",
               ROOT / "src/EngineMemory.h", ROOT / "src/GameMemory.h",
               ROOT / "src/GameTrace.cpp", ROOT / "src/GameTrace.h",
               ROOT / "src/CodecOperations.h", ROOT / "tests/owned_chunk_oracle.cpp",
               *sorted(FIXTURE.rglob("*.h")), *sorted(FIXTURE.glob("*.cpp"))]
    report = {"schema": 1, "scope": "synthetic span correctness and compilation only",
              "local_safety_only": local, "architecture": arch,
              "ios_executed": False, "app_built": False, "performance_measured": False,
              "source_sha256": {p.relative_to(ROOT).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest() for p in sources}}
    with tempfile.TemporaryDirectory(prefix="nfs-owned-chunk-") as folder:
        results = {}
        for variant in ("baseline", "candidate", "boundaries"):
            candidate = variant != "baseline"
            includes = [FIXTURE, ROOT / "src"] if candidate else [ROOT / "src", FIXTURE]
            includes += [FIXTURE / "synthetic"]
            inc = ["-I" + str(p) for p in includes]
            if variant == "boundaries":
                units = [FIXTURE / "boundaries.cpp"]
            else:
                units = [ROOT / "tests/owned_chunk_oracle.cpp",
                         FIXTURE / "EngineMemory.cpp" if candidate else ROOT / "src/EngineMemory.cpp",
                         ROOT / "src/GameTrace.cpp"]
            binary = Path(folder) / variant
            checked([compiler, *flags, *inc, *map(str, units), "-o", str(binary)], deadline)
            if not local:
                magic, cpu, _, kind = struct.unpack_from("<IIII", binary.read_bytes())
                if (magic, cpu, kind) != (0xfeedfacf, 0x0100000c, 2):
                    raise ValueError("Native ARM executable identity mismatch")
            results[variant] = json.loads(checked([str(binary)], deadline, timeout=30))
        if results["baseline"] != results["candidate"]:
            raise ValueError("Control/candidate oracle mismatch")
        oracle = results["candidate"]
        if (oracle["checks"], oracle["dataBytes"], oracle["bssBytes"],
            oracle["concurrentOwnedChunkReads"], oracle["concurrentLiveSpanReads"],
            oracle["concurrentStaticSpanReads"], oracle["oracleDigest"],
            oracle["bindingTransitionsChecked"]) != (12109, 69037, 680644, 80000, 80000, 160000, 8306997119277202445, True):
            raise ValueError("Oracle coverage mismatch")
        if results["boundaries"]["boundaryCases"] != 42 or not results["boundaries"]["passed"]:
            raise ValueError("Boundary coverage mismatch")
        report["sanitized_results"] = results
        if not local:
            if checked(["xcrun", "--sdk", "iphoneos", "--show-sdk-version"], deadline).strip() != b"18.5":
                raise ValueError("Pinned iOS SDK required")
            ios_sdk = checked(["xcrun", "--sdk", "iphoneos", "--show-sdk-path"], deadline).decode().strip()
            for variant, source in (("baseline", ROOT / "src/EngineMemory.cpp"), ("candidate", FIXTURE / "EngineMemory.cpp")):
                obj = Path(folder) / (variant + "-ios.o")
                checked([compiler, "-std=c++14", "-O2", "-target", "arm64-apple-ios16.0",
                         "-isysroot", ios_sdk, "-I" + str(FIXTURE / "synthetic"),
                         "-I" + str(ROOT / "src"), "-c", str(source), "-o", str(obj)], deadline)
                magic, cpu, _, kind = struct.unpack_from("<IIII", obj.read_bytes())
                if (magic, cpu, kind) != (0xfeedfacf, 0x0100000c, 1):
                    raise ValueError("iOS ARM object identity mismatch")
            report["ios_arm64_control_and_candidate_objects_compiled"] = True
    report["complete"] = True
    return report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--local-safety-check", action="store_true")
    args = parser.parse_args()
    try:
        print(json.dumps(execute(args.local_safety_check), sort_keys=True, indent=2))
    except (OSError, ValueError, KeyError, struct.error, subprocess.SubprocessError):
        parser.exit(1, "Owned-chunk probe failed its platform, coverage, compile, runtime or time bound.\n")


if __name__ == "__main__":
    main()
