# SPDX-License-Identifier: MIT
"""Bounded native ARM64 TLS component timing; no app, game, assets or upload."""
import argparse
import hashlib
import itertools
import json
import os
from pathlib import Path
import platform
import re
import shutil
import statistics
import struct
import subprocess
import sys
import tempfile
import time

from apple_stop_slot_probe import ROOT, VARIANTS, materialize

# Declare before collecting any measurements. Passing permits further research,
# never promotion into an app or an FPS/battery claim by itself.
GATE = {"minimum_accessor_cpu_improvement_percent": 10,
        "maximum_transform9_rep8_cpu_regression_percent": 5,
        "rounds": 12, "order": "all six variant permutations twice",
        "kernel_order": "rotate accessor/transform9/rep8 by round modulo3",
        "native_execution_budget_seconds": 90,
        "per_invocation_timeout_seconds": 10,
        "unchanged_retry": False}
KERNELS = ("accessor", "transform9", "rep8")
ITERATIONS = {"accessor": 2000000, "transform9": 100000, "rep8": 100000}
MAX_OUTPUT = 8192


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def checked(args, *, timeout=120):
    result = subprocess.run(args, capture_output=True, timeout=timeout, check=False)
    if result.returncode or len(result.stdout) > 1024 * 1024 or len(result.stderr) > 1024 * 1024:
        # Compiler diagnostics may contain home/temp paths. Do not echo them.
        raise ValueError("Probe command failed or exceeded output bound")
    return result.stdout.decode("utf-8", errors="strict").strip()


def native_arm64_macho(path):
    if path.is_symlink() or not path.is_file() or not 32 <= path.stat().st_size <= 8 * 1024 * 1024:
        raise ValueError("Invalid native executable storage")
    with path.open("rb") as stream:
        header = stream.read(32)
    magic, cpu, _, filetype = struct.unpack_from("<IIII", header)
    if (magic, cpu, filetype) != (0xFEEDFACF, 0x0100000C, 2):
        raise ValueError("Expected thin native ARM64 Mach-O executable")
    if not path.stat().st_mode & 0o111:
        raise ValueError("Native executable mode missing")


def configuration(local):
    if local:
        if sys.platform != "linux":
            raise ValueError("Local safety execution requires Linux")
        clang = shutil.which("clang++")
        if not clang:
            raise ValueError("Existing clang++ unavailable")
        return {"compiler": clang,
                "flags": ["-std=c++14", "-O2", "-g", "-fwrapv", "-fno-strict-aliasing",
                          "-pthread", "-fsanitize=address,undefined", "-fno-omit-frame-pointer"],
                "identity": {"platform": "linux-safety-only", "architecture": platform.machine(),
                             "compiler": checked([clang, "--version"]).splitlines()[0],
                             "apple_performance_evidence": False}}
    if sys.platform != "darwin" or platform.machine() != "arm64":
        raise ValueError("Runtime comparison requires native Darwin ARM64")
    xcode = checked(["xcodebuild", "-version"])
    if not xcode.startswith("Xcode 16.4\n"):
        raise ValueError("Expected pinned Xcode16.4")
    clang = checked(["xcrun", "--sdk", "macosx", "--find", "clang++"])
    sdk = checked(["xcrun", "--sdk", "macosx", "--show-sdk-path"])
    return {"compiler": clang,
            "flags": ["-target", "arm64-apple-macos11.0", "-std=c++14", "-O3", "-DNDEBUG", "-fwrapv",
                      "-fno-strict-aliasing", "-isysroot", sdk, "-pthread"],
            "identity": {"platform": "macos", "architecture": "arm64", "xcode": xcode,
                         "sdk": "macosx", "sdk_version": checked(["xcrun", "--sdk", "macosx", "--show-sdk-version"]),
                         "compiler": checked([clang, "--version"]).splitlines()[0],
                         "apple_performance_evidence": "native macOS component only; not iOS app or device"}}


def compile_variant(config, folder, variant, compile_deadline):
    executable = folder / (variant + "-runtime")
    inputs = (ROOT / "tests/apple_stop_slot_runtime_probe.cpp",
              ROOT / "tests/apple_stop_slot_assembly_probe.cpp")
    common = [config["compiler"], *config["flags"],
              "-ffile-prefix-map=" + str(ROOT) + "=/source",
              "-fdebug-prefix-map=" + str(ROOT) + "=/source",
              "-ffile-prefix-map=" + str(folder) + "=/probe",
              "-fdebug-prefix-map=" + str(folder) + "=/probe",
              "-I" + str(folder / variant)]
    objects = []
    for index, source in enumerate(inputs):
        remaining = min(120, compile_deadline - time.monotonic())
        if remaining <= 0:
            raise ValueError("Compile budget exhausted")
        obj = folder / (variant + "-" + str(index) + ".o")
        # Independent translation units, no LTO: caller cannot fold probe calls,
        # and accessor/transform/REP TU cannot fold the runtime resolver checks.
        print("TLS_STAGE compile " + variant + " unit=" + str(index), file=sys.stderr, flush=True)
        checked([*common, "-c", str(source), "-o", str(obj)], timeout=remaining)
        objects.append(obj)
    remaining = min(120, compile_deadline - time.monotonic())
    if remaining <= 0:
        raise ValueError("Compile budget exhausted")
    checked([config["compiler"], *config["flags"], *map(str, objects), "-o", str(executable)], timeout=remaining)
    return executable


def parse_rows(output, measurement):
    if len(output) > MAX_OUTPUT:
        raise ValueError("Native output exceeds bound")
    lines = output.splitlines()
    if len(lines) != (3 if measurement else 1):
        raise ValueError("Unexpected native row count")
    def invalid(_):
        raise ValueError("Nonfinite native number")
    rows = [json.loads(line, parse_constant=invalid) for line in lines]
    if any(type(row) is not dict for row in rows):
        raise ValueError("Unexpected native output")
    if not measurement:
        row = rows[0]
        if (row.get("type") != "verification" or row.get("separate_probe_tu") is not True
                or row.get("actual_thread_tls") is not True
                or any(type(row.get(key)) is not int or row[key] <= 0
                       for key in ("checks", "resolver_calls", "memory_checksum"))):
            raise ValueError("Incomplete runtime correctness evidence")
    else:
        if {r.get("kernel") for r in rows} != set(KERNELS):
            raise ValueError("Missing or duplicate native kernels")
        for row in rows:
            if (row.get("type") != "measurement" or row.get("iterations") != ITERATIONS[row["kernel"]]
                    or any(type(row.get(key)) is not int or not 0 < row[key] < 2**64
                           for key in ("cpu_ns", "wall_ns", "checksum"))):
                raise ValueError("Invalid bounded kernel measurement")
    return rows


def run_native(executable, args, deadline, measurement):
    remaining = min(10, deadline - time.monotonic())
    if remaining <= 0:
        raise ValueError("Native execution budget exhausted")
    output = checked([str(executable), *args], timeout=remaining)
    return parse_rows(output, measurement)


def summarize(measurements):
    medians = {}
    for variant in VARIANTS:
        medians[variant] = {}
        for kernel in KERNELS:
            samples = [r for r in measurements if r["variant"] == variant and r["kernel"] == kernel]
            if len(samples) != GATE["rounds"]:
                raise ValueError("Incomplete balanced timing")
            medians[variant][kernel] = {
                "cpu_ns_per_invocation": statistics.median(r["cpu_ns"] / r["iterations"] for r in samples),
                "wall_ns_per_invocation": statistics.median(r["wall_ns"] / r["iterations"] for r in samples)}
    decisions = {}
    for variant in ("candidate", "unified"):
        changes = {k: 100 * (medians["old"][k]["cpu_ns_per_invocation"]
                            - medians[variant][k]["cpu_ns_per_invocation"])
                   / medians["old"][k]["cpu_ns_per_invocation"] for k in KERNELS}
        decisions[variant] = {
            "cpu_improvement_percent": changes,
            "component_gate_passed": changes["accessor"] >= 10
                                     and all(changes[k] >= -5 for k in ("transform9", "rep8")),
            "scope": "component decision only; no app promotion, FPS or energy claim"}
    return medians, decisions


def execute(local=False):
    config = configuration(local)
    print("TLS_STAGE configuration verified", file=sys.stderr, flush=True)
    variants = ("old", "unified") if local else VARIANTS
    source_names = ("tests/apple_stop_slot_runtime_probe.cpp", "tests/apple_stop_slot_assembly_probe.cpp",
                    "scripts/apple_stop_slot_runtime_probe.py", "scripts/apple_stop_slot_probe.py")
    source_commit = os.environ.get("GITHUB_SHA", "local")
    if source_commit != "local" and not re.fullmatch(r"[0-9a-f]{40}", source_commit):
        raise ValueError("Invalid source commit identity")
    report = {"schema": "nfsiise-native-stop-slot-runtime-v1", "local_safety_only": local,
              "scope": "Synthetic checked copies and stop checkpoints; no game/assets/GPU/iPad/FPS/energy claim",
              "predeclared_gate": GATE, "configuration": config["identity"],
              "source_commit": source_commit,
              "source_sha256": {name: digest(ROOT / name) for name in source_names},
              "build_options": {"language": "c++14", "optimization": "O2 plus address/undefined sanitizers" if local else "O3",
                                "wrap_signed_overflow": True, "strict_aliasing": False,
                                "translation_units": 2, "separate_compile_and_link": True,
                                "native_target": "host-safety-only" if local else "arm64-apple-macos11.0"},
              "variants": {}, "measurements": []}
    with tempfile.TemporaryDirectory(prefix="nfs-native-tls-") as temporary:
        folder = Path(temporary)
        closures = materialize(ROOT, folder)
        compile_deadline = time.monotonic() + 360
        executables = {}
        for variant in variants:
            binary = compile_variant(config, folder, variant, compile_deadline)
            if not local:
                print("TLS_STAGE native-identity " + variant, file=sys.stderr, flush=True)
                native_arm64_macho(binary)
                symbols = set(checked(["nm", "-gj", str(binary)]).splitlines())
                if not {"_nfs_probe_accessor", "_nfs_probe_transform", "_nfs_probe_rep8"} <= symbols:
                    raise ValueError("Separate probe entry symbols missing")
            executables[variant] = binary
            report["variants"][variant] = {"binary_sha256": digest(binary), "binary_bytes": binary.stat().st_size,
                                           "header_closure_sha256": closures[variant]}
        execution_start = time.monotonic()
        deadline = execution_start + 90
        expected_proof = None
        expected_checksums = {}
        for variant in variants:
            print("TLS_STAGE verify " + variant, file=sys.stderr, flush=True)
            proof = run_native(executables[variant], ["--verify"], deadline, False)[0]
            if expected_proof is None:
                expected_proof = proof
            elif proof != expected_proof:
                raise ValueError("Variant correctness results differ")
            report["variants"][variant]["verification"] = proof
            warmup = run_native(executables[variant], ["--measure", "0"], deadline, True)
            for row in warmup:
                expected = expected_checksums.setdefault(row["kernel"], row["checksum"])
                if row["checksum"] != expected:
                    raise ValueError("Variant kernel checksum differs")
        report["discarded_warmup_invocations"] = len(variants)
        if not local:
            orders = list(itertools.permutations(VARIANTS)) * 2
            for index, order in enumerate(orders):
                print("TLS_STAGE measured-round " + str(index), file=sys.stderr, flush=True)
                for variant in order:
                    rows = run_native(executables[variant], ["--measure", str(index % 3)], deadline, True)
                    for row in rows:
                        if row["checksum"] != expected_checksums[row["kernel"]]:
                            raise ValueError("Measured kernel checksum differs")
                        report["measurements"].append({"round": index, "variant": variant, **row})
            report["medians"], report["decisions"] = summarize(report["measurements"])
        report["native_execution_seconds"] = round(time.monotonic() - execution_start, 3)
        report["all_variants_verified"] = True
        report["separate_translation_units"] = True
        report["link_time_optimization"] = False
        report["artifacts_uploaded"] = False
        report["app_built"] = False
    return report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--local-safety-check", action="store_true",
                        help="Linux OLD+UNIFIED sanitizer oracle only; no Apple performance comparison")
    args = parser.parse_args()
    try:
        print(json.dumps(execute(args.local_safety_check), sort_keys=True, indent=2, allow_nan=False))
    except (OSError, ValueError, subprocess.SubprocessError, UnicodeError):
        # Do not disclose environment/compiler/temp paths in public job logs.
        parser.exit(1, "Native TLS probe failed an input, compile, correctness, architecture or time bound.\n")


if __name__ == "__main__":
    main()
