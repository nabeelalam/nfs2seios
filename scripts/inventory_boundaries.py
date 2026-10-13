"""Inventory the pinned translation's boundaries; this is a lexical audit aid.

Detailed locations stay in .build; a compact, reproducible summary is evidence.
Counts identify work, and do not establish ABI correctness or complete coverage.
"""
import json
import re
from collections import Counter
from prepare_engine import ROOT, PIN, prepare


def inventory():
    source = prepare()
    patterns = {
        "narrow_pointer_cast": re.compile(r"\((?:u?int32_t|int)\)\s*\((?:u?intptr_t|void\s*\*)\)"),
        "memory_operand": re.compile(r"\bto(?:8i|16i|32i|64i|32f|64f)\("),
        "indirect_game_call": re.compile(r"\bcall\("),
        "wrapper_declaration": re.compile(r'extern "C"[^\n;]+;'),
        "native_pointer_cast": re.compile(r"\([^()\n]+\*\)\s*(?:\(uintptr_t\)|\(intptr_t\)|\(uint32_t\)|[a-zA-Z_]\w*)"),
    }
    findings = {name: [] for name in patterns}
    files = sorted(p for p in source.rglob("*") if p.suffix in (".h", ".c", ".cpp")
                   and p.name not in ("NFS2SE.cpp", "SwapInit.h"))
    for path in files:
        # Monolithic Methods duplicate split units; SwapInit is an unused target.
        for number, line in enumerate(path.read_text(errors="strict").splitlines(), 1):
            for name, pattern in patterns.items():
                matches = list(pattern.finditer(line))
                if matches:
                    findings[name].append({"file": path.relative_to(source).as_posix(),
                                           "line": number, "count": len(matches),
                                           "expressions": [m.group(0) for m in matches]})
    memory = (source / "Cpp/Memory.cpp").read_text()
    pointer_tables = []
    for number, line in enumerate(memory.splitlines(), 1):
        if "(int32_t)(intptr_t)" in line:
            label = re.search(r"/\*\s*(\w+)\s*\*/", line)
            pointer_tables.append({"line": number, "field": label.group(1) if label else None,
                                   "count": line.count("(int32_t)(intptr_t)")})
    report = {"source_commit": PIN, "limitations": "Lexical matches require semantic review; comments may match. No ABI adapters are implied.",
              "files_scanned": len(files), "findings": findings,
              "pointer_initializer_fields": pointer_tables}
    detailed = ROOT / ".build/boundary-inventory.json"
    detailed.write_text(json.dumps(report, indent=2) + "\n")
    summary = {"source_commit": PIN, "files_scanned": len(files), "limitations": report["limitations"],
               "excluded_duplicate_or_unused": ["NFS2SE.cpp", "SwapInit.h"],
               "categories": {name: {"occurrences": sum(x["count"] for x in rows),
                                        "files": dict(sorted(Counter(x["file"] for x in rows).items()))}
                              for name, rows in findings.items()},
               "pointer_initializer_fields": len(pointer_tables),
               "pointer_initializer_casts": sum(x["count"] for x in pointer_tables)}
    (ROOT / "evidence/boundary-inventory.json").write_text(json.dumps(summary, indent=2) + "\n")
    print(json.dumps({name: data["occurrences"] for name, data in summary["categories"].items()}))
    return report


if __name__ == "__main__":
    inventory()
