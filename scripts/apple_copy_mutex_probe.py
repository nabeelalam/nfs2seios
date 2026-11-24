"""Bounded source-only Apple copy-lock proxy; no game, assets, app or energy claim."""
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
SOURCE=ROOT/'tests/guest_copy_lock_comparison.cpp'
GATES=dict(minimumMedianCpuGainPercentBothOrders=10,maximumWorkloadMedianCpuOrWallRegressionPercent=5,maximumControlCpuSpreadPercent=5,maximumSampledWaitNs=50000000,proxyOnly=True,productionPromotionAllowed=False)

def checked(args,deadline,timeout=60):
    remaining=deadline-time.monotonic()
    if remaining<=0:raise ValueError('Probe deadline exhausted')
    p=subprocess.run(args,capture_output=True,timeout=min(timeout,remaining))
    if p.returncode or max(len(p.stdout),len(p.stderr))>1048576:raise ValueError('Probe command failed or exceeded output bound')
    return p.stdout

def parse(output,backends,rounds,scale):
    lines=output.decode().splitlines();checks=[];rows=[];waits=[]
    for line in lines:
        if line.startswith('CHECK '):
            m=re.fullmatch(r'CHECK backend=(\w+) shared_updates=80000 concurrent_owners=0 exception_cleanup=pass',line)
            if not m:raise ValueError('Shared exclusion check mismatch')
            checks.append(m[1])
        elif line.startswith('BENCH '):
            m=re.fullmatch(r'BENCH backend=(\w+) workload=(\w+) round=(\d+) seconds=([\d.]+) operations=(\d+) ns_per_operation=([\d.]+) cpu_ns=(\d+) cpu_ns_per_operation=([\d.]+)',line)
            if not m:raise ValueError('Benchmark record mismatch')
            backend,workload,round_id,seconds,ops,wall,cpu,cpu_op=m.groups();row=dict(backend=backend,workload=workload,round=int(round_id),seconds=float(seconds),operations=int(ops),wallNsPerOperation=float(wall),cpuNs=int(cpu),cpuNsPerOperation=float(cpu_op))
            expected={'uncontended':scale,'two_equal_workers':2*(scale//4),'eight_to_one_workers':scale+scale//8}
            if workload not in expected or row['operations']!=expected[workload] or min(row['seconds'],row['wallNsPerOperation'],row['cpuNs'],row['cpuNsPerOperation'])<=0:raise ValueError('Workload accounting mismatch')
            if abs(row['cpuNs']/row['operations']-row['cpuNsPerOperation'])>.001:raise ValueError('CPU accounting mismatch')
            if abs(row['seconds']*1e9/row['operations']-row['wallNsPerOperation'])>.06:raise ValueError('Wall accounting mismatch')
            rows.append(row)
        elif line.startswith('WAIT '):
            m=re.fullmatch(r'WAIT backend=(\w+) workload=(\w+) round=(\d+) worker=(\d+) max_sampled_wait_ns=(\d+)',line)
            if not m:raise ValueError('Wait record mismatch')
            backend,workload,round_id,worker,ns=m.groups();waits.append(dict(backend=backend,workload=workload,round=int(round_id),worker=int(worker),maximumSampledWaitNs=int(ns)))
    if sorted(checks)!=sorted(backends):raise ValueError('Missing shared exclusion result')
    expected={(b,w,n)for b in backends for w in ['uncontended','two_equal_workers','eight_to_one_workers']for n in range(rounds)}
    if len(rows)!=len(expected) or {(r['backend'],r['workload'],r['round'])for r in rows}!=expected:raise ValueError('Missing or duplicate benchmark records')
    expected_wait={(b,w,n,worker)for b,w,n in expected for worker in range(1 if w=='uncontended'else 2)}
    if len(waits)!=len(expected_wait) or {(r['backend'],r['workload'],r['round'],r['worker'])for r in waits}!=expected_wait:raise ValueError('Missing or duplicate worker wait records')
    if not lines or lines[-1]!='PASS: packed payload counts and exception cleanup; no game assets used':raise ValueError('Terminal check missing')
    return dict(sharedExclusionPassed=True,rows=rows,waits=waits)

def execute(local=False):
    deadline=time.monotonic()+240
    if local:
        if sys.platform!='linux':raise ValueError('Local safety requires Linux')
        compiler=shutil.which('clang++');extra=[]
        if not compiler:raise ValueError('Existing compiler missing')
    else:
        if sys.platform!='darwin' or platform.machine()!='arm64':raise ValueError('Native Apple ARM host required')
        if checked(['xcodebuild','-version'],deadline).splitlines()[0]!=b'Xcode 16.4':raise ValueError('Pinned Xcode required')
        compiler=checked(['xcrun','--sdk','macosx','--find','clang++'],deadline).decode().strip();sdk=checked(['xcrun','--sdk','macosx','--show-sdk-path'],deadline).decode().strip();extra=['-arch','arm64','-isysroot',sdk]
    source=SOURCE.read_bytes();report=dict(schema=1,scope='Source-only lock correctness and native Mac proxy costs, never game FPS or battery evidence',localSafetyOnly=local,architecture=platform.machine(),gates=GATES,sourceProvenance=[dict(file=p.relative_to(ROOT).as_posix(),sha256=hashlib.sha256(p.read_bytes()).hexdigest())for p in [Path(__file__).resolve(),SOURCE]],appBuilt=False,iosExecuted=False,productionChanged=False)
    with tempfile.TemporaryDirectory(prefix='nfs-copy-mutex-')as folder:
        folder=Path(folder);binary=folder/'safety';flags=['-std=c++17','-pthread','-Wall','-Wextra','-Werror']
        checked([compiler,*flags,*extra,'-O1','-g','-fsanitize=address,undefined','-fno-sanitize-recover=all',str(SOURCE),'-o',str(binary)],deadline)
        if not local:
            if struct.unpack_from('<IIII',binary.read_bytes())[:2]!=(0xfeedfacf,0x0100000c):raise ValueError('Native ARM executable mismatch')
        report['sanitizedSafety']=parse(checked([str(binary),'--quick-check'],deadline,30),['std_mutex']if local else['std_mutex','os_unfair_lock'],1,20000)
        if not local:
            binary=folder/'comparison';checked([compiler,*flags,*extra,'-O3',str(SOURCE),'-o',str(binary)],deadline)
            if struct.unpack_from('<IIII',binary.read_bytes())[:2]!=(0xfeedfacf,0x0100000c):raise ValueError('Native ARM executable mismatch')
            report['nativeProxy']=parse(checked([str(binary)],deadline,120),['std_mutex','os_unfair_lock'],6,2000000)
            if checked(['xcrun','--sdk','iphoneos','--show-sdk-version'],deadline).strip()!=b'18.5':raise ValueError('Pinned iOS SDK required')
            sdk=checked(['xcrun','--sdk','iphoneos','--show-sdk-path'],deadline).decode().strip();obj=folder/'ios.o';checked([compiler,*flags,'-O3','-target','arm64-apple-ios16.0','-isysroot',sdk,'-c',str(SOURCE),'-o',str(obj)],deadline)
            magic,cpu,_,kind=struct.unpack_from('<IIII',obj.read_bytes())
            if(magic,cpu,kind)!=(0xfeedfacf,0x0100000c,1):raise ValueError('iOS ARM object mismatch')
            report['iosArm64PublicLockObjectCompiled']=True
    if SOURCE.read_bytes()!=source:raise ValueError('Source changed during probe')
    report['complete']=True
    return report

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--local-safety-check',action='store_true');args=parser.parse_args()
    try:print(json.dumps(execute(args.local_safety_check),sort_keys=True,indent=2))
    except(OSError,ValueError,KeyError,struct.error,subprocess.SubprocessError):parser.exit(1,'Copy-mutex probe failed platform, correctness, accounting or time bounds.\n')
if __name__=='__main__':main()
