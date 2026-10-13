"""Track declared platform imports used by the prepared full translation.

This is a source inventory, not a linker or ABI acceptance test. Only production
bridge sources count; test witnesses and the unsafe original wrappers do not.
"""
from pathlib import Path
import json
import re

ROOT = Path(__file__).resolve().parent.parent


def code(path):
    text = path.read_text(encoding="utf-8")
    return re.sub(r'/\*[\s\S]*?\*/|//[^\n]*|"(?:\\.|[^"\\])*"', " ", text)


def inventory():
    prepared = ROOT / ".build/fullmapped"
    header = (prepared / "DATA.h").read_text()
    imports = set(re.findall(r'extern "C" int32_t (\w+)\(', header))
    references = {name: [] for name in imports}
    for path in sorted(prepared.glob("Methods_*.cpp")):
        for name in set(re.findall(r"\b(\w+)\s*\(", code(path))) & imports:
            references[name].append(path.name)
    # Explicit declarations are excluded by requiring the definition's brace.
    bridges = sorted((ROOT / "src").glob("*.cpp"))
    bridges += [ROOT / ".build/renderer/GlideBridge.cpp"]
    definitions = {}
    for path in bridges:
        if not path.exists():
            raise FileNotFoundError(f"Prepare the renderer first: {path}")
        text = path.read_text()
        for name in re.findall(r'extern "C" int32_t (\w+)\([^;{}]*\)\s*\{', text):
            if name in definitions:
                raise ValueError(f"Duplicate production import: {name}")
            definitions[name] = str(path.relative_to(ROOT)).replace("\\", "/")
    used = sorted(name for name, paths in references.items() if paths)
    result = {
        "scope": "lexical full translated Methods call sites and production bridge definitions",
        "proves_linkage": False,
        "proves_ABI_correctness": False,
        "declared_imports": len(imports),
        "referenced_imports": len(used),
        "production_definitions": sum(name in definitions for name in used),
        "missing_definitions": [name for name in used if name not in definitions],
        "unavailable_windows_service_exports": [name for name in used
            if definitions.get(name)=="src/UnavailableWindowsServices.cpp"
            or name=="SetThreadPriority_wrap"],
        "imports": [{"name": name, "call_site_units": references[name],
                     "production_source": definitions.get(name)} for name in used],
    }
    destination = ROOT / ".build/platform-imports.json"
    destination.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    print(f"{len(used)} referenced platform imports; "
          f"{result['production_definitions']} production definitions; "
          f"{len(result['missing_definitions'])} remaining. Details: {destination}")
    return result


if __name__ == "__main__":
    inventory()
