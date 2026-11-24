# SPDX-License-Identifier: MIT
"""Bounded synthetic data-window safety/SDK check; no app or timing claim."""
import argparse
import importlib.util
import json
from pathlib import Path
import platform
import shutil
import subprocess
import sys
import tempfile
import time

ROOT = Path(__file__).resolve().parents[1]
FIXTURE = ROOT / 'tests/fixtures/data_span_cache'
helper_spec = importlib.util.spec_from_file_location('frame_probe_helpers', ROOT / 'scripts/apple_frame_cache_probe.py')
helper = importlib.util.module_from_spec(helper_spec)
helper_spec.loader.exec_module(helper)
OMITTED = {'tests/fixtures/frame_span_cache/CachedFrameTransaction.h',
           'tests/frame_span_cache_oracle.cpp', 'tests/owned_chunk_oracle.cpp',
           'tests/native_span_oracle.cpp'}
EXPECTED = (helper.EXPECTED - OMITTED) | {
    'tests/fixtures/data_span_cache/CachedFrameTransaction.h',
    'tests/fixtures/data_span_cache/data_span_cache_oracle.cpp',
    'scripts/apple_frame_cache_probe.py'}
COVERAGE = dict(passed=True, framePlusThreeProbeBudgetChecked=True,
                fourthWindowFallsBack=True, widthCrossingAndNoPartialWriteChecked=True,
                dataRevisionAndSmallerReuseChecked=True, negativeProbeRemembered=True,
                errnoAndFpFlagsPreserved=True, retiredDataRevisionNotDereferenced=True)


def sources():
    rows = json.loads((FIXTURE / 'source-provenance.json').read_text())['sources']
    if len(rows) != len(EXPECTED) or {row['file'] for row in rows} != EXPECTED:
        raise ValueError('Data-cache source closure mismatch')
    names = set()
    for row in rows:
        path = ROOT / row['file']
        if path.is_symlink() or not path.is_file() or helper.sha(path) != row['sha256'] or path.name in names:
            raise ValueError('Frozen data-cache source mismatch')
        names.add(path.name)
    return rows


def verify_result(result):
    if result != COVERAGE or any(type(result[key]) is not bool for key in COVERAGE):
        raise ValueError('Data-cache oracle coverage mismatch')
    return result


def access_source():
    return '''#include "CachedFrameTransaction.h"
#include "EngineMemory.h"
#include "DATA.h"
#include "BSS.h"
alignas(4) DataLayout _data{};alignas(4) BssLayout _bss{};
extern "C" __attribute__((noinline)) uint32_t nfs_data_probe_kernel(uint32_t frame,uint32_t data){
 nfs::CachedFrameTransaction tx{frame};auto first=tx.load<uint32_t>(data);
 tx.store<uint32_t>(data+4,first);return tx.load<uint32_t>(data+4);
}
int main(){nfs::EngineMemory memory(65536);auto*frame=memory.allocate(128);auto*data=memory.allocate(128);
 if(!frame || !data)return 2;uint32_t expected=0x13579bdfu;__builtin_memcpy(data,&expected,4);
 const auto result=nfs_data_probe_kernel(memory.encode(frame),memory.encode(data));
 memory.release(data);memory.release(frame);return result==expected?0:3;}
'''


def execute(local=False):
    deadline = time.monotonic() + 180
    helper.phase('data_source_review')
    rows = sources()
    helper.phase('compiler_discovery')
    common = ['-std=c++14', '-pthread', '-fwrapv', '-fno-strict-aliasing', '-frounding-math']
    if local:
        if sys.platform != 'linux':
            raise ValueError('Linux required for local safety check')
        compiler = shutil.which('clang++')
        if not compiler:
            raise ValueError('Existing compiler unavailable')
    else:
        if sys.platform != 'darwin' or platform.machine() != 'arm64':
            raise ValueError('Native Apple ARM host required')
        compiler = helper.checked(['xcrun', '--find', 'clang++'], deadline).decode().strip()
        sdk = helper.checked(['xcrun', '--sdk', 'macosx', '--show-sdk-path'], deadline).decode().strip()
        common += ['-arch', 'arm64', '-isysroot', sdk]
    report = dict(schema=1, scope='Synthetic data-window correctness and SDK only; no app, game data, timing or energy',
                  localSafetyOnly=local, architecture=platform.machine(), sources=rows,
                  appBuilt=False, performanceMeasured=False, iosExecuted=False,
                  concurrentFreeSafetyClaimed=False, deliveredBuild27Changed=False)
    with tempfile.TemporaryDirectory(prefix='nfs-data-cache-') as temporary:
        folder = Path(temporary)
        for row in rows:
            (folder / Path(row['file']).name).write_bytes((ROOT / row['file']).read_bytes())
        includes = ['-I' + str(folder)]
        rename = ['-Dnfs_host_address=nfs_raw_host_address', '-Dnfs_host_native_span=nfs_raw_host_native_span']
        for label, optimization in [('sanitized', ['-O1', '-g', '-fsanitize=address,undefined',
                                                  '-fno-sanitize-recover=all', '-fno-omit-frame-pointer']),
                                    ('optimized', ['-O3'])]:
            flags = common + optimization + includes
            obj = folder / (label + '-memory.o')
            helper.phase(label + '_memory_compile')
            helper.checked([compiler, *flags, *rename, '-c', str(folder / 'EngineMemory.cpp'), '-o', str(obj)], deadline)
            binary = folder / label
            helper.phase(label + '_oracle_compile')
            helper.checked([compiler, *flags, str(folder / 'data_span_cache_oracle.cpp'), str(obj),
                            str(folder / 'GameTrace.cpp'), '-o', str(binary)], deadline)
            if not local:
                helper.macho(binary, 2)
            helper.phase(label + '_oracle_run')
            report[label + 'Results'] = verify_result(json.loads(helper.checked([str(binary)], deadline, 30)))
        access = folder / 'access.cpp'
        access.write_text(access_source())
        host = folder / 'host-data-probe'
        helper.phase('host_synthetic_link')
        helper.checked([compiler, *common, '-O3', *includes, str(folder / 'EngineMemory.cpp'),
                        str(folder / 'GameTrace.cpp'), str(access), '-o', str(host)], deadline)
        helper.phase('host_synthetic_run')
        helper.checked([str(host)], deadline, 30)
        report['hostSyntheticLinkedKernelPassed'] = True
        if not local:
            helper.phase('ios_sdk_verification')
            if helper.checked(['xcrun', '--sdk', 'iphoneos', '--show-sdk-version'], deadline).strip() != b'18.5':
                raise ValueError('Pinned iOS SDK required')
            sdk = helper.checked(['xcrun', '--sdk', 'iphoneos', '--show-sdk-path'], deadline).decode().strip()
            flags = ['-std=c++14', '-O3', '-fwrapv', '-fno-strict-aliasing', '-frounding-math',
                     '-target', 'arm64-apple-ios16.0', '-isysroot', sdk, *includes]
            objects = []
            for name in ['EngineMemory.cpp', 'GameTrace.cpp', 'access.cpp']:
                obj = folder / (name + '.o')
                helper.phase('ios_' + name + '_compile')
                helper.checked([compiler, *flags, '-c', str(folder / name), '-o', str(obj)], deadline)
                helper.macho(obj, 1, 2, 16 << 16, (0, (18 << 16) | (5 << 8)))
                objects.append(obj)
            binary = folder / 'ios-data-probe'
            helper.phase('ios_link')
            helper.checked([compiler, *flags, *map(str, objects), '-o', str(binary)], deadline)
            report['iosLinkedProbe'] = helper.macho(binary, 2, 2, 16 << 16, (18 << 16) | (5 << 8))
            helper.phase('ios_linked_kernel')
            assembly = helper.checked(['xcrun', 'otool', '-tvV', str(binary)], deadline).decode()
            start = assembly.find('_nfs_data_probe_kernel:')
            if start < 0:
                raise ValueError('Linked data kernel missing')
            body = assembly[start:]
            end = body.find('\n_', 1)
            if end >= 0:
                body = body[:end]
            if len(body) > 65536:
                raise ValueError('Linked data kernel exceeds bound')
            report.update(linkedIosKernel=body, iosSdk='18.5', iosMinimum='16.0', iosObjectsCompiled=True)
    sources()
    report['complete'] = True
    return report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--local-safety-check', action='store_true')
    args = parser.parse_args()
    try:
        print(json.dumps(execute(args.local_safety_check), sort_keys=True, indent=2))
    except (OSError, ValueError, KeyError, helper.struct.error, subprocess.SubprocessError) as error:
        print(json.dumps(dict(type='probe_failure', phase=helper.CURRENT_PHASE, reason=helper.safe_detail(error))), flush=True)
        parser.exit(1, 'Data-cache probe failed; inspect the bounded phase diagnostic.\n')


if __name__ == '__main__':
    main()
