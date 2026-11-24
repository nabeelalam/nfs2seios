# SPDX-License-Identifier: MIT
"""Bounded synthetic checked native-span probe; no assets, app or timing claim."""
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

ROOT=Path(__file__).resolve().parents[1]
FIXTURE=ROOT/'tests/fixtures/native_span'
SYNTHETIC=ROOT/'tests/fixtures/owned_chunk/synthetic'

def checked(args,deadline,timeout=60):
    remaining=deadline-time.monotonic()
    if remaining<=0:raise ValueError('Probe deadline exhausted')
    p=subprocess.run(args,capture_output=True,timeout=min(timeout,remaining))
    if p.returncode or max(len(p.stdout),len(p.stderr))>1048576:raise ValueError('Probe command failed or exceeded output bound')
    return p.stdout

def execute(local=False):
    deadline=time.monotonic()+240
    flags=['-std=c++14','-O1','-g','-pthread','-fwrapv','-fno-strict-aliasing','-fsanitize=address,undefined','-fno-sanitize-recover=all','-fno-omit-frame-pointer']
    if local:
        if sys.platform!='linux':raise ValueError('Local safety check requires Linux')
        compiler=shutil.which('clang++')
        if not compiler:raise ValueError('Existing compiler unavailable')
    else:
        if sys.platform!='darwin' or platform.machine()!='arm64':raise ValueError('Native Apple ARM host required')
        if checked(['xcodebuild','-version'],deadline).splitlines()[0]!=b'Xcode 16.4':raise ValueError('Pinned Xcode required')
        compiler=checked(['xcrun','--sdk','macosx','--find','clang++'],deadline).decode().strip()
        sdk=checked(['xcrun','--sdk','macosx','--show-sdk-path'],deadline).decode().strip()
        flags+=['-arch','arm64','-isysroot',sdk]
    provenance=json.loads((FIXTURE/'source-provenance.json').read_text())
    for entry in provenance['sources']:
        for variant in ['control','candidate']:
            if hashlib.sha256((FIXTURE/variant/entry['file']).read_bytes()).hexdigest()!=entry[variant+'_sha256']:raise ValueError('Frozen source mismatch')
    sources=[Path(__file__).resolve(),ROOT/'tests/native_span_oracle.cpp',ROOT/'tests/owned_chunk_oracle.cpp',ROOT/'src/HeapArena.h',ROOT/'src/GameMemory.h',ROOT/'src/GameTrace.cpp',ROOT/'src/GameTrace.h',ROOT/'src/CodecOperations.h',ROOT/'src/WorkerControl.h',*sorted(FIXTURE.rglob('*.h')),*sorted(FIXTURE.rglob('*.cpp')),*sorted(SYNTHETIC.glob('*.h'))]
    report=dict(schema=1,scope='Synthetic native-span correctness and SDK compilation only',local_safety_only=local,architecture=platform.machine(),ios_executed=False,app_built=False,performance_measured=False,source_provenance=[dict(file=p.relative_to(ROOT).as_posix(),sha256=hashlib.sha256(p.read_bytes()).hexdigest())for p in sources])
    results={}
    with tempfile.TemporaryDirectory(prefix='nfs-native-span-')as folder:
        folder=Path(folder)
        for variant in ['control','candidate']:
            includes=['-I'+str(FIXTURE/variant),'-I'+str(ROOT/'src'),'-I'+str(SYNTHETIC)]
            define=['-DNFS_NATIVE_SPAN_CANDIDATE=1']if variant=='candidate'else[]
            results[variant]={}
            for name,oracle in [('integer',ROOT/'tests/owned_chunk_oracle.cpp'),('native',ROOT/'tests/native_span_oracle.cpp')]:
                binary=folder/(variant+'-'+name)
                checked([compiler,*flags,*define,*includes,str(oracle),str(FIXTURE/variant/'EngineMemory.cpp'),str(ROOT/'src/GameTrace.cpp'),'-o',str(binary)],deadline)
                if not local:
                    magic,cpu,_,kind=struct.unpack_from('<IIII',binary.read_bytes())
                    if(magic,cpu,kind)!=(0xfeedfacf,0x0100000c,2):raise ValueError('Native ARM executable mismatch')
                results[variant][name]=json.loads(checked([str(binary)],deadline,30))
        if results['control']!=results['candidate']:raise ValueError('Control/candidate outputs differ')
        integer=results['candidate']['integer'];native=results['candidate']['native']
        expected=(12109,69037,680644,80000,80000,160000,8306997119277202445,True)
        if tuple(integer[k]for k in ['checks','dataBytes','bssBytes','concurrentOwnedChunkReads','concurrentLiveSpanReads','concurrentStaticSpanReads','oracleDigest','bindingTransitionsChecked'])!=expected:raise ValueError('Integer coverage mismatch')
        if not(native['passed']and native['nativeSpanDifferentialCases']==15277 and native['concurrentNativeCopyCycles']==40000 and native['bindingTransitionsChecked']and native['exactErrorsPreserved']and native['engineObjectBytes']==8):raise ValueError('Native coverage mismatch')
        report['sanitized_results']=results
        if not local:
            if checked(['xcrun','--sdk','iphoneos','--show-sdk-version'],deadline).strip()!=b'18.5':raise ValueError('Pinned iOS SDK required')
            sdk=checked(['xcrun','--sdk','iphoneos','--show-sdk-path'],deadline).decode().strip()
            access=folder/'access.cpp';access.write_text('#include "GuestCopyTransaction.h"\nvoid instantiate(uint32_t a,void*p){guest_store<uint32_t>(a,guest_load<uint32_t>(p));nfs::GuestCopyTransaction tx;tx.store<uint16_t>(p,tx.load<uint16_t>(a));}\n')
            for variant in ['control','candidate']:
                for name,source in [('memory',FIXTURE/variant/'EngineMemory.cpp'),('access',access)]:
                    obj=folder/(variant+'-'+name+'.o')
                    checked([compiler,'-std=c++14','-O2','-target','arm64-apple-ios16.0','-isysroot',sdk,'-I'+str(FIXTURE/variant),'-I'+str(ROOT/'src'),'-I'+str(SYNTHETIC),'-c',str(source),'-o',str(obj)],deadline)
                    magic,cpu,_,kind=struct.unpack_from('<IIII',obj.read_bytes())
                    if(magic,cpu,kind)!=(0xfeedfacf,0x0100000c,1):raise ValueError('iOS ARM object mismatch')
            report['ios_arm64_memory_and_native_integer_access_objects_compiled']=True
    report['complete']=True
    return report

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--local-safety-check',action='store_true');args=parser.parse_args()
    try:print(json.dumps(execute(args.local_safety_check),sort_keys=True,indent=2))
    except(OSError,ValueError,KeyError,struct.error,subprocess.SubprocessError):parser.exit(1,'Native-span probe failed its platform, coverage, compile, runtime or time bound.\n')
if __name__=='__main__':main()
