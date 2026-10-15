"""Prepare a bounded experimental engine subset with explicit load/store boundaries.

No low mappings, native pointer truncation, alias caches or instruction interpreter.
Source extraction preserves translated function bodies; unsupported syntax fails.
This is deliberately separate from the full compilation-only candidate target.
"""
from pathlib import Path
import re
import shutil
import json
from prepare_engine import prepare, ROOT

TYPES = {"64f": "double", "32f": "float", "64i": "int64_t", "32i": "int32_t", "16i": "int16_t", "8i": "int8_t"}

def closing(text, start, left="(", right=")"):
    depth = 0
    for i in range(start, len(text)):
        if text[i] == left: depth += 1
        elif text[i] == right:
            depth -= 1
            if not depth: return i
    raise ValueError("Unbalanced generated expression")

def rewrite_stores(text):
    # Only single-line statements in the reviewed generated dialect are supported.
    pattern = re.compile(r"(?m)^([ \t]*)to(64f|32f|64i|32i|16i|8i)\(")
    for match in reversed(list(pattern.finditer(text))):
        begin = match.end() - 1
        end = closing(text, begin)
        rest = text[end+1:]
        assignment = re.match(r"\s*=\s*([^\n;]+);", rest)
        if not assignment:
            continue
        address = text[begin+1:end]
        value = assignment.group(1)
        suffix = match.group(2)
        replacement = f"{match.group(1)}guest_store<{TYPES[suffix]}>({address}, static_cast<{TYPES[suffix]}>({value}));"
        text = text[:match.start()] + replacement + text[end+1+assignment.end():]
    # A read/write operand must be handled with a typed local, then committed.
    mutable = r"add|sub|adc|sbb|inc|dec|and_|or_|xor_|neg|shl|shr|sar|rol|ror|imul32|imul16|shrd|bsr|pop32|pop16|fstp|fst|fistp|fstcw|fnstsw|setnbe"
    pattern = re.compile(r"(?m)^([ \t]*)(" + mutable + r")\(to(64f|32f|64i|32i|16i|8i)\(")
    for match in reversed(list(pattern.finditer(text))):
        inner = match.end() - 1
        inner_end = closing(text, inner)
        outer = text.index("(", match.start())
        outer_end = closing(text, outer)
        address = text[inner+1:inner_end]
        args = text[inner_end+1:outer_end]
        typename = TYPES[match.group(3)]
        # The one-operand signed multiply reads memory and writes EDX:EAX.
        # Its by-value helper leaves the memory operand untouched. Two/three
        # operand forms still modify their first operand and need a store.
        if match.group(2) == "imul32" and not args.strip():
            replacement = (f"{match.group(1)}imul32(guest_load<{typename}>({address}))")
            text = text[:match.start()] + replacement + text[outer_end+1:]
            continue
        replacement = (f"{match.group(1)}{{ const auto mapped_address = {address}; "
                       f"auto mapped_value = guest_load<{typename}>(mapped_address); "
                       f"{match.group(2)}(mapped_value{args}); "
                       f"guest_store<{typename}>(mapped_address, mapped_value); }}")
        text = text[:match.start()] + replacement + text[outer_end+1:]
    code = re.sub(r"/\*[\s\S]*?\*/|//[^\n]*", "", text)
    remaining = re.search(r"to(?:64f|32f|64i|32i|16i|8i)\([^;\n]*\)\s*=(?!=)", code)
    if remaining:
        raise ValueError("Unconverted memory assignment: " + remaining.group())
    return text

def replace_function(text, name, replacement):
    match = re.search(r"(?:\ttemplate[^\n]*\n)?\t(?:static )?FnInl\([^\n]+\) " + re.escape(name) + r"\(", text)
    if not match: raise ValueError(f"Missing helper {name}")
    begin = text.index("{", match.end())
    end = closing(text, begin, "{", "}")
    return text[:match.start()] + replacement + text[end+1:]

def prepare_mapped():
    source = prepare() / "Cpp"
    dest = ROOT / ".build/mappedengine"
    dest.mkdir(parents=True, exist_ok=True)
    for name in ["config.h", "ByteSwapPortable.h", "FPU.h", "CPU.h", "Application.h", "Game.h", "BSS.h", "DATA.h", "MemoryTranslate.h"]:
        shutil.copyfile(source / name, dest / name)
    cpu = (dest / "CPU.h").read_text()
    cpu = cpu.replace('#include "FPU.h"', '#include "FPU.h"\n#include "GuestAccess.h"')
    cpu = cpu.replace("(int32_t)(intptr_t)stack", "(int32_t)nfs_guest_address(stack)")
    cpu = cpu.replace('#include "GuestAccess.h"', '#include "GuestAccess.h"\n#include <new>')
    cpu = cpu.replace("stack = (uint8_t *)malloc32(stack_size);", "stack = (uint8_t *)malloc32(stack_size);\n\t\tif (!stack) throw std::bad_alloc();")
    (dest / "CPU.h").write_text(cpu)
    app = (dest / "Application.h").read_text()
    app = app.replace('#include "MemoryTranslate.h"', '')
    for suffix, typename in TYPES.items():
        replacement = f"\ttemplate<typename T> static FnInl({typename}) to{suffix}(T address) {{ return guest_load<{typename}>(address); }}"
        app = replace_function(app, "to" + suffix, replacement)
    # The original PPC reference cache and ARM low-address helper are no longer
    # used by value-based mapped operands. Remove those pointer-recovery paths.
    begin = app.index('#if defined(__powerpc64__) || defined(__PPC64__)')
    end = app.index('\ttemplate<typename T> static FnInl(double) to64f', begin)
    app = app[:begin] + app[end:]
    for name, body in {
        "movsb": "guest_store<int8_t>(edi, guest_load<int8_t>(esi)); esi += 1; edi += 1;",
        "movsw": "guest_store<int16_t>(edi, guest_load<int16_t>(esi)); esi += 2; edi += 2;",
        "movsd": "guest_store<int32_t>(edi, guest_load<int32_t>(esi)); esi += 4; edi += 4;",
        "stosb": "guest_store<int8_t>(edi, al); edi += 1;",
        "stosd": "guest_store<int32_t>(edi, eax); edi += 4;",
        "scasb": "const int8_t val = al - guest_load<int8_t>(edi); set_ZF_SF_flags(val); edi += 1;",
        "cmpsb": "cmp(guest_load<int8_t>(esi), guest_load<int8_t>(edi)); esi += 1; edi += 1;",
    }.items():
        app = replace_function(app, name, f"\tFnInl(void) {name}() {{ {body} }}")
    app = app.replace("(int32_t)(intptr_t)val", "static_cast<int32_t>(guest_address(val))")
    app = rewrite_stores(app)
    app = replace_function(app, "call", "\ttemplate<typename T> FnInl(void) call(T id) { const auto fn = nfs_lookup_guest_function(static_cast<uint32_t>(id)); esp -= 4; fn(this); esp += 4; }")
    # Match machine-register semantics for signed arithmetic/shift operations.
    app = app.replace("val1 <<= val2;", "UNSIGNED_REF(val1) <<= val2;")
    (dest / "Application.h").write_text(app)
    bodies = {}
    origins = {}
    for path in sorted(source.glob("Methods_*.cpp")):
        text = path.read_text()
        for match in re.finditer(r"Fn\(void\) Game::(\w+)\(\)\s*\{", text):
            end = closing(text, match.end()-1, "{", "}")
            bodies[match.group(1)] = text[match.start():end+1]
            origins[match.group(1)] = path.name
    chosen = {"_sub_4217F0", "_sub_477850", "_sub_401080", "_strcmp_", "_strncpy_", "_memset_", "_toupper_", "___CHP", "_sub_49CAD4", "_sprintf_", "_vsprintf_", "_sub_489E0C", "_sub_489E20", "_sub_489F48", "_sub_489F60", "_sub_489A88", "_StartAddress", "_sub_497244", "_sub_4972BC", "_sub_488718"}
    pending = list(chosen)
    while pending:
        name = pending.pop()
        for callee in re.findall(r"\b(_\w+)\(\)", bodies[name]):
            if callee in bodies and callee not in chosen:
                chosen.add(callee); pending.append(callee)
    prepare_initializers(source, dest)
    function_symbols=set(re.findall(r'game_id_(\w+) =',(dest/'FunctionIds.h').read_text()))
    out = '#include "Game.h"\n#include "BSS.h"\n#include "DATA.h"\n#include "FunctionIds.h"\n'
    for name in sorted(chosen):
        body = rewrite_stores(bodies[name])
        def function_cast(match):
            symbol=match.group(1)
            if symbol not in function_symbols:raise ValueError(f'Unknown selected callback symbol: {symbol}')
            return f'static_cast<int32_t>(game_id_{symbol})'
        body=re.sub(r'\(int32_t\)\(intptr_t\)\s*(sub_\w+)',function_cast,body)
        body=re.sub(r'push32\((sub_\w+)\)',lambda m:'push32('+function_cast(m)+')',body)
        # Explicit addresses of packed native DATA/BSS fields must be encoded;
        # integral register LEAs below still retain their 32-bit arithmetic.
        body=re.sub(r'\(int32_t\)\(intptr_t\)\s*((?:(?:dword|byte|off|word)_\w+|a[A-Z_]\w*))',
                    lambda m:f'static_cast<int32_t>(guest_address({m.group(1)}))',body)
        # These selected pointer casts are integral LEA expressions, verified below.
        body = body.replace("(int32_t)(intptr_t)", "(int32_t)")
        if re.search(r"\(int32_t\)\s*(?:a[A-Z_]|(?:sub|dword|byte|off|word)_)", body):
            raise ValueError(f"Unexpected native-symbol pointer cast in {name}")
        out += f"\n// Source: {origins[name]}, pinned candidate\n" + body + "\n"
    (dest / "SelectedMethods.cpp").write_text(out)
    print("Mapped real engine routines: " + ", ".join(sorted(chosen)))
    return dest


def prepare_initializers(source, dest):
    # Packed structure macros address real fields; compute byte offsets from the
    # actual host layout, never the original hexadecimal names. Code IDs occupy
    # a separate namespace and retain no native pointer bits.
    symbols = {}
    field_widths = {}
    for layout, base in [("DATA", 0x00400000), ("BSS", 0x02000000)]:
        header = (source / (layout + ".h")).read_text()
        for symbol in re.findall(r"^#define (\w+) ", header, re.M):
            symbols[symbol] = (layout.lower(), base)
        for fields, symbol in re.findall(r"struct packed \{([^}]+)\} (\w+);", header):
            widths = []
            for typename, count in re.findall(r"(int(?:8|16|32|64)_t|float|double) \w+(?:\[(\d+)\])?;", fields):
                width = {"int8_t": 1, "int16_t": 2, "int32_t": 4, "int64_t": 8, "float": 4, "double": 8}[typename]
                widths.extend([width] * (int(count) if count else 1))
            field_widths[symbol] = widths
    functions = re.findall(r"Fn\(void\) (\w+)\(Game \*self\)", (source / "Stubs.h").read_text())
    ids = {name: 0xf0000000 + index * 4 for index, name in enumerate(sorted(functions))}
    memory = (source / "Memory.cpp").read_text().replace('#include "Stubs.cpp"', '')
    pattern = re.compile(r"\(int32_t\)\(intptr_t\)\(")
    relocations = []
    checks = '#include "Game.h"\n#include "DATA.h"\n#include "BSS.h"\n#include "GuestAccess.h"\n#include <cstdio>\nextern "C" bool verify_original_relocations() {\n'
    for match in reversed(list(pattern.finditer(memory))):
        start = match.end()-1
        end = closing(memory, start)
        expression = memory[start+1:end]
        symbol = re.match(r"\w+", expression)
        if not symbol: raise ValueError(f"Unsupported initializer {expression}")
        symbol = symbol.group()
        if expression == symbol and symbol in ids:
            replacement = f"static_cast<int32_t>(0x{ids[symbol]:08x}u)"
            kind = "function_id"
        elif symbol in symbols and re.fullmatch(r"\w+(?:\+(?:0x[0-9A-Fa-f]+|\d+))?", expression):
            layout, base = symbols[symbol]
            replacement = (f"static_cast<int32_t>(0x{base:08x}u + "
                           f"(reinterpret_cast<uintptr_t>({expression}) - reinterpret_cast<uintptr_t>(&_{layout})))")
            kind = "data_offset"
        else:
            raise ValueError(f"Unclassified pointer initializer: {expression}")
        begin_line = memory.rfind("\n", 0, match.start())+1
        end_line = memory.find("\n", end)
        label = re.search(r"/\*\s*(\w+)\s*\*/", memory[end:end_line])
        if not label: raise ValueError("Initializer field has no label")
        field = label.group(1)
        index = memory[begin_line:match.start()].count(",")
        widths = field_widths[field]
        if widths[index] != 4: raise ValueError("Pointer initializer is not 32 bits")
        offset = sum(widths[:index])
        expected = f"0x{ids[symbol]:08x}u" if kind == "function_id" else f"guest_address({expression})"
        checks += (f' if (guest_load<uint32_t>({field}+{offset}) != {expected}) '
                   f'{{ std::fprintf(stderr, "Relocation mismatch: {field}+{offset}\\n"); return false; }}\n')
        relocations.append({"expression": expression, "kind": kind, "field": field, "byte_offset": offset})
        memory = memory[:match.start()] + replacement + memory[end+1:]
    if "(int32_t)(intptr_t)" in memory: raise ValueError("Unconverted DATA initializer")
    (dest / "MappedMemory.cpp").write_text(memory)
    (dest / "RelocationChecks.cpp").write_text(checks + 'return true;\n}\n')
    (dest / "FunctionIds.h").write_text('#pragma once\n' + ''.join(
        f'constexpr uint32_t game_id_{name} = 0x{value:08x}u;\n' for name,value in ids.items()))
    (dest / "relocations.json").write_text(json.dumps(relocations, indent=2) + "\n")
    print(f"Relocated {len(relocations)} packed DATA pointers; generated {len(ids)} function IDs")

if __name__ == "__main__": prepare_mapped()
