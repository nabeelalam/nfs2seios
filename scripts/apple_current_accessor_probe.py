"""Frozen current-resolver ARM accessor study; no app, assets or promotion."""
import argparse
import hashlib
import json
from pathlib import Path
import platform
import re
import shutil
import struct
import subprocess
import sys
import tempfile
import time
ROOT=Path(__file__).resolve().parents[1]
FIXTURE=ROOT/'tests/fixtures/control_slot_cold'
MANIFEST=FIXTURE/'source-provenance.json'
VARIANTS=('control','candidate')
WORDS=(1,4,8,16,32)
EXPECTED={'src/GuestCopyTransaction.h','src/GuestAccess.h','src/GuestRuntimeC.h','src/WorkerControl.h','src/EngineMemory.h','src/HeapArena.h','src/GameMemory.h','src/GameTrace.cpp','src/GameTrace.h','src/CodecOperations.h','tests/fixtures/owned_chunk/synthetic/DATA.h','tests/fixtures/owned_chunk/synthetic/BSS.h','tests/fixtures/control_slot_cold/GuestCopyTransaction.h','tests/fixtures/control_slot_cold/EngineMemory.cpp','tests/current_accessor_real_kernels.cpp','tests/current_accessor_real_runtime.cpp'}
GATES=dict(rounds=6,iterationsPerWord=1000000,warmupsPerNormalCell=1,nativeMeasurementBudgetSeconds=90,backendOrder='control/candidate on even rounds, candidate/control on odd rounds',primaryWords=[8,16,32],minimumPrimaryMedianCpuGainPercentBothOrders=10,maximumAnyMedianCpuOrWallRegressionPercent=5,maximumControlCpuSpreadPercent=5,deferredRounds=2,deferredWords=[8,32],maximumDeferredMedianCpuRegressionPercent=5,noOutlierRemoval=True,unchangedRetry=False,productionPromotionAllowed=False)
PROBES=('_nfs_current_heap','_nfs_current_native','_nfs_current_mixed')
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def emit(kind,**data):print(json.dumps(dict(type=kind,**data)),flush=True)
def checked(args,deadline,timeout=60):
    left=deadline-time.monotonic()
    if left<=0:raise ValueError('Study deadline exhausted')
    p=subprocess.run(args,capture_output=True,timeout=min(left,timeout))
    if p.returncode or max(len(p.stdout),len(p.stderr))>2*1024*1024:raise ValueError('Study command failed or output exceeded bound')
    return p.stdout

def sources():
    rows=json.loads(MANIFEST.read_text())['sources']
    if len(rows)!=len(EXPECTED)or{r['file']for r in rows}!=EXPECTED:raise ValueError('Frozen source set mismatch')
    for row in rows:
        p=ROOT/row['file']
        if p.is_symlink()or not p.is_file()or sha(p)!=row['sha256']:raise ValueError('Frozen source mismatch')
    return rows

def materialize(folder,rows):
    for variant in VARIANTS:
        target=folder/variant;target.mkdir()
        for row in rows:
            if Path(row['file']).name=='GuestCopyTransaction.h':continue
            p=target/Path(row['file']).name
            if p.exists():raise ValueError('Duplicate closure name')
            p.write_bytes((ROOT/row['file']).read_bytes())
        source=ROOT/'src/GuestCopyTransaction.h'if variant=='control'else FIXTURE/'GuestCopyTransaction.h'
        (target/'GuestCopyTransaction.h').write_bytes(source.read_bytes())

def verification(output,variant):
    row=json.loads(output)
    expected=dict(type='verification',transactionBytes=16 if variant=='control'else 24,cases=300,checksum=9907831787116055075,actualEngineMemory=True,overlapAndStopPrecedenceVerified=True)
    if row!=expected or any(type(row[k])is not type(v)for k,v in expected.items()):raise ValueError('Actual-resolver correctness or layout mismatch')
    return row

def measurement(output,mode,words,iterations,stop):
    row=json.loads(output)
    expected=dict(type='measurement',threadedProcess=True,mode=mode,words=words,iterations=iterations,stopMode=stop)
    if set(row)!=set(expected)|{'cpuNs','wallNs','checksum'}or any(row.get(k)!=v or type(row[k])is not type(v)for k,v in expected.items()):raise ValueError('Measurement workload mismatch')
    if any(type(row[k])is not int or not 0<row[k]<2**64 for k in ['cpuNs','wallNs']):raise ValueError('Invalid measured duration')
    if type(row['checksum'])is not int or not 0<=row['checksum']<2**64:raise ValueError('Invalid measured checksum')
    return row

def macho_platform(path,platform_id,minimum,sdk):
    if not 32<=path.stat().st_size<=8*1024*1024:raise ValueError('Mach-O size outside bound')
    data=path.read_bytes()
    if not 32<=len(data)<=8*1024*1024:raise ValueError('Mach-O size outside bound')
    magic,cpu,_,kind,commands,size=struct.unpack_from('<IIIIII',data)
    if(magic,cpu,kind)!=(0xfeedfacf,0x0100000c,2)or not 0<commands<=256 or size>len(data)-32:raise ValueError('Native ARM executable mismatch')
    cursor=32;limit=cursor+size;builds=[]
    for _ in range(commands):
        if cursor+8>limit:raise ValueError('Mach-O command truncated')
        kind,length=struct.unpack_from('<II',data,cursor)
        if length<8 or length%8 or length>limit-cursor:raise ValueError('Mach-O command outside bound')
        if kind==0x32:
            if length<24:raise ValueError('Mach-O build command truncated')
            target,version,sdk_version,tools=struct.unpack_from('<IIII',data,cursor+8)
            if tools>32 or length!=24+8*tools:raise ValueError('Mach-O build tools outside bound')
            builds.append((target,version,sdk_version))
        cursor+=length
    if cursor!=limit or builds!=[(platform_id,minimum,sdk)]:raise ValueError('Mach-O target/SDK mismatch')
    return dict(architecture='arm64',platform=platform_id,minimum=minimum,sdk=sdk,filetype='MH_EXECUTE')

def selected_bodies(output,variant):
    if len(output)>2*1024*1024:raise ValueError('Code output exceeds bound')
    text=output.decode();bodies={};name=None
    for line in text.splitlines():
        label=re.fullmatch(r'(_[A-Za-z0-9_.$]+):\s*',line)
        if label:
            candidate=label[1];name=candidate if candidate in PROBES or 'handle_requested_stop'in candidate else None
            if name:bodies[name]=[]
        elif name:
            if len(line)>2048 or len(bodies[name])>=2048:raise ValueError('Selected code body exceeds bound')
            bodies[name].append(line)
    if not set(PROBES)<=set(bodies)or any(not bodies[n]for n in PROBES):raise ValueError('Required linked kernels missing')
    if variant=='candidate'and not any('handle_requested_stop'in n for n in bodies):raise ValueError('Outlined true-stop helper missing')
    return {n:'\n'.join(lines)for n,lines in bodies.items()}

def execute(local=False):
    deadline=time.monotonic()+240
    if local:
        if sys.platform!='linux':raise ValueError('Local safety requires Linux')
        compiler=shutil.which('clang++');target=[]
        if not compiler:raise ValueError('Existing compiler missing')
    else:
        if sys.platform!='darwin'or platform.machine()!='arm64':raise ValueError('Native Apple ARM host required')
        if checked(['xcodebuild','-version'],deadline).splitlines()[0]!=b'Xcode 16.4':raise ValueError('Pinned Xcode required')
        compiler=checked(['xcrun','--sdk','macosx','--find','clang++'],deadline).decode().strip();mac_sdk=checked(['xcrun','--sdk','macosx','--show-sdk-path'],deadline).decode().strip();target=['-target','arm64-apple-macos11.0','-isysroot',mac_sdk]
        if checked(['xcrun','--sdk','iphoneos','--show-sdk-version'],deadline).strip()!=b'18.5':raise ValueError('Pinned iOS SDK required')
        ios_sdk=checked(['xcrun','--sdk','iphoneos','--show-sdk-path'],deadline).decode().strip()
    rows=sources();provenance=rows+[dict(file=p.relative_to(ROOT).as_posix(),sha256=sha(p))for p in [MANIFEST,Path(__file__).resolve()]]
    emit('study_identity',sources=provenance,localSafetyOnly=local,gates=GATES)
    report=dict(scope='Actual current-resolver finite ARM accessor study; not game FPS, battery or promotion evidence',architecture=platform.machine(),localSafetyOnly=local,sourceProvenance=provenance,gates=GATES,safety={},iosCodegen={},measurements=[],warmups=[],productionChanged=False,appBuilt=False,iosExecuted=False,linkedNormalPathProofPerformed=False)
    with tempfile.TemporaryDirectory(prefix='nfs-current-accessor-')as temporary:
        folder=Path(temporary)
        if shutil.disk_usage(folder).free<6*1024**3:raise ValueError('Compile free-space bound failed')
        materialize(folder,rows);binaries={}
        for variant in VARIANTS:
            stage=folder/variant;units=[stage/n for n in ['current_accessor_real_runtime.cpp','current_accessor_real_kernels.cpp','EngineMemory.cpp','GameTrace.cpp']]
            flags=['-std=c++14','-pthread','-fwrapv','-fno-strict-aliasing','-I'+str(stage),'-ffile-prefix-map='+str(folder)+'=/probe','-fdebug-prefix-map='+str(folder)+'=/probe']
            binary=stage/'safety';checked([compiler,*flags,*target,'-O1','-g','-fsanitize=address,undefined','-fno-sanitize-recover=all',*map(str,units),'-o',str(binary)],deadline)
            report['safety'][variant]=verification(checked([str(binary),'--verify'],deadline,30),variant);emit('verified_variant',variant=variant,result=report['safety'][variant])
            smoke=measurement(checked([str(binary),'--measure','0','1','1000','0'],deadline,10),0,1,1000,0)
            if smoke['checksum']!=2087410249000:raise ValueError('Parked peer functional smoke mismatch')
            if local:continue
            binary=stage/'native';checked([compiler,*flags,*target,'-O3','-flto=thin','-Wl,-mllvm,-threads=2',*map(str,units),'-o',str(binary)],deadline)
            header=struct.unpack_from('<IIII',binary.read_bytes())
            if(header[0],header[1],header[3])!=(0xfeedfacf,0x0100000c,2):raise ValueError('Native ARM program mismatch')
            verification(checked([str(binary),'--verify'],deadline,30),variant);binaries[variant]=binary
            ios=stage/'ios-helper';exports=['-Wl,-exported_symbol,'+name for name in PROBES]
            checked([compiler,*flags,'-target','arm64-apple-ios16.0','-isysroot',ios_sdk,'-O3','-flto=thin','-Wl,-mllvm,-threads=2',*exports,*map(str,units),'-o',str(ios)],deadline)
            platform_record=macho_platform(ios,2,16<<16,(18<<16)|(5<<8));bodies=selected_bodies(checked(['xcrun','otool','-tvV',str(ios)],deadline),variant)
            report['iosCodegen'][variant]=dict(platform=platform_record,thinLto=True,backendJobs=2,selectedBodies=bodies,descriptiveOnly=True);emit('ios_codegen',variant=variant,result=report['iosCodegen'][variant])
        if not local:
            native_deadline=min(deadline,time.monotonic()+GATES['nativeMeasurementBudgetSeconds']);checksums={}
            def measure(variant,mode,words,stop):
                count=GATES['iterationsPerWord']//words
                row=measurement(checked([str(binaries[variant]),'--measure',str(mode),str(words),str(count),str(stop)],native_deadline,10),mode,words,count,stop)
                key=(mode,words,stop)
                if key in checksums and checksums[key]!=row['checksum']:raise ValueError('Cross-variant workload checksum mismatch')
                checksums[key]=row['checksum'];return dict(variant=variant,**row)
            for variant in VARIANTS:
                for mode in range(3):
                    for words in WORDS:report['warmups'].append(measure(variant,mode,words,0))
            for round_id in range(GATES['rounds']):
                order=VARIANTS if round_id%2==0 else tuple(reversed(VARIANTS))
                for mode in range(3):
                    for words in WORDS:
                        for variant in order:
                            row=dict(round=round_id,**measure(variant,mode,words,0));report['measurements'].append(row);emit('accessor_measurement',result=row)
            for round_id in range(GATES['deferredRounds']):
                order=VARIANTS if round_id%2==0 else tuple(reversed(VARIANTS))
                for stop in [1,2]:
                    for mode in range(3):
                        for words in GATES['deferredWords']:
                            for variant in order:
                                row=dict(round=round_id,**measure(variant,mode,words,stop));report['measurements'].append(row);emit('accessor_measurement',result=row)
            if len(report['measurements'])!=228:raise ValueError('Incomplete balanced measurements')
    if sources()!=rows:raise ValueError('Frozen sources changed during study')
    report['complete']=True
    return report

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--local-safety-check',action='store_true');args=parser.parse_args()
    try:emit('study_result',result=execute(args.local_safety_check))
    except(OSError,ValueError,KeyError,TypeError,IndexError,struct.error,subprocess.SubprocessError):parser.exit(1,'Current accessor study failed its source, platform, accounting, code-shape or time bounds.\n')
if __name__=='__main__':main()
