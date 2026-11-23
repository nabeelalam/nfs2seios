# SPDX-License-Identifier: MIT
"""Bounded OLD/COLD stop-path component experiment; never runs an iOS binary."""
import argparse
import json
import os
from pathlib import Path
import re
import shutil
import statistics
import subprocess
import struct
import sys
import tempfile
import time

import apple_stop_slot_probe as assembly
import apple_stop_slot_runtime_probe as runtime

ROOT = assembly.ROOT
VARIANTS = ('old', 'cold')
MODES = ('normal', 'critical', 'deferrals')
COLD_FILE = 'tests/fixtures/cold_stop_path/WorkerControl.h'
COLD_SHA = '087cf104b49896b6e4f7f66d990bd067b8c60d763e668cb69005549bc81c9e7f'
HELPER = '__ZN3nfs24worker_stop_not_deferredEv'
PROBES = ('_nfs_probe_accessor', '_nfs_probe_transform', '_nfs_probe_rep8')
GATE = dict(rounds=12, minimum_accessor_cpu_improvement_percent=10,
            maximum_transform9_rep8_cpu_regression_percent=5,
            native_execution_budget_seconds=90, per_invocation_timeout_seconds=10,
            compile_budget_seconds=360, unchanged_retry=False)
MAX_TEXT = 2 * 1024 * 1024
MAX_FUNCTION = 768
EXPECTED_PROOF = dict(type='verification', checks=126, resolver_calls=583,
                      memory_checksum=5252414852042320239,
                      separate_probe_tu=True, actual_thread_tls=True)


def materialize(root, folder):
    # Reuse the frozen historical closure, without altering its three variants.
    closures = assembly.materialize(root, folder)
    source = root / COLD_FILE
    if source.is_symlink() or not source.is_file() or runtime.digest(source) != COLD_SHA:
        raise ValueError('Cold fixture identity changed')
    cold = folder / 'cold'
    cold.mkdir()
    for name in assembly.CLOSURE:
        shutil.copyfile(folder / 'old' / name, cold / name)
    shutil.copyfile(source, cold / 'WorkerControl.h')
    result = {'old': closures['old'], 'cold': {
        name: runtime.digest(cold / name) for name in assembly.CLOSURE}}
    if [name for name in assembly.CLOSURE if result['old'][name] != result['cold'][name]] != ['WorkerControl.h']:
        raise ValueError('Unexpected cold closure difference')
    return result


def remaining(deadline):
    value = min(120, deadline - time.monotonic())
    if value <= 0:
        raise ValueError('Compile budget exhausted')
    return value


def sdk_number(value):
    if not re.fullmatch(r"[0-9]{1,3}(?:\.[0-9]{1,3}){0,2}", value):
        raise ValueError("Invalid SDK version")
    pieces = [int(v) for v in value.split(".")]
    pieces += [0] * (3 - len(pieces))
    if pieces[0] > 65535 or any(v > 255 for v in pieces[1:]):raise ValueError("Invalid SDK version")
    return (pieces[0] << 16) | (pieces[1] << 8) | pieces[2]


def macho_platform(path, platform, minimum, expected_sdk=None):
    runtime.native_arm64_macho(path)
    data = path.read_bytes()
    if len(data) > 8 * 1024 * 1024:
        raise ValueError('Mach-O exceeds bound')
    commands, size = struct.unpack_from('<II', data, 16)
    if not 0 < commands <= 256 or size > len(data) - 32:
        raise ValueError('Invalid Mach-O command bounds')
    cursor, limit, builds = 32, 32 + size, []
    for _ in range(commands):
        if cursor + 8 > limit:
            raise ValueError('Truncated Mach-O command')
        kind, length = struct.unpack_from('<II', data, cursor)
        if length < 8 or length % 8 or length > limit - cursor:
            raise ValueError('Invalid Mach-O command length')
        if kind == 0x32:
            if length < 24:
                raise ValueError('Truncated platform command')
            target, version, sdk, tools = struct.unpack_from('<IIII', data, cursor + 8)
            if tools > 32 or length != 24 + 8 * tools:
                raise ValueError('Invalid build tools')
            builds.append((target, version, sdk))
        cursor += length
    if cursor != limit or len(builds) != 1 or builds[0][0:2] != (platform, minimum):
        raise ValueError('Wrong Mach-O platform or deployment target')
    if expected_sdk is not None and builds[0][2] != expected_sdk:
        raise ValueError('Wrong Mach-O SDK version')
    return dict(architecture='arm64', filetype='MH_EXECUTE', platform=platform,
                minimum=minimum, sdk=builds[0][2])


def symbols(text):
    if len(text) > MAX_TEXT:
        raise ValueError('Symbol output exceeds bound')
    result = {}
    for line in text.splitlines():
        match = re.fullmatch(r'\s*([0-9a-fA-F]{8,16})\s+[A-Za-z]\s+(\S+)\s*', line)
        if match:
            result[int(match[1], 16)] = match[2]
    return result


def parse_functions(text, linked=False, symbol_addresses=None):
    """Recognize full bounded selected functions; no raw path headers are emitted."""
    if len(text) > MAX_TEXT:
        raise ValueError('Disassembly exceeds bound')
    functions, current = {}, None
    for original in text.splitlines():
        if len(original) > 2048:
            raise ValueError('Disassembly line exceeds bound')
        line = original.strip()
        label = re.fullmatch(r'([A-Za-z_.$][\w.$]*):(?:\s*(?:;|//).*)?', line)
        if label:
            name = label[1]
            if name.startswith('_') or name.startswith('OUTLINED_FUNCTION_'):
                current = dict(name=name, code=[], labels={})
                functions[name] = current
            if current is not None:
                current['labels'][name] = len(current['code'])
            continue
        if line.startswith('.cfi_endproc'):
            current = None
            continue
        if current is None or not line or line.startswith(('.', ';', '//')):
            continue
        address = None
        if linked:
            match = re.fullmatch(r'([0-9a-fA-F]{8,16})\s+(?:(?:[0-9a-fA-F]{8})\s+)?([a-z][a-z0-9.]*)\s*(.*)', line)
            if not match:
                raise ValueError('Unknown linked instruction format')
            address, op, args = int(match[1], 16), match[2], match[3]
        else:
            match = re.fullmatch(r'([a-z][a-z0-9.]*)\s*(.*)', line)
            if not match:
                raise ValueError('Unknown assembly instruction format')
            op, args = match[1], match[2]
        # Keep comments internally for symbolic call resolution, not in previews.
        current['code'].append(dict(op=op, args=args, address=address))
    for name in PROBES:
        if name not in functions or not functions[name]['code']:
            raise ValueError('Missing full probe body')
    selected = [f for name, f in functions.items() if name in PROBES or 'worker_stop_not_deferred' in name or 'OUTLINED_FUNCTION_' in name]
    if any(len(f['code']) > MAX_FUNCTION for f in selected):
        raise ValueError('Selected body exceeds bound; no truncated proof')
    return functions


def operand(text):
    return text.split(';', 1)[0].split('//', 1)[0].strip()


def call_target(instruction, addresses):
    value = operand(instruction['args']).split(',')[-1].strip()
    if re.fullmatch(r'(?:0x)?[0-9a-fA-F]{8,16}', value):
        target = addresses.get(int(value.removeprefix('0x'), 16))
        if target:
            return target
        stub = re.search(r'symbol stub for:\s*([_A-Za-z][\w.$]*)', instruction['args'])
        if stub:
            return stub[1]
        raise ValueError('Unresolved direct call target')
    if not re.fullmatch(r'[_A-Za-z.$][\w.$]*', value):
        raise ValueError('Unknown direct call target')
    return value


EXCEPTION_TERMINALS = frozenset(('___cxa_throw', '__cxa_throw', '__Unwind_Resume'))


def graph(function, symbol_addresses=None):
    code, labels = function['code'], function['labels']
    addresses = {row['address']: i for i, row in enumerate(code) if row['address'] is not None}
    def target(value):
        value = value.strip()
        if value in labels:
            result = labels[value]
        elif re.fullmatch(r'(?:0x)?[0-9a-fA-F]{8,16}', value):
            result = addresses.get(int(value.removeprefix('0x'), 16), -1)
        else:
            result = -1
        if not 0 <= result < len(code):
            raise ValueError('Branch leaves recognized function')
        return result
    edges = {}
    for i, row in enumerate(code):
        op, args = row['op'], operand(row['args'])
        fall = [i + 1] if i + 1 < len(code) else []
        if op in ('ret', 'retab', 'brk', 'udf'):
            edges[i] = []
        elif op == 'b':
            edges[i] = [target(args)]
        elif op in ('cbz', 'cbnz', 'tbz', 'tbnz') or op.startswith('b.'):
            edges[i] = [target(args.split(',')[-1]), *fall]
        elif op == 'bl':
            # Resolve the actual operand through the supplied symbol table.
            # Recorded resolved_call_target fields never authorize an edge.
            callee = call_target(row, symbol_addresses or {})
            edges[i] = [] if callee in EXCEPTION_TERMINALS else fall
        elif op in ('br', 'braa', 'brab', 'blraa', 'blrab', 'svc', 'hlt', 'eret'):
            raise ValueError('Unsupported indirect control flow')
        else:
            edges[i] = fall
    return edges


def reachable(edges, start, stop=None):
    seen, pending = set(), [start]
    while pending:
        at = pending.pop()
        if at in seen:
            continue
        seen.add(at)
        if at != stop:
            pending.extend(edges[at])
    return seen


SLOT_SYMBOLS = {
    'current': '__ZZN3nfs22current_worker_controlEvE5value',
    'critical': '__ZZN3nfs20owned_critical_depthEvE5value',
    'deferrals': '__ZZN3nfs21worker_stop_deferralsEvE5value'}


def slot_kind(name):
    for kind, symbol in SLOT_SYMBOLS.items():
        if name == symbol or re.fullmatch(re.escape(symbol) + r'\.llvm\.[0-9]+', name):
            return kind
    return None


def macho_tls_symbols(path):
    """Resolve TLVP slots from bounded Mach-O indirect symbols, not code comments."""
    data = path.read_bytes()
    if not 32 <= len(data) <= 8 * 1024 * 1024:
        raise ValueError('Invalid Mach-O symbol storage')
    count, size = struct.unpack_from('<II', data, 16)
    if not 0 < count <= 256 or size > len(data) - 32:
        raise ValueError('Invalid Mach-O symbol command bounds')
    symtab = dysymtab = None
    sections, cursor = [], 32
    for _ in range(count):
        if cursor + 8 > 32 + size:
            raise ValueError('Truncated symbol command')
        kind, length = struct.unpack_from('<II', data, cursor)
        if length < 8 or length > 32 + size - cursor:
            raise ValueError('Invalid symbol command')
        if kind == 2:
            if length != 24 or symtab is not None:raise ValueError('Invalid symbol table')
            symtab = struct.unpack_from('<IIII', data, cursor + 8)
        elif kind == 0xb:
            if length != 80 or dysymtab is not None:raise ValueError('Invalid indirect symbol table')
            dysymtab = struct.unpack_from('<II', data, cursor + 56)
        elif kind == 0x19:
            if length < 72:raise ValueError('Truncated segment')
            nsects = struct.unpack_from('<I', data, cursor + 64)[0]
            if nsects > 128 or length != 72 + 80 * nsects:raise ValueError('Invalid section bounds')
            for k in range(nsects):
                at = cursor + 72 + 80 * k
                flags, first = struct.unpack_from('<II', data, at + 64)
                if flags & 255 in (6, 0x14):
                    address, bytes_ = struct.unpack_from('<QQ', data, at + 32)
                    if bytes_ % 8 or bytes_ // 8 > 4096:raise ValueError('Invalid pointer section')
                    sections.append((address, bytes_ // 8, first))
        cursor += length
    if cursor != 32 + size or symtab is None or dysymtab is None:
        raise ValueError('Missing symbol metadata')
    symoff, nsyms, stroff, strsize = symtab
    indirectoff, nindirect = dysymtab
    if (nsyms > 65536 or symoff > len(data) or 16 * nsyms > len(data) - symoff
            or strsize > 4 * 1024 * 1024 or stroff > len(data) or strsize > len(data) - stroff
            or nindirect > 65536 or indirectoff > len(data) or 4 * nindirect > len(data) - indirectoff):
        raise ValueError('Unbounded Mach-O symbol table')
    names, direct = [], {}
    for k in range(nsyms):
        at = symoff + 16 * k
        string, type_, _, _, value = struct.unpack_from('<IBBHQ', data, at)
        name = ''
        if 0 < string < strsize:
            end = data.find(b'\0', stroff + string, min(stroff + strsize, stroff + string + 512))
            if end >= 0:name = data[stroff + string:end].decode('ascii', errors='strict')
        names.append(name)
        if slot_kind(name) and type_ & 0xe == 0xe:
            if value in direct and direct[value] != name:raise ValueError('Ambiguous TLS symbol')
            direct[value] = name
    indirect = {}
    for address, entries, first in sections:
        if first > nindirect or entries > nindirect - first:raise ValueError('Invalid indirect section index')
        for k in range(entries):
            index = struct.unpack_from('<I', data, indirectoff + 4 * (first + k))[0]
            if index & 0xc0000000:continue  # LOCAL/ABS cannot establish identity.
            if index >= nsyms:raise ValueError('Invalid indirect symbol index')
            if slot_kind(names[index]):indirect[address + 8 * k] = names[index]
    return dict(direct=direct, indirect=indirect)


def dominators(edges):
    live = reachable(edges, 0)
    pred = {i: set() for i in live}
    for i in live:
        for j in edges[i]:pred[j].add(i)
    dom = {i: ({0} if i == 0 else set(live)) for i in live}
    changed = True
    while changed:
        changed = False
        for i in live - {0}:
            value = {i} | set.intersection(*(dom[j] for j in pred[i]))
            if value != dom[i]:dom[i] = value;changed = True
    return dom, pred


# A closed instruction/destination model. Unknown status/RMW/authenticated branch
# instructions never inherit a stale stop/control register tag.
CONDITIONS = {'eq','ne','cs','hs','cc','lo','mi','pl','vs','vc','hi','ls','ge','lt','gt','le'}
NO_DEST = {'str','stur','stp','stnp','cmp','cmn','tst','ccmp','ccmn','b','bl','blr','cbz','cbnz','tbz','tbnz','ret','retab','nop','bti','brk','udf'}
ONE_DEST = {'mov','movz','movk','movn','adr','adrp','add','adds','sub','subs','and','ands','orr','orn','eor','eon','bic','bics','mvn','neg','negs','lsl','lsr','asr','ror','ubfx','sbfx','ubfiz','sbfiz','bfi','bfxil','sxtb','sxth','sxtw','uxtb','uxth','uxtw','cset','csetm','csel','csinc','csinv','csneg','ldr','ldur','ldrb','ldurb','ldrh','ldurh','ldrsb','ldrsh','ldrsw','ldursw','ldar','ldarb','ldarh'}


def instruction_destinations(row):
    op, args = row['op'], operand(row['args'])
    if op.startswith('b.'):
        if op[2:] not in CONDITIONS:raise ValueError('Unknown branch condition')
        return []
    if op in NO_DEST:
        if op in ('str', 'stur', 'stp', 'stnp') and (']!' in args or re.search(r'\]\s*,', args)):
            base = re.search(r'\[(x[0-9]+|sp)', args)
            if not base:raise ValueError('Unsupported store writeback')
            return [base[1]]
        return []
    if op not in ONE_DEST and op != 'ldp':raise ValueError('Unsupported instruction semantics')
    if (']!' in args or re.search(r'\]\s*,', args)) and not re.search(r'\[sp(?:,|\])', args):
        raise ValueError('Unsupported non-stack load writeback')
    values = [v.strip() for v in args.split(',')]
    return values[:2] if op == 'ldp' else values[:1]


def tls_lookup(fn, at, linked, tls):
    code = fn['code']
    if at < 3 or code[at]['op'] != 'blr':raise ValueError('Unrecognized TLS lookup')
    page, pointer, function = code[at - 3:at]
    if page['op'] != 'adrp' or not operand(page['args']).startswith('x0,'):
        raise ValueError('TLS page materialization missing')
    if function['op'] != 'ldr' or not re.fullmatch(r'x\d+,\s*\[x0\]', operand(function['args'])):
        raise ValueError('TLS descriptor function load missing')
    if operand(function['args']).split(',')[0] != operand(code[at]['args']):
        raise ValueError('TLS indirect call target changed')
    if not linked:
        a = re.findall(r'([_A-Za-z][\w.$]*)@TLVPPAGE\b', operand(page['args']))
        b = re.findall(r'([_A-Za-z][\w.$]*)@TLVPPAGEOFF\b', operand(pointer['args']))
        if pointer['op'] != 'ldr' or len(a) != 1 or b != a or not slot_kind(a[0]):
            raise ValueError('Missing exact source TLS relocation')
        if not re.fullmatch(r'x0,\s*\[x0,\s*' + re.escape(a[0]) + r'@TLVPPAGEOFF\]', operand(pointer['args'])):
            raise ValueError('Source TLS descriptor register changed')
        return slot_kind(a[0])
    if tls is None:raise ValueError('Linked TLS provenance unavailable')
    # otool's decoded ADRP comment carries the absolute page. Its GOT slot is
    # independently matched against LC_DYSYMTAB, not inferred from a code label.
    absolute = re.findall(r'(?:;\s*|,\s*)(0x[0-9a-fA-F]+)', page['args'])
    if absolute:
        address = int(absolute[-1], 16)
    else:
        # Apple's otool prints the signed decoded A64 page displacement in
        # decimal when no absolute-page comment is present. ADRP's PC is its
        # own instruction page; the immediate has a signed 21-bit range.
        relative = re.fullmatch(r'x0,\s*#?(-?[0-9]+)', operand(page['args']))
        pc = page.get('address')
        if not relative or type(pc) is not int or not 0 <= pc < 1 << 64:
            raise ValueError('Unresolved linked ADRP page')
        delta = int(relative[1])
        if not -(1 << 20) <= delta < 1 << 20:
            raise ValueError('ADRP displacement exceeds encoding range')
        address = (pc & ~4095) + (delta << 12)
        if not 0 <= address < 1 << 64:raise ValueError('ADRP address outside range')
    if address & 4095:raise ValueError('ADRP page not aligned')
    if pointer['op'] == 'ldr':
        offset = re.fullmatch(r'x0,\s*\[x0(?:,\s*#(0x[0-9a-fA-F]+|[0-9]+))?\]', operand(pointer['args']))
        if not offset:raise ValueError('Unrecognized linked TLVP load')
        name = tls['indirect'].get(address + (int(offset[1], 0) if offset[1] else 0), '')
    elif pointer['op'] == 'add':
        offset = re.fullmatch(r'x0,\s*x0,\s*#(0x[0-9a-fA-F]+|[0-9]+)', operand(pointer['args']))
        if not offset:raise ValueError('Unrecognized relaxed TLS address')
        name = tls['direct'].get(address + int(offset[1], 0), '')
    else:raise ValueError('Unrecognized linked descriptor materialization')
    if not slot_kind(name):raise ValueError('Unknown linked TLS descriptor identity')
    return slot_kind(name)


def prove_cfg(functions, *, linked=False, symbol_addresses=None, tls_addresses=None):
    """Prove only recognized full tiny-accessor CFGs; unfamiliar shapes fail closed."""
    addresses = symbol_addresses or {}
    fn = functions['_nfs_probe_accessor'];code = fn['code']
    helpers = [name for name in functions if 'worker_stop_not_deferred' in name]
    if len(helpers) != 1:raise ValueError('Missing or ambiguous cold helper')
    helper = functions[helpers[0]]
    for f in (fn, helper):
        for row in f['code']:instruction_destinations(row)
    if not linked and any('@TLVP' in operand(r['args']) and SLOT_SYMBOLS['current'] not in operand(r['args']) for r in code):
        raise ValueError('Unexpected accessor TLS materialization')
    lookup_sites = [i for i, row in enumerate(code) if row['op'] == 'blr']
    if len(lookup_sites) != 1 or tls_lookup(fn, lookup_sites[0], linked, tls_addresses) != 'current':
        raise ValueError('Accessor does not use exactly the current-control TLS slot')
    helper_sites = [i for i, row in enumerate(helper['code']) if row['op'] == 'blr']
    if len(helper_sites) != 2 or {tls_lookup(helper, i, linked, tls_addresses) for i in helper_sites} != {'critical','deferrals'}:
        raise ValueError('Helper does not retain both exact depth slots')
    if sum(row['op'].startswith('ldar') for row in code) != 1 or any(row['op'].startswith('ldar') for row in helper['code']):
        raise ValueError('Atomic checkpoint count changed')
    edges = graph(fn, addresses);dom, pred = dominators(edges)
    # Bind each value to definitions which dominate EVERY use. Linear scanning
    # alone would accept unreachable guards or path-dependent clobbers.
    regs, atomic, checks, decision = {}, None, [], None
    for i, row in enumerate(code):
        if i not in dom:continue
        op, args = row['op'], operand(row['args']);parts = [x.strip() for x in args.split(',')]
        destinations = instruction_destinations(row)
        if atomic and i > atomic[0]:
            if any(re.fullmatch('[wx]' + atomic[1][1:], d) for d in destinations) or op in ('bl','blr'):
                raise ValueError('Atomic value clobbered before decision')
        def tag(reg):
            value = regs.get(reg)
            return value if value and value[1] <= dom[i] else None
        assigned = None
        if op == 'blr':
            definitions = set(range(i - 3, i + 1))
            if not definitions <= dom[i]:raise ValueError('TLS materialization does not dominate lookup')
            assigned = ('x0', ('slot', definitions))
            regs = {r:v for r,v in regs.items() if int(r[1:]) >= 19}
        elif op == 'mov' and len(parts) == 2 and tag(parts[1]):
            value = tag(parts[1]);assigned = (parts[0], (value[0], value[1] | {i}))
        elif op == 'ldr' and re.fullmatch(r'x\d+,\s*\[x\d+\]', args):
            dest, base = re.findall('x[0-9]+', args);value = tag(base)
            if value and value[0] == 'slot':assigned = (dest, ('control', value[1] | {i}))
        elif op.startswith('ldar'):
            match = re.fullmatch(r'(w\d+),\s*\[(x\d+)\]', args)
            value = tag(match[2]) if match else None
            if not value or value[0] != 'control':raise ValueError('Atomic address lost current-control provenance')
            atomic = (i, match[1])
        if op in ('cbz','cbnz') and len(parts) == 2:
            value = tag(parts[0])
            if value and value[0] == 'control':
                branch, fall = edges[i];checks.append((i, branch if op == 'cbz' else fall, fall if op == 'cbz' else branch))
        for dest in destinations:
            if re.fullmatch('[wx][0-9]+', dest):
                for prefix in ('w','x'):regs.pop(prefix + dest[1:], None)
        if assigned:regs[assigned[0]] = assigned[1]
        if op == 'bl':regs = {r:v for r,v in regs.items() if int(r[1:]) >= 19}
        if atomic and i > atomic[0] and op in ('tbz','tbnz','cbz','cbnz') and parts[0] == atomic[1]:
            if op in ('tbz','tbnz') and parts[1] not in ('#0','#0x0'):raise ValueError('Wrong atomic test bit')
            branch, fall = edges[i];decision = (i, branch if op in ('tbz','cbz') else fall, fall if op in ('tbz','cbz') else branch);break
    if atomic is None or decision is None:raise ValueError('Atomic decision was outlined or unrecognized')
    for i in range(1, decision[0] + 1):
        if i in dom and pred.get(i) != {i - 1}:raise ValueError('Unsupported nonlinear checkpoint prefix')
    allowed_pages = {lookup_sites[0] - 3}
    if any(r['op'] == 'adrp' and i not in allowed_pages for i,r in enumerate(code[:decision[0] + 1])):
        raise ValueError('Additional checkpoint page materialization is unproven')
    if atomic[0] not in dom[decision[0]]:raise ValueError('Atomic does not dominate decision')
    # The atomic-to-test segment must be one basic block, so a backedge cannot
    # replace the tested value through a lexically earlier instruction.
    for i in range(atomic[0] + 1, decision[0] + 1):
        if pred.get(i) != {i - 1}:raise ValueError('Nonlinear atomic reaching definition')
    if not any(check in dom[atomic[0]] and nonzero in dom[atomic[0]] and atomic[0] not in reachable(edges, zero) for check,zero,nonzero in checks):
        raise ValueError('Nonnull guard does not dominate atomic stop')
    def slow_call(name, visited=()):
        if name in helpers:return True
        if name in visited:raise ValueError('Recursive stop fragment')
        if name not in functions:
            # Only pinned C resolver and standard C++ exception/runtime callees
            # are permitted opaque. Unknown local fragments are not benign.
            if name in EXCEPTION_TERMINALS or name == '_nfs_host_address' or name.startswith(('___cxa_','__cxa_','__ZNSt','__ZSt','___stack_chk_fail','_memcpy','_memset')):return False
            raise ValueError('Unparsed internal or unknown direct callee')
        f = functions[name]
        graph(f, addresses)  # External tail branches are unsupported, never assumed benign.
        for at, row in enumerate(f['code']):
            instruction_destinations(row)
            if row['op'] == 'blr' and tls_lookup(f, at, linked, tls_addresses) in ('critical','deferrals'):return True
            if row['op'] == 'bl' and slow_call(call_target(row, addresses), (*visited,name)):return True
        return False
    sites = {i for i,row in enumerate(code) if row['op'] == 'bl' and slow_call(call_target(row, addresses))}
    before = reachable(edges, 0, decision[0]);false = reachable(edges, decision[1]);true = reachable(edges, decision[2])
    if not sites or sites & (before - {decision[0]}) or sites & false or not sites & true:
        raise ValueError('False stop can enter cold work or helper is unguarded')
    # The helper's two exact TLS lookups must feed live integer value loads.
    helper_edges = graph(helper, addresses);helper_dom,_ = dominators(helper_edges)
    for at in helper_sites:
        loaded = False
        for j in range(at + 1, len(helper['code'])):
            r = helper['code'][j]
            if r['op'] == 'ldr' and re.fullmatch(r'w\d+,\s*\[x0\]', operand(r['args'])) and at in helper_dom.get(j,set()):loaded = True;break
            if 'x0' in instruction_destinations(r) or r['op'] in ('bl','blr'):break
        if not loaded:raise ValueError('Depth TLS lookup has no live integer read')
    if not any(code[i]['op'] in ('ret','retab') for i in false):raise ValueError('No recognized normal return')
    return dict(proven=True, format='linked' if linked else 'source-assembly', accessor_tls_lookups=1,
                live_atomic_loads=1, helper_tls_lookups=2, atomic_index=atomic[0], stop_decision_index=decision[0],
                helper_call_sites=sorted(sites), false_path_bypasses_helper=True, nonnull_control_guard=True,
                tls_descriptor_provenance=True, scope='Pinned tiny accessor only; not final app code generation')


def preview(functions):
    return {name: [row['op'] + (' ' + operand(row['args']) if operand(row['args']) else '')
                   for row in fn['code']]
            for name, fn in functions.items() if name in PROBES or 'worker_stop_not_deferred' in name or 'OUTLINED_FUNCTION_' in name}


def replay_metadata(functions, addresses=None, tls=None):
    """Bounded selected PCs/labels and resolved symbols, never tool path headers."""
    addresses = addresses or {}
    selected, referenced = {}, {}
    for name, fn in functions.items():
        if name not in PROBES and 'worker_stop_not_deferred' not in name and 'OUTLINED_FUNCTION_' not in name:
            continue
        code = []
        for row in fn['code']:
            clean = dict(op=row['op'], args=operand(row['args']), address=row.get('address'))
            if row['op'] == 'adrp':
                absolute = re.findall(r'(?:;\s*|,\s*)(0x[0-9a-fA-F]+)', row['args'])
                if absolute:clean['args'] += ' ; ' + absolute[-1]
            if row['op'] == 'bl':
                try:
                    target = call_target(row, addresses)
                    clean['resolved_call_target'] = target
                    arg = operand(row['args'])
                    if re.fullmatch(r'(?:0x)?[0-9a-fA-F]{8,16}', arg):
                        referenced[hex(int(arg.removeprefix('0x'), 16))] = target
                except ValueError:
                    clean['call_target_unresolved'] = True
            code.append(clean)
        selected[name] = dict(name=name, labels=dict(fn['labels']), code=code)
    result = dict(functions=selected, call_target_symbols=referenced)
    if tls is not None:
        result['tls_symbols'] = {kind: {hex(k): value for k,value in table.items()}
                                 for kind,table in tls.items()}
    return result


PROOF_FAILURE_REASONS = frozenset(('ADRP address outside range', 'ADRP displacement exceeds encoding range', 'ADRP page not aligned', 'Accessor does not use exactly the current-control TLS slot', 'Additional checkpoint page materialization is unproven', 'Ambiguous TLS symbol', 'Atomic address lost current-control provenance', 'Atomic checkpoint count changed', 'Atomic decision was outlined or unrecognized', 'Atomic does not dominate decision', 'Atomic value clobbered before decision', 'Branch leaves recognized function', 'Cold correctness differs from frozen OLD oracle', 'Cold fixture identity changed', 'Compile budget exhausted', 'Deferred state was not verified', 'Depth TLS lookup has no live integer read', 'Disassembly exceeds bound', 'Disassembly line exceeds bound', 'False stop can enter cold work or helper is unguarded', 'Helper does not retain both exact depth slots', 'Incomplete balanced cold timing', 'Invalid Mach-O command bounds', 'Invalid Mach-O command length', 'Invalid Mach-O symbol command bounds', 'Invalid Mach-O symbol storage', 'Invalid SDK version', 'Invalid build tools', 'Invalid indirect section index', 'Invalid indirect symbol index', 'Invalid indirect symbol table', 'Invalid pointer section', 'Invalid section bounds', 'Invalid source identity', 'Invalid symbol command', 'Invalid symbol table', 'Linked TLS provenance unavailable', 'Mach-O exceeds bound', 'Measured mode checksum differs', 'Missing exact source TLS relocation', 'Missing full probe body', 'Missing or ambiguous cold helper', 'Missing symbol metadata', 'Mode or variant warmup checksum differs', 'No recognized normal return', 'Nonlinear atomic reaching definition', 'Nonnull guard does not dominate atomic stop', 'Normal output protocol changed', 'Recursive stop fragment', 'Required linked probe symbols were eliminated', 'Selected body exceeds bound; no truncated proof', 'Source TLS descriptor register changed', 'Symbol output exceeds bound', 'TLS descriptor function load missing', 'TLS indirect call target changed', 'TLS materialization does not dominate lookup', 'TLS page materialization missing', 'Truncated Mach-O command', 'Truncated platform command', 'Truncated segment', 'Truncated symbol command', 'Unbounded Mach-O symbol table', 'Unexpected accessor TLS materialization', 'Unexpected cold closure difference', 'Unknown assembly instruction format', 'Unknown branch condition', 'Unknown direct call target', 'Unknown linked TLS descriptor identity', 'Unknown linked instruction format', 'Unparsed internal or unknown direct callee', 'Unrecognized TLS lookup', 'Unrecognized linked TLVP load', 'Unrecognized linked descriptor materialization', 'Unrecognized relaxed TLS address', 'Unresolved direct call target', 'Unresolved linked ADRP page', 'Unsupported indirect control flow', 'Unsupported instruction semantics', 'Unsupported measurement mode/order', 'Unsupported non-stack load writeback', 'Unsupported nonlinear checkpoint prefix', 'Unsupported store writeback', 'Wrong Mach-O SDK version', 'Wrong Mach-O platform or deployment target', 'Wrong atomic test bit'))


def proof_failure_reason(error):
    reason = str(error)
    return reason if reason in PROOF_FAILURE_REASONS else 'Unrecognized proof failure'


def ios_flags(clang, sdk, folder, variant):
    return [clang, '-target', 'arm64-apple-ios16.0', '-std=c++14', '-O3', '-DNDEBUG',
            '-fwrapv', '-fno-strict-aliasing', '-isysroot', sdk,
            '-ffile-prefix-map=' + str(ROOT) + '=/source',
            '-ffile-prefix-map=' + str(folder) + '=/probe',
            '-fdebug-prefix-map=' + str(ROOT) + '=/source',
            '-fdebug-prefix-map=' + str(folder) + '=/probe', '-I' + str(folder / variant)]


def ios_codegen(config, folder, deadline):
    clang = runtime.checked(['xcrun', '--sdk', 'iphoneos', '--find', 'clang++'])
    sdk = runtime.checked(['xcrun', '--sdk', 'iphoneos', '--show-sdk-path'])
    expected_sdk = sdk_number(runtime.checked(['xcrun', '--sdk', 'iphoneos', '--show-sdk-version']))
    report = dict(target='arm64-apple-ios16.0', executed=False, variants={})
    print('COLD_STAGE ios-codegen', file=sys.stderr, flush=True)
    for variant in VARIANTS:
        flags = ios_flags(clang, sdk, folder, variant)
        source = ROOT / 'tests/apple_stop_slot_assembly_probe.cpp'
        output = folder / (variant + '-ios.s')
        runtime.checked([*flags, '-S', str(source), '-o', str(output)], timeout=remaining(deadline))
        print('COLD_STAGE source-assembly ' + variant, file=sys.stderr, flush=True)
        pre = parse_functions(output.read_text())
        objects = []
        for i, unit in enumerate((ROOT / 'tests/apple_stop_slot_runtime_probe.cpp', source)):
            obj = folder / (variant + '-ios-' + str(i) + '.o')
            runtime.checked([*flags, '-flto=thin', '-c', str(unit), '-o', str(obj)], timeout=remaining(deadline))
            objects.append(obj)
        binary = folder / (variant + '-ios-helper')
        exports = list(PROBES) + ([HELPER] if variant == 'cold' else [])
        runtime.checked([*flags, '-flto=thin', '-Wl,-mllvm,-threads=2',
                         *('-Wl,-exported_symbol,' + name for name in exports),
                         *map(str, objects), '-o', str(binary)], timeout=remaining(deadline))
        print('COLD_STAGE linked-ios ' + variant, file=sys.stderr, flush=True)
        platform = macho_platform(binary, 2, 16 << 16, expected_sdk)
        address_names = symbols(runtime.checked(['nm', '-n', str(binary)], timeout=remaining(deadline)))
        if not set(exports) <= set(address_names.values()):
            raise ValueError('Required linked probe symbols were eliminated')
        linked = parse_functions(runtime.checked(['xcrun', 'otool', '-tvV', str(binary)], timeout=remaining(deadline)), True, address_names)
        linked_tls = macho_tls_symbols(binary)
        row = dict(platform=platform, thin_lto=True, backend_workers=2,
                   pre_lto_selected=preview(pre), linked_selected=preview(linked),
                   pre_lto_replay=replay_metadata(pre),
                   linked_replay=replay_metadata(linked, address_names, linked_tls))
        if variant == 'cold':
            for stage, functions, options in (
                    ('pre_lto', pre, {}),
                    ('linked', linked, dict(linked=True, symbol_addresses=address_names, tls_addresses=linked_tls))):
                try:
                    row[stage + '_cfg'] = prove_cfg(functions, **options)
                except ValueError as error:
                    row['cfg_unproven'] = True
                    row[stage + '_cfg_failure_reason'] = proof_failure_reason(error)
        else:
            for functions in (pre, linked):
                accessor = functions['_nfs_probe_accessor']['code']
                if sum(x['op'] == 'blr' for x in accessor) != 3 or sum(x['op'].startswith('ldar') for x in accessor) != 1:
                    row['old_reference_unrecognized'] = True
        report['variants'][variant] = row
    report['proven'] = all(not row.get('cfg_unproven') and not row.get('old_reference_unrecognized') for row in report['variants'].values())
    return report


def measure_args(mode, order):
    if mode not in MODES or type(order) is not int or not 0 <= order < 3:
        raise ValueError('Unsupported measurement mode/order')
    return [('--measure' if mode == 'normal' else '--measure-' + mode), str(order)]


def validate_mode(rows, mode):
    for row in rows:
        if mode == 'normal':
            if 'deferred_mode' in row or 'stop_after_clear_verified' in row:
                raise ValueError('Normal output protocol changed')
        elif row.get('deferred_mode') != mode or row.get('stop_after_clear_verified') is not True:
            raise ValueError('Deferred state was not verified')


def summarize(rows):
    medians = {mode: {} for mode in MODES}
    for mode in MODES:
        for variant in VARIANTS:
            medians[mode][variant] = {}
            for kernel in runtime.KERNELS:
                selected = [r for r in rows if r['mode'] == mode and r['variant'] == variant and r['kernel'] == kernel]
                if len(selected) != GATE['rounds'] or {r['round'] for r in selected} != set(range(GATE['rounds'])):
                    raise ValueError('Incomplete balanced cold timing')
                medians[mode][variant][kernel] = {kind + '_ns_per_invocation': statistics.median(r[kind + '_ns'] / r['iterations'] for r in selected) for kind in ('cpu', 'wall')}
    changes = {mode: {kernel: 100 * (medians[mode]['old'][kernel]['cpu_ns_per_invocation'] - medians[mode]['cold'][kernel]['cpu_ns_per_invocation']) / medians[mode]['old'][kernel]['cpu_ns_per_invocation'] for kernel in runtime.KERNELS} for mode in MODES}
    decision = dict(normal_cpu_improvement_percent=changes['normal'],
                    deferred_cpu_improvement_percent={m: changes[m] for m in MODES[1:]},
                    component_gate_passed=changes['normal']['accessor'] >= 10 and all(changes['normal'][k] >= -5 for k in ('transform9', 'rep8')),
                    scope='Normal gate only; deferred modes report cost, not actual shutdown latency, app FPS or energy')
    return medians, decision


def execute(local=False):
    config = runtime.configuration(local)
    commit = os.environ.get('GITHUB_SHA', 'local')
    if commit != 'local' and not re.fullmatch('[0-9a-f]{40}', commit):
        raise ValueError('Invalid source identity')
    report = dict(schema='nfsiise-cold-stop-probe-v1', source_commit=commit,
                  local_safety_only=local, configuration=config['identity'], predeclared_gate=GATE,
                  source_sha256={name: runtime.digest(ROOT / name) for name in ('scripts/apple_cold_stop_probe.py', 'tests/apple_stop_slot_runtime_probe.cpp', 'tests/apple_stop_slot_assembly_probe.cpp', COLD_FILE)},
                  scope='Synthetic OLD/COLD component; no game, assets, GPU, iOS execution, app/FPS/energy claim', variants={}, measurements=[])
    with tempfile.TemporaryDirectory(prefix='nfs-cold-stop-') as temporary:
        folder = Path(temporary)
        closures = materialize(ROOT, folder)
        compile_deadline = time.monotonic() + GATE['compile_budget_seconds']
        executables = {}
        for variant in VARIANTS:
            binary = runtime.compile_variant(config, folder, variant, compile_deadline)
            if not local:
                macho_platform(binary, 1, 11 << 16, sdk_number(config['identity']['sdk_version']))
            executables[variant] = binary
            report['variants'][variant] = dict(binary_sha256=runtime.digest(binary), header_closure_sha256=closures[variant])
        if not local:
            report['ios_codegen'] = ios_codegen(config, folder, compile_deadline)
            if not report['ios_codegen']['proven']:
                report['status'] = 'ios-cfg-unproven'
                return report
        start = time.monotonic()
        deadline = start + GATE['native_execution_budget_seconds']
        expected = {}
        for variant in VARIANTS:
            proof = runtime.run_native(executables[variant], ['--verify'], deadline, False)[0]
            if proof != EXPECTED_PROOF:
                raise ValueError('Cold correctness differs from frozen OLD oracle')
            report['variants'][variant]['verification'] = proof
            for mode in MODES:
                warmup = runtime.run_native(executables[variant], measure_args(mode, 0), deadline, True)
                validate_mode(warmup, mode)
                for row in warmup:
                    value = expected.setdefault(row['kernel'], row['checksum'])
                    if row['checksum'] != value:
                        raise ValueError('Mode or variant warmup checksum differs')
        report['discarded_warmup_invocations'] = 6
        if not local:
            for round_ in range(GATE['rounds']):
                order = VARIANTS if round_ % 2 == 0 else tuple(reversed(VARIANTS))
                modes = MODES[round_ % 3:] + MODES[:round_ % 3]
                for mode in modes:
                    for variant in order:
                        values = runtime.run_native(executables[variant], measure_args(mode, round_ % 3), deadline, True)
                        validate_mode(values, mode)
                        for row in values:
                            if row['checksum'] != expected[row['kernel']]:
                                raise ValueError('Measured mode checksum differs')
                            report['measurements'].append(dict(round=round_, variant=variant, mode=mode, **row))
            report['medians'], report['decision'] = summarize(report['measurements'])
        report.update(complete=True, all_variants_verified=True, native_execution_seconds=round(time.monotonic() - start, 3),
                      native_link_time_optimization=False, ios_executed=False, artifacts_uploaded=False, app_built=False)
    return report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--local-safety-check', action='store_true')
    args = parser.parse_args()
    try:
        report = execute(args.local_safety_check)
        print(json.dumps(report, sort_keys=True, indent=2, allow_nan=False))
        return 0 if report.get('complete') else 1
    except (OSError, ValueError, KeyError, TypeError, UnicodeError, RuntimeError, struct.error, subprocess.SubprocessError):
        # No compiler stderr, raw otool header, argv or temporary paths in logs.
        parser.exit(1, 'Cold-stop probe failed an input, compile, protocol, platform, CFG or time bound.\n')


if __name__ == '__main__':
    raise SystemExit(main())
