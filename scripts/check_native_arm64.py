"""Compile current native engine/runtime to ARM64 Linux objects, not an iOS app."""
import json
import os
import struct
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
zig = ROOT / ".tools/python/ziglang/zig.exe"
out = ROOT / ".build/native-arm64"
out.mkdir(parents=True, exist_ok=True)
env = os.environ.copy()
env["ZIG_LOCAL_CACHE_DIR"] = str(ROOT / ".build/zig-local")
env["ZIG_GLOBAL_CACHE_DIR"] = str(ROOT / ".build/zig-global")
mapped = ROOT / ".build/fullmapped"
sources = sorted(mapped.glob("Methods_*.cpp")) + [mapped / "MappedMemory.cpp", mapped / "FullDispatch.cpp"]
sources += sorted(p for p in (ROOT / "src").glob("*.cpp") if p.name != "CoreProbe.cpp")
sources += [ROOT / "app/main.cpp", ROOT / "app/RunLog.cpp", ROOT / "app/desktop_assets.cpp",
            ROOT / ".build/renderer/GlideBridge.cpp", ROOT / ".build/renderer/Glide2x.c", ROOT / "src/RendererSurface.c"]
results = []
for index, source in enumerate(sources):
    if not source.is_file():
        raise RuntimeError(f"Prepare pinned mapped engine/renderer first: {source}")
    obj = out / f"{index:02}-{source.stem}.o"
    c = source.suffix == ".c"
    standard = "c11" if c else "c++17" if source.name in {"SandboxPaths.cpp", "AssetImport.cpp"} else "c++14"
    args = [str(zig), "cc" if c else "c++", "-target", "aarch64-linux-musl", f"-std={standard}",
            "-fwrapv", "-fno-strict-aliasing", "-Wno-c++11-narrowing", "-DGLES2", "-DNFS_CPP", "-DNFS_SDL_RUNTIME",
            "-DSDL_USE_BUILTIN_OPENGL_DEFINITIONS"]
    for include in [ROOT / "src", ROOT / "app", mapped, ROOT / ".build/renderer", ROOT / ".build/vendor/SDL/include"]:
        args += ["-I", str(include)]
    run = subprocess.run(args + ["-c", str(source), "-o", str(obj)], env=env, capture_output=True, text=True)
    (out / f"{index:02}-{source.stem}.log").write_text(run.stdout + run.stderr, encoding="utf-8")
    header = obj.read_bytes()[:64] if run.returncode == 0 else b""
    valid = len(header) >= 20 and header[:6] == b"\x7fELF\x02\x01" and struct.unpack_from("<H", header, 18)[0] == 183
    results.append({"source": source.relative_to(ROOT).as_posix(), "exit_code": run.returncode, "elf64_little_endian_aarch64": valid})
    print(f"{index+1}/{len(sources)} {source.name}: {'PASS' if valid else 'FAIL'}", flush=True)
report = {"source_commit": subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=ROOT, text=True).strip(),
          "target": "aarch64-linux-musl", "compiler": "workspace Zig 0.14.1/Clang", "linked": False,
          "executed": False, "apple_sdk_compilation": False, "results": results}
(out / "results.json").write_text(json.dumps(report, indent=2)+"\n", encoding="utf-8")
raise SystemExit(0 if all(item["elf64_little_endian_aarch64"] for item in results) else 1)
