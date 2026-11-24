# SPDX-License-Identifier: MIT
"""Same-process synthetic cache timing; not game FPS, battery or promotion."""
import argparse
import hashlib
import json
from pathlib import Path
import platform
import struct
import re
import shutil
import statistics
import subprocess
import sys
import tempfile
import time
import apple_frame_cache_probe as probe
ROOT=Path(__file__).resolve().parents[1]
MANIFEST=ROOT/'tests/fixtures/frame_cache_timing/source-provenance.json'
EXPECTED=probe.EXPECTED|{'tests/frame_cache_timing.cpp'}
EXPECTED_DIGEST=12253948951681505686 # Independent reference-model preflight, not a credential.
GATES=dict(identityDifferencePercent=2,controlSpreadPercent=5,minimumPrimaryMedianCpuGainBothOrdersPercent=10,maximumAnyMedianCpuOrWallRegressionPercent=5,maximumDeferredRegressionPercent=5,cpuRawRatio=1.005,withinRunHalfClockRateDifferencePercent=.5,minimumCalibrationCpuNs=200000000,maximumCalibrationCpuNs=220000000,maximumCalibrationRefinements=5,calibrateDeferredSeparately=True,normalCells=6,negativeBlocks=2,normalCandidateBlocks=3,deferredCells=4,deferredBlocks=2,warmupNormalCpuSeconds=2,warmupDeferredCpuSeconds=1,timingBudgetSeconds=90,noOutlierRemoval=True,productionPromotionAllowed=False)

def sources():
    rows=json.loads(MANIFEST.read_text())['sources']
    if len(rows)!=19 or {x['file']for x in rows}!=EXPECTED:raise ValueError('Timing source closure mismatch')
    if {x['file']:x['sha256']for x in rows if x['file']!='tests/frame_cache_timing.cpp'}!={x['file']:x['sha256']for x in probe.sources()}:raise ValueError('Underlying cache closure changed')
    names=set()
    for row in rows:
        p=ROOT/row['file']
        if p.is_symlink()or not p.is_file()or probe.sha(p)!=row['sha256']or p.name in names:raise ValueError('Timing source drift')
        names.add(p.name)
    return rows

def verification(row,local=False):
    expected=dict(type='verification',cases=114,referenceStateMatched=True,stopAndFaultPrecedence=True,sameAllocation=True)
    if set(row)!=set(expected)|{'digest'}or any(row.get(k)!=v or type(row[k])is not type(v)for k,v in expected.items()):raise ValueError('Timing oracle coverage mismatch')
    if type(row['digest'])is not int or not 0<=row['digest']<2**64:raise ValueError('Timing oracle digest invalid')
    if EXPECTED_DIGEST is None:
        if not local:raise ValueError('Freeze reference digest before native dispatch')
    elif row['digest']!=EXPECTED_DIGEST:raise ValueError('Timing oracle digest mismatch')
    return row

def analyze(events):
    if not events or events[0]!=dict(type='study_start',sameProcessAndAllocation=True,negativeFunctionPointersEqual=True,threadedProcess=True):raise ValueError('Shared-process identity missing')
    if sum(e.get('type')=='study_start'for e in events)!=1 or sum(e.get('type')=='study_complete'for e in events)!=1:raise ValueError('Ambiguous study boundaries')
    end=events[-1]
    if end.get('type')!='study_complete'or end.get('stopStateRestored')is not True:raise ValueError('Study incomplete or stop state not restored')
    allowed={'type','phase','cell','mode','words','stopMode','block','order','variant','iterations','cpuNs','rawNs','wallNs','firstRawNs','firstWallNs','clockRateDriftPercent','qualityPassed','checksum','stateVerified'}
    if any(e.get('type')not in ('study_start','study_complete','calibration','warmup','measurement')for e in events):raise ValueError('Unknown study event')
    timing=[e for e in events if e.get('type')in ('calibration','warmup','measurement')]
    for e in timing:
        if set(e)!=allowed or e['stateVerified']is not True:raise ValueError('Unexpected timing row or invalid state')
        if any(type(e[k])is not int or not 0<=e[k]<2**64 for k in allowed-{'type','clockRateDriftPercent','qualityPassed','stateVerified'}):raise ValueError('Invalid timing integer')
        phase=e['phase'];cell=e['cell'];mode=0 if phase==2 else cell//2;words=16 if cell&1 else 8;stop=1+cell//2 if phase==2 else 0
        if phase not in (0,1,2)or cell>=(4 if phase==2 else 6)or(e['mode'],e['words'],e['stopMode'])!=(mode,words,stop):raise ValueError('Timing cell mismatch')
        if e['type']=='calibration'and(phase not in (0,2)or e['variant']!=0):raise ValueError('Calibration variant mismatch')
        if not e['iterations']or e['iterations']%2 or e['iterations']>8388608 or not 0<e['firstRawNs']<e['rawNs']or not 0<e['firstWallNs']<e['wallNs']or not e['cpuNs']:raise ValueError('Timing duration/count outside bound')
        first=e['firstRawNs']/e['firstWallNs'];second=(e['rawNs']-e['firstRawNs'])/(e['wallNs']-e['firstWallNs']);drift=100*abs(first-second)/min(first,second)
        good=e['cpuNs']<=1.005*e['rawNs']and drift<=.5
        if type(e['qualityPassed'])is not bool or good!=e['qualityPassed']:raise ValueError('Timing quality not reproduced')
    measurements=[e for e in timing if e['type']=='measurement'];controls=[];candidate=[]
    def group(phase,cell,block):
        rows=sorted([e for e in measurements if(e['phase'],e['cell'],e['block'])==(phase,cell,block)],key=lambda e:e['order'])
        mode=0 if phase==2 else cell//2;words=16 if cell&1 else 8;stop=1+cell//2 if phase==2 else 0
        if len(rows)!=4 or [e['order']for e in rows]!=[0,1,2,3]or [e['variant']for e in rows]!=([0,1,1,0]if phase==0 else[0,2,2,0]):raise ValueError('Balanced order mismatch')
        if any((e['mode'],e['words'],e['stopMode'])!=(mode,words,stop)for e in rows)or len({e['iterations']for e in rows})!=1 or len({e['checksum']for e in rows})!=1:raise ValueError('Workload/checksum mismatch')
        return rows
    for cell in range(6):
        calibration=[e for e in timing if e['type']=='calibration'and e['phase']==0 and e['cell']==cell]
        if not 1<=len(calibration)<=5 or not 200000000<=calibration[-1]['cpuNs']<=220000000:raise ValueError('Calibration band not reproduced')
        if any(e['iterations']!=calibration[-1]['iterations']for e in measurements if e['phase']in (0,1)and e['cell']==cell):raise ValueError('Measured count differs from calibration')
        blocks=[];outer=[]
        for block in range(2):
            rows=group(0,cell,block);v=[e['cpuNs']for e in rows];a=(v[0]+v[3])/2;b=(v[1]+v[2])/2;difference=100*abs(b-a)/a;spread=100*abs(v[0]-v[3])/min(v[0],v[3]);quality=all(e['qualityPassed']for e in rows);outer.extend([v[0],v[3]])
            blocks.append(dict(block=block,identityDifferencePercent=difference,controlSpreadPercent=spread,qualityPassed=quality,passed=difference<=2 and spread<=5 and quality))
        spread=100*(max(outer)-min(outer))/min(outer);controls.append(dict(cell=cell,blocks=blocks,pooledControlSpreadPercent=spread,passed=spread<=5 and all(x['passed']for x in blocks)))
    qualified=all(x['passed']for x in controls)
    if type(end.get('timingControlQualified'))is not bool or end['timingControlQualified']!=qualified or end.get('candidateTimed')is not qualified:raise ValueError('Executable control gate not independently reproduced')
    if len(measurements)!=(152 if qualified else 48)or end.get('measurements')!=len(measurements):raise ValueError('Measurement set incomplete')
    for phase,cells in ([(0,6),(1,6),(2,4)]if qualified else[(0,6)]):
        for cell in range(cells):
            for variant in ([0]if phase==0 else[0,2]):
                warm=[e for e in timing if e['type']=='warmup'and(e['phase'],e['cell'],e['variant'])==(phase,cell,variant)]
                if sum(e['cpuNs']for e in warm)<(1000000000 if phase==2 else 2000000000):raise ValueError('Declared warmup incomplete')
    if not qualified and any(e['phase']for e in timing):raise ValueError('Candidate ran after identity failure')
    if qualified:
        for cell in range(4):
            calibration=[e for e in timing if e['type']=='calibration'and e['phase']==2 and e['cell']==cell]
            if not 1<=len(calibration)<=5 or not 200000000<=calibration[-1]['cpuNs']<=220000000:raise ValueError('Deferred calibration band not reproduced')
            if any(e['iterations']!=calibration[-1]['iterations']for e in measurements if e['phase']==2 and e['cell']==cell):raise ValueError('Deferred count differs from calibration')
        for phase,cells,blocks in [(1,6,3),(2,4,2)]:
            for cell in range(cells):
                forward=[];reverse=[];wall=[];cpu=[];outer=[];quality=True
                for block in range(blocks):
                    rows=group(phase,cell,block);v=[e['cpuNs']for e in rows];w=[e['wallNs']for e in rows];forward.append(100*(v[0]-v[1])/v[0]);reverse.append(100*(v[3]-v[2])/v[3]);cpu.extend([-forward[-1],-reverse[-1]]);wall.extend([100*(w[1]-w[0])/w[0],100*(w[2]-w[3])/w[3]]);outer.extend([v[0],v[3]]);quality&=all(e['qualityPassed']for e in rows)
                spread=100*(max(outer)-min(outer))/min(outer);f=statistics.median(forward);rev=statistics.median(reverse);cr=statistics.median(cpu);wr=statistics.median(wall);primary=phase==1 and cell//2<2
                passed=quality and spread<=5 and cr<=5 and wr<=5 and(not primary or(f>=10 and rev>=10))
                candidate.append(dict(phase=phase,cell=cell,primary=primary,forwardMedianCpuGainPercent=f,reverseMedianCpuGainPercent=rev,medianCpuRegressionPercent=cr,medianWallRegressionPercent=wr,pooledControlSpreadPercent=spread,qualityPassed=quality,passed=passed))
    return dict(scope='Synthetic native component timing only; no game/device/energy acceptance',timingControlQualified=qualified,candidateTimed=qualified,measurementRows=len(measurements),controlCells=controls,candidateCells=candidate,throughputQualified=qualified and bool(candidate)and all(x['passed']for x in candidate),productionPromotionAllowed=False,noOutlierRemoval=True)

def linked_text(output):
    text=output.decode();match=re.search(r'(?m)^_[A-Za-z0-9_.$]+:\s*$',text)
    if not match:raise ValueError('Linked function text missing')
    text=text[match.start():]
    if len(text)>2*1024*1024 or not all(name+':'in text for name in ['_nfs_frame_control','_nfs_frame_candidate']):raise ValueError('Linked roots missing or text exceeds bound')
    return text

def execute(local=False):
    deadline=time.monotonic()+240;probe.phase('timing_source_review');rows=sources()
    if not local and(sys.platform!='darwin'or platform.machine()!='arm64'):raise ValueError('Native Apple ARM host required')
    if local and sys.platform!='linux':raise ValueError('Linux local preflight required')
    compiler=shutil.which('clang++')if local else probe.checked(['xcrun','--find','clang++'],deadline).decode().strip()
    if not compiler:raise ValueError('Existing compiler unavailable')
    target=[]if local else['-arch','arm64','-isysroot',probe.checked(['xcrun','--sdk','macosx','--show-sdk-path'],deadline).decode().strip()]
    flags=['-std=c++14','-pthread','-fwrapv','-fno-strict-aliasing','-frounding-math']
    report=dict(schema=1,scope='Synthetic same-process cache study; not game FPS, battery or promotion',architecture=platform.machine(),localSafetyOnly=local,sources=rows,gates=GATES,appBuilt=False,iosExecuted=False,productionChanged=False)
    with tempfile.TemporaryDirectory(prefix='nfs-frame-timing-')as folder:
        folder=Path(folder)
        for row in rows:(folder/Path(row['file']).name).write_bytes((ROOT/row['file']).read_bytes())
        units=[str(folder/n)for n in ['frame_cache_timing.cpp','EngineMemory.cpp','GameTrace.cpp']];include=['-I'+str(folder)];binary=folder/'sanitized'
        probe.phase('timing_sanitized_compile');probe.checked([compiler,*flags,*target,*include,'-O1','-g','-fsanitize=address,undefined','-fno-sanitize-recover=all',*units,'-o',str(binary)],deadline)
        report['sanitizedVerification']=verification(json.loads(probe.checked([str(binary),'--verify'],deadline,30)),local)
        binary=folder/'optimized';lto=['-flto=thin','-Wl,-plugin-opt,jobs=2']if local else['-flto=thin','-Wl,-mllvm,-threads=2']
        probe.phase('timing_optimized_compile');probe.checked([compiler,*flags,*target,*include,'-O3',*lto,*units,'-o',str(binary)],deadline)
        report['optimizedVerification']=verification(json.loads(probe.checked([str(binary),'--verify'],deadline,30)),local)
        if report['sanitizedVerification']!=report['optimizedVerification']:raise ValueError('Optimized verification differs')
        if local:
            denied=subprocess.run([str(binary),'--study'],capture_output=True,timeout=10)
            if denied.returncode!=2 or b'local timing disabled'not in denied.stderr:raise ValueError('Local performance was not disabled')
            report['localTimingDisabledVerified']=True
        else:
            if probe.checked(['xcrun','--sdk','iphoneos','--show-sdk-version'],deadline).strip()!=b'18.5':raise ValueError('Pinned iOS SDK required')
            sdk=probe.checked(['xcrun','--sdk','iphoneos','--show-sdk-path'],deadline).decode().strip();ios=folder/'ios-helper';exports=['-Wl,-exported_symbol,'+n for n in ['_main','_nfs_frame_control','_nfs_frame_candidate']]
            probe.phase('timing_ios_link');probe.checked([compiler,*flags,*include,'-target','arm64-apple-ios16.0','-isysroot',sdk,'-O3',*lto,*exports,*units,'-o',str(ios)],deadline,90)
            report['iosPlatform']=probe.macho(ios,2,2,16<<16,(18<<16)|(5<<8));report['linkedIosText']=linked_text(probe.checked(['xcrun','otool','-tvV',str(ios)],deadline)).replace(str(folder),'<probe>');report['linkedTextPathRedacted']=True;report['linkedProjectBodiesCollected']=True;report['externalLibraryBodiesNotCollected']=True
            probe.phase('timing_native_measurement')
            remaining=min(100,deadline-time.monotonic());p=subprocess.run([str(binary),'--study'],capture_output=True,timeout=remaining)
            if max(len(p.stdout),len(p.stderr))>2*1024*1024:raise ValueError('Native output bound exceeded')
            for line in p.stdout.decode().splitlines():print(line,flush=True)
            if p.returncode:raise ValueError('Native study incomplete: '+probe.safe_detail(p.stderr))
            events=[json.loads(line)for line in p.stdout.decode().splitlines()];report['events']=events;report['analysis']=analyze(events)
    sources();report['complete']=True
    return report

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--local-safety-check',action='store_true');args=parser.parse_args()
    try:print(json.dumps(dict(type='frame_timing_result',result=execute(args.local_safety_check)),sort_keys=True))
    except(OSError,ValueError,KeyError,struct.error,subprocess.SubprocessError)as error:
        print(json.dumps(dict(type='probe_failure',phase=probe.CURRENT_PHASE,reason=probe.safe_detail(error))),flush=True);parser.exit(1,'Frame timing probe failed its declared checks.\n')
if __name__=='__main__':main()
