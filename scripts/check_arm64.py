"""Cross-compile every candidate engine TU; save logs and verify ELF architecture."""
import json
import os
from pathlib import Path
import subprocess
import struct
import sys
from prepare_engine import prepare, ROOT

engine = prepare()
zig = ROOT / ".tools/python/ziglang/zig.exe"
output = ROOT / ".build/arm64"
output.mkdir(parents=True, exist_ok=True)
env = os.environ.copy()
env["ZIG_LOCAL_CACHE_DIR"] = str(ROOT / ".build/zig-local")
env["ZIG_GLOBAL_CACHE_DIR"] = str(ROOT / ".build/zig-global")
sources = sorted(p for p in (engine / "Cpp").glob("*.cpp") if p.name not in {"NFS2SE.cpp", "Stubs.cpp"})
results = []
for source in sources:
    obj = output / (source.stem + ".o")
    args = [str(zig), "c++", "-target", "aarch64-linux-musl", "-std=c++14", "-fno-rtti",
            "-fno-exceptions", "-fno-strict-aliasing", "-Wno-c++11-narrowing", "-c", str(source), "-o", str(obj)]
    run = subprocess.run(args, env=env, capture_output=True, text=True)
    (output / (source.stem + ".log")).write_text(run.stdout + run.stderr, encoding="utf-8")
    valid = False
    if run.returncode == 0:
        header = obj.read_bytes()[:64]
        valid = header[:6] == b"\x7fELF\x02\x01" and struct.unpack_from("<H", header, 18)[0] == 183
    results.append({"source": source.name, "exit_code": run.returncode, "elf64_aarch64": valid})
    print(f"{source.name}: {'PASS' if valid else 'FAIL'}", flush=True)
report = {"target": "aarch64-linux-musl", "compiler": "Zig 0.14.1 / Clang", "linked": False, "executed": False, "results": results}
(ROOT / "evidence/arm64-compilation.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
sys.exit(0 if all(r["elf64_aarch64"] for r in results) else 1)
