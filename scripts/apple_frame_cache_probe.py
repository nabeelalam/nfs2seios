# SPDX-License-Identifier: MIT
"""Bounded synthetic cache correctness/SDK probe; no app or performance claim."""
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
FIXTURE=ROOT/'tests/fixtures/frame_span_cache'
EXPECTED={*('tests/fixtures/frame_span_cache/'+n for n in ['EngineMemory.cpp','HeapArena.h','CachedFrameTransaction.h','FrameSpanCacheAbi.h']),*('tests/'+n for n in ['frame_span_cache_oracle.cpp','owned_chunk_oracle.cpp','native_span_oracle.cpp']),*('src/'+n for n in ['EngineMemory.h','GuestCopyTransaction.h','GuestAccess.h','GuestRuntimeC.h','WorkerControl.h','GameMemory.h','GameTrace.cpp','GameTrace.h','CodecOperations.h']),*('tests/fixtures/owned_chunk/synthetic/'+n for n in ['DATA.h','BSS.h'])}

CURRENT_PHASE='initialization'
def phase(name):
    global CURRENT_PHASE
    CURRENT_PHASE=name
    print(json.dumps(dict(type='probe_phase',phase=name)),flush=True)
def safe_detail(value):
    text=value.decode(errors='replace')if isinstance(value,bytes)else str(value)
    for path,label in [(str(ROOT),'<source>'),(tempfile.gettempdir(),'<temporary>')]:
        text=text.replace(path,label).replace(path.replace('\\','/'),label)
    return text[:4096]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def sources():
    rows=json.loads((FIXTURE/'source-provenance.json').read_text())['sources']
    if len(rows)!=len(EXPECTED)or {x['file']for x in rows}!=EXPECTED:raise ValueError('Source closure mismatch')
    names=set()
    for x in rows:
        p=ROOT/x['file']
        if p.is_symlink()or not p.is_file()or sha(p)!=x['sha256']or p.name in names:raise ValueError('Frozen source mismatch')
        names.add(p.name)
    return rows

def checked(args,deadline,timeout=60):
    remaining=deadline-time.monotonic()
    if remaining<=0:raise ValueError('Probe deadline exhausted')
    p=subprocess.run(args,capture_output=True,timeout=min(timeout,remaining))
    if max(len(p.stdout),len(p.stderr))>1048576:raise ValueError('Probe command exceeded output bound')
    if p.returncode:raise ValueError('Command '+Path(args[0]).name+' exited '+str(p.returncode)+': '+safe_detail(p.stderr))
    return p.stdout

def macho(path,kind,platform_id=None,minimum=None,sdk=None):
    data=path.read_bytes()
    if not 32<=len(data)<=8*1024*1024:raise ValueError('Mach-O size outside bound')
    magic,cpu,_,filetype=struct.unpack_from('<IIII',data)
    if (magic,cpu,filetype)!=(0xfeedfacf,0x0100000c,kind):raise ValueError('ARM64 Mach-O type mismatch')
    if platform_id is not None:
        _,_,_,_,commands,size=struct.unpack_from('<IIIIII',data)
        if not 0<commands<=256 or size>len(data)-32:raise ValueError('Mach-O load commands outside bound')
        cursor=32;limit=cursor+size;builds=[]
        for _ in range(commands):
            if cursor+8>limit:raise ValueError('Mach-O command truncated')
            command,length=struct.unpack_from('<II',data,cursor)
            if length<8 or length%8 or length>limit-cursor:raise ValueError('Mach-O command outside bound')
            if command==0x32:
                if length<24:raise ValueError('Build command truncated')
                target,version,sdk_version,tools=struct.unpack_from('<IIII',data,cursor+8)
                if tools>32 or length!=24+8*tools:raise ValueError('Build tools outside bound')
                builds.append((target,version,sdk_version))
            cursor+=length
        allowed=sdk if isinstance(sdk,tuple)else(sdk,)
        if cursor!=limit or len(builds)!=1 or builds[0][:2]!=(platform_id,minimum)or builds[0][2]not in allowed:raise ValueError('Mach-O platform or SDK mismatch: '+str(builds))
        sdk=builds[0][2]
    return dict(architecture='arm64',filetype=filetype,sha256=sha(path),platform=platform_id,minimum=minimum,sdk=sdk)

def verify_results(results,heap_bytes=49256):
    edge=results['edges']
    expected=dict(passed=True,checks=90,probeMatchesCanonicalValidity=True,probePreservesErrnoAndFpFlags=True,fullWidthAndPartialWindowsChecked=True,revisionInvalidationAndSmallerReuseChecked=True,unboundAndRebindingChecked=True,noRetiredRevisionPointerDereference=True)
    if edge!=expected or any(type(edge[k])is not type(v)for k,v in expected.items()):raise ValueError('Cache oracle coverage mismatch')
    integer=results['integer'];expected=(12109,69037,680644,80000,80000,160000,8306997119277202445,True,8,heap_bytes)
    keys=['checks','dataBytes','bssBytes','concurrentOwnedChunkReads','concurrentLiveSpanReads','concurrentStaticSpanReads','oracleDigest','bindingTransitionsChecked','engineObjectBytes','heapObjectBytes']
    if tuple(integer[k]for k in keys)!=expected or any(type(integer[k])is not type(v)for k,v in zip(keys,expected)):raise ValueError('Integer coverage mismatch')
    native=results['native'];expected=dict(passed=True,nativeSpanDifferentialCases=15277,concurrentNativeCopyCycles=40000,bindingTransitionsChecked=True,exactErrorsPreserved=True,engineObjectBytes=8)
    if native!=expected or any(type(native[k])is not type(v)for k,v in expected.items()):raise ValueError('Native coverage mismatch')
    return results

def execute(local=False):
    deadline=time.monotonic()+240;phase('source_review');rows=sources();phase('compiler_discovery')
    flags=['-std=c++14','-O1','-g','-pthread','-fwrapv','-fno-strict-aliasing','-frounding-math','-fsanitize=address,undefined','-fno-sanitize-recover=all','-fno-omit-frame-pointer','-DNFS_NATIVE_SPAN_CANDIDATE=1']
    if local:
        if sys.platform!='linux':raise ValueError('Linux required for local safety check')
        compiler=shutil.which('clang++')
        if not compiler:raise ValueError('Existing compiler unavailable')
    else:
        if sys.platform!='darwin'or platform.machine()!='arm64':raise ValueError('Native Apple ARM host required')
        compiler=checked(['xcrun','--find','clang++'],deadline).decode().strip()
        sdk=checked(['xcrun','--sdk','macosx','--show-sdk-path'],deadline).decode().strip();flags+=['-arch','arm64','-isysroot',sdk]
    report=dict(schema=1,scope='Synthetic frame-cache correctness and SDK only; no app, game data, timing or energy',localSafetyOnly=local,architecture=platform.machine(),sources=rows,appBuilt=False,performanceMeasured=False,iosExecuted=False)
    with tempfile.TemporaryDirectory(prefix='nfs-frame-cache-')as folder:
        folder=Path(folder)
        for row in rows:(folder/Path(row['file']).name).write_bytes((ROOT/row['file']).read_bytes())
        includes=['-I'+str(folder)];results={}
        for name,oracle in [('edges','frame_span_cache_oracle.cpp'),('integer','owned_chunk_oracle.cpp'),('native','native_span_oracle.cpp')]:
            obj=folder/(name+'-memory.o');defines=['-Dnfs_host_address=nfs_raw_host_address','-Dnfs_host_native_span=nfs_raw_host_native_span']if name=='edges'else[]
            phase(name+'_memory_compile');checked([compiler,*flags,*includes,*defines,'-c',str(folder/'EngineMemory.cpp'),'-o',str(obj)],deadline)
            binary=folder/name
            phase(name+'_oracle_compile');checked([compiler,*flags,*includes,str(folder/oracle),str(obj),str(folder/'GameTrace.cpp'),'-o',str(binary)],deadline)
            if not local:macho(binary,2)
            phase(name+'_oracle_run');results[name]=json.loads(checked([str(binary)],deadline,30))
        phase('oracle_result_verification');report['sanitizedResults']=verify_results(results,49256 if local else 49280)
        if not local:
            phase('ios_sdk_verification')
            if checked(['xcrun','--sdk','iphoneos','--show-sdk-version'],deadline).strip()!=b'18.5':raise ValueError('Pinned iOS SDK required')
            sdk=checked(['xcrun','--sdk','iphoneos','--show-sdk-path'],deadline).decode().strip()
            access=folder/'access.cpp';access.write_text('#include "CachedFrameTransaction.h"\n#include "EngineMemory.h"\nextern "C" __attribute__((noinline)) uint32_t nfs_frame_probe_kernel(uint32_t a){nfs::CachedFrameTransaction tx{a};auto first=tx.load<uint32_t>(a);tx.store<uint32_t>(a+4,first);return tx.load<uint32_t>(a+4);}\nint main(){nfs::EngineMemory memory(65536);auto*p=memory.allocate(128);return int(nfs_frame_probe_kernel(memory.encode(p)));}\n')
            iosflags=['-std=c++14','-O3','-fwrapv','-fno-strict-aliasing','-frounding-math','-target','arm64-apple-ios16.0','-isysroot',sdk,*includes]
            objects=[]
            for name in ['EngineMemory.cpp','GameTrace.cpp','access.cpp']:
                obj=folder/(name+'.o');phase('ios_'+name+'_compile');checked([compiler,*iosflags,'-c',str(folder/name),'-o',str(obj)],deadline);objects.append(obj);macho(obj,1,2,16<<16,(0,(18<<16)|(5<<8)))
            binary=folder/'ios-frame-probe';phase('ios_link');checked([compiler,*iosflags,*map(str,objects),'-o',str(binary)],deadline)
            phase('ios_linked_metadata');report['iosLinkedProbe']=macho(binary,2,2,16<<16,(18<<16)|(5<<8))
            # Collect the actual linked kernel, rather than inferring code shape from source.
            phase('ios_linked_kernel');assembly=checked(['xcrun','otool','-tvV',str(binary)],deadline).decode();start=assembly.find('_nfs_frame_probe_kernel:')
            if start<0:raise ValueError('Linked kernel missing')
            body=assembly[start:];end=body.find('\n_',1)
            if end>=0:body=body[:end]
            if len(body)>65536:raise ValueError('Linked kernel exceeds bound')
            report['linkedIosKernel']=body;report['iosSdk']='18.5';report['iosMinimum']='16.0';report['iosObjectsCompiled']=True
    sources();report['complete']=True
    return report

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--local-safety-check',action='store_true');args=parser.parse_args()
    try:print(json.dumps(execute(args.local_safety_check),sort_keys=True,indent=2))
    except (OSError,ValueError,KeyError,struct.error,subprocess.SubprocessError)as error:
        print(json.dumps(dict(type='probe_failure',phase=CURRENT_PHASE,reason=safe_detail(error))),flush=True)
        parser.exit(1,'Frame-cache probe failed; inspect the bounded phase diagnostic.\n')
if __name__=='__main__':main()
