# SPDX-License-Identifier: MIT
"""Prepare an explicit baseline-default transform comparison from reviewed sources.

Normal preparation is unchanged. Outputs remain in a new ignored build directory.
No original method text or game assets are embedded in this script.
"""
import argparse
import hashlib
import json
import re
from pathlib import Path
import shutil
from prepare_mapped_engine import closing
from apple_frame_cache_probe import sources as cache_sources

ROOT = Path(__file__).resolve().parents[1]
SIGNATURE = "Fn(void) Game::_sub_480330()"
BASELINE_BODY_SHA = "35af4085892ef4e50a5137d101cb8599bd9bb1b702d11e3774e4b19b1b89f0b0"
CACHED_BODY_SHA = "c71cda02499caf7b698957f164686aab1da8650762b15f94daf0211828d11ca3"
GAME_HEADER_SHA = "171582c776772674ca59221733de19be822d86015a522d5057b92c30a065daba"
CACHE_MANIFEST_SHA = "103a38b4299166fcb7bf80c750175a3c34ffab61c57acc30a3248a97a45f58be"
BEGIN = "\t{ // checked copy transaction begin\n\tnfs::GuestCopyTransaction guest_transaction;\n"
END = "\t} // checked copy transaction end\n"
PREFIX = '#include "CachedFrameTransaction.h"\n#include "TransformTrialPrototype.h"\n'
PROTOTYPE = b'#pragma once\nextern "C" bool nfs_transform_trial_cached() noexcept;\n'


def digest(data):
    return hashlib.sha256(data).hexdigest()


def transform_unit(data):
    if b"\r" in data:
        raise ValueError("Trial preparation requires LF generated source")
    text = data.decode("utf-8")
    if text.count(SIGNATURE) != 1 or "nfs_transform_trial_cached" in text:
        raise ValueError("Expected one unmodified transform method")
    start = text.index("{", text.index(SIGNATURE))
    end = closing(text, start, "{", "}") + 1
    baseline = text[start:end]
    if digest(baseline.encode()) != BASELINE_BODY_SHA:
        raise ValueError("Transform method differs from reviewed baseline")
    if baseline.count(BEGIN) != 9 or baseline.count(END) != 9:
        raise ValueError("Incomplete checked transform scope set")
    first = list(re.finditer(re.escape(BEGIN), baseline))[3].start()
    last = baseline.rindex(END) + len(END)
    region = baseline[first:last]
    if region.count(BEGIN) != 6 or region.count(END) != 6 or region.count("guest_store<") != 1:
        raise ValueError("Unexpected standalone transform store set")
    region = region.replace(BEGIN, "\t{ // bounded forward transform region\n\tnfs::CachedFrameTransaction guest_transaction{uint32_t(esp)};\n", 1)
    region = region.replace(BEGIN, "\tnfs::check_worker_stop(); // retained original constructor sample\n")
    region = region.replace(END, "", 5).replace(END, "\t} // release before next vertex iteration\n", 1)
    region = region.replace("guest_store<", "guest_transaction.store<", 1)
    cached = baseline[:first] + region + baseline[last:]
    if digest(cached.encode()) != CACHED_BODY_SHA:
        raise ValueError("Transform candidate differs from tested body")
    wrapper = ("{\n    const auto nfs_original_transform_body=[this]() -> void " + baseline
               + ";\n    const auto nfs_cached_transform_body=[this]() -> void " + cached
               + ";\n    if(nfs_transform_trial_cached())nfs_cached_transform_body();\n"
                 "    else nfs_original_transform_body();\n}")
    return (PREFIX + text[:start] + wrapper + text[end:]).encode()


def build_directory(path):
    path = Path(path).resolve()
    if not path.is_relative_to((ROOT / ".build").resolve()) or path == (ROOT / ".build").resolve():
        raise ValueError("Trial paths must be inside ignored project build storage")
    return path


def inventory(folder):
    found = {}
    for path in sorted(folder.rglob("*")):
        if path.is_symlink():
            raise ValueError("Trial input/output cannot contain symlinks")
        if path.is_file():
            name = path.relative_to(folder).as_posix()
            if ".git" in path.relative_to(folder).parts or path.stat().st_size > 256 * 1024 * 1024:
                raise ValueError("Unexpected trial source entry")
            found[name] = digest(path.read_bytes())
    if len(found) > 4096:
        raise ValueError("Trial source inventory exceeds bound")
    return found


def dependencies():
    manifest = ROOT / "tests/fixtures/frame_span_cache/source-provenance.json"
    if digest(manifest.read_bytes()) != CACHE_MANIFEST_SHA:
        raise ValueError("Native-validated cache closure manifest changed")
    pins = {row["file"]: row["sha256"] for row in cache_sources()}
    for name in ("src/TransformTrial.h", "tests/fixtures/transform_trial/registered_backend.cpp"):
        path = ROOT / name
        if path.is_symlink() or not path.is_file():
            raise ValueError("Invalid trial control source")
        pins[name] = digest(path.read_bytes())
    return pins


def prepare(engine_dir, output_dir):
    source, output = build_directory(engine_dir), build_directory(output_dir)
    if not source.is_dir() or output.exists() or output.is_relative_to(source) or source.is_relative_to(output):
        raise ValueError("Require existing source and separate new output")
    before = inventory(source)
    if before.get("Game.h") != GAME_HEADER_SHA or "transform-trial.json" in before:
        raise ValueError("Unexpected Game header or already instrumented sources")
    required = {"Game.h", "FullDispatch.cpp", "MappedMemory.cpp", "CPU.h", "FPU.h", "Application.h", "DATA.h", "BSS.h", "FunctionIds.h"}
    required.update("Methods_%02d.cpp" % number for number in range(1, 16))
    if not required.issubset(before):
        raise ValueError("Incomplete prepared engine source set")
    pins = dependencies()
    transformed = transform_unit((source / "Methods_09.cpp").read_bytes())
    for path in source.glob("*"):
        if path.is_file() and path.suffix in {".h", ".cpp"} and b"classic-source-stages" in path.read_bytes():
            raise ValueError("Compiled engine references intermediate generation stages")
    needed = sum(path.stat().st_size for path in source.rglob("*")
                 if path.is_file() and "classic-source-stages" not in path.relative_to(source).parts)
    if shutil.disk_usage(output.parent).free < needed + 512 * 1024 * 1024:
        raise ValueError("Insufficient space for isolated trial sources")
    output.mkdir()
    engine, runtime = output / "engine", output / "runtime"
    shutil.copytree(source, engine, ignore=lambda directory, names:
                    {"classic-source-stages"} if Path(directory) == source else set())
    runtime.mkdir()
    (engine / "Methods_09.cpp").write_bytes(transformed)
    fixture = ROOT / "tests/fixtures/frame_span_cache"
    for name in ("CachedFrameTransaction.h", "FrameSpanCacheAbi.h"):
        shutil.copyfile(fixture / name, engine / name)
    (engine / "TransformTrialPrototype.h").write_bytes(PROTOTYPE)
    for name in ("EngineMemory.cpp", "HeapArena.h", "FrameSpanCacheAbi.h"):
        shutil.copyfile(fixture / name, runtime / name)
    shutil.copyfile(ROOT / "tests/fixtures/transform_trial/registered_backend.cpp", runtime / "registered_backend.cpp")
    pair = digest(json.dumps(dict(input=before, dependencies=pins, transformed=digest(transformed)), sort_keys=True).encode())
    (engine / "transform-trial.json").write_text(json.dumps(dict(schema=1, pair=pair)) + "\n", encoding="utf-8")
    files = inventory(output)
    if inventory(source) != before:
        raise ValueError("Original prepared sources changed")
    manifest = dict(schema=1, pair=pair, scope="Explicit baseline-default Game trial, not a qualified optimization",
                    files=files, dependencies=pins, originalFiles=before,
                    baselineBodySha256=BASELINE_BODY_SHA, cachedBodySha256=CACHED_BODY_SHA,
                    gameClassUnchanged=True, defaultBaseline=True, excludedPreparationDirectories=["classic-source-stages"])
    (runtime / "transform-trial.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    verify(engine, runtime)
    return manifest


def verify(engine_dir, runtime_dir):
    engine, runtime = build_directory(engine_dir), build_directory(runtime_dir)
    if engine.name != "engine" or runtime.name != "runtime" or engine.parent != runtime.parent:
        raise ValueError("Trial engine/runtime must come from the same prepared pair")
    path = runtime / "transform-trial.json"
    if path.is_symlink() or path.stat().st_size > 1024 * 1024:
        raise ValueError("Invalid trial manifest")
    manifest = json.loads(path.read_text())
    if (manifest.get("schema") != 1 or manifest.get("defaultBaseline") is not True
            or manifest.get("dependencies") != dependencies()
            or manifest.get("baselineBodySha256") != BASELINE_BODY_SHA
            or manifest.get("cachedBodySha256") != CACHED_BODY_SHA):
        raise ValueError("Trial manifest or dependency mismatch")
    pins = dependencies()
    actual = inventory(engine.parent)
    actual.pop("runtime/transform-trial.json", None)
    if actual != manifest["files"]:
        raise ValueError("Prepared trial files changed")
    if json.loads((engine / "transform-trial.json").read_text()) != dict(schema=1, pair=manifest["pair"]):
        raise ValueError("Prepared trial pair mismatch")
    expected_runtime = {
        "runtime/EngineMemory.cpp": pins["tests/fixtures/frame_span_cache/EngineMemory.cpp"],
        "runtime/HeapArena.h": pins["tests/fixtures/frame_span_cache/HeapArena.h"],
        "runtime/FrameSpanCacheAbi.h": pins["tests/fixtures/frame_span_cache/FrameSpanCacheAbi.h"],
        "runtime/registered_backend.cpp": pins["tests/fixtures/transform_trial/registered_backend.cpp"],
        "engine/CachedFrameTransaction.h": pins["tests/fixtures/frame_span_cache/CachedFrameTransaction.h"],
        "engine/FrameSpanCacheAbi.h": pins["tests/fixtures/frame_span_cache/FrameSpanCacheAbi.h"],
        "engine/TransformTrialPrototype.h": digest(PROTOTYPE),
        "engine/Game.h": GAME_HEADER_SHA,
    }
    if any(actual.get(name) != value for name, value in expected_runtime.items()):
        raise ValueError("Prepared backend differs from reviewed implementation")
    text = (engine / "Methods_09.cpp").read_bytes().decode("utf-8")
    if not text.startswith(PREFIX) or text.count(SIGNATURE) != 1:
        raise ValueError("Prepared transform dispatch missing")
    start = text.index("{", text.index(SIGNATURE))
    end = closing(text, start, "{", "}") + 1
    marker = "    const auto nfs_original_transform_body=[this]() -> void "
    if text.count(marker) != 1:
        raise ValueError("Prepared original transform body missing")
    body_start = text.index("{", text.index(marker) + len(marker))
    body_end = closing(text, body_start, "{", "}") + 1
    original = (text[len(PREFIX):start] + text[body_start:body_end] + text[end:]).encode()
    if digest(original) != manifest["originalFiles"].get("Methods_09.cpp"):
        raise ValueError("Original transform unit provenance mismatch")
    pair = digest(json.dumps(dict(input=manifest["originalFiles"], dependencies=pins,
                                 transformed=digest(text.encode())), sort_keys=True).encode())
    if pair != manifest["pair"]:
        raise ValueError("Trial pair provenance mismatch")
    if transform_unit(original) != text.encode():
        raise ValueError("Prepared dispatch differs from reviewed transformation")
    return manifest


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--engine-dir", type=Path, required=True)
    parser.add_argument("--output-dir", type=Path)
    parser.add_argument("--verify-runtime", type=Path)
    args = parser.parse_args()
    if (args.output_dir is None) == (args.verify_runtime is None):
        parser.error("Choose preparation output or runtime verification")
    result = verify(args.engine_dir, args.verify_runtime) if args.verify_runtime else prepare(args.engine_dir, args.output_dir)
    print(json.dumps(dict(passed=True, pair=result["pair"], files=len(result["files"]), defaultBaseline=True)))


if __name__ == "__main__":
    main()
