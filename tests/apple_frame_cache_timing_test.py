# SPDX-License-Identifier: MIT
# Artificial protocol fixtures below are never performance evidence.
import importlib.util
import json
from pathlib import Path
import sys
import unittest
from unittest.mock import patch
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'scripts'))
import apple_frame_cache_timing as timing

def row(kind,phase,cell,variant=0,block=0,order=0,cpu=205000000):
    mode=0 if phase==2 else cell//2;words=16 if cell&1 else 8;stop=1+cell//2 if phase==2 else 0
    return dict(type=kind,phase=phase,cell=cell,mode=mode,words=words,stopMode=stop,block=block,order=order,variant=variant,iterations=1024,cpuNs=cpu,rawNs=cpu,wallNs=cpu,firstRawNs=cpu//2,firstWallNs=cpu//2,clockRateDriftPercent=0.,qualityPassed=True,checksum=0,stateVerified=True)

def events(qualified=True):
    out=[dict(type='study_start',sameProcessAndAllocation=True,negativeFunctionPointersEqual=True,threadedProcess=True)]
    for phase,cells,blocks in ([(0,6,2),(1,6,3),(2,4,2)]if qualified else[(0,6,2)]):
        for cell in range(cells):
            if phase in (0,2):out.append(row('calibration',phase,cell))
            for variant in ([0]if phase==0 else[0,2]):out.append(row('warmup',phase,cell,variant,cpu=1000000000 if phase==2 else 2000000000))
            for block in range(blocks):
                order=[0,1,1,0]if phase==0 else[0,2,2,0]
                for i,variant in enumerate(order):
                    cpu=174250000 if phase==1 and cell//2<2 and variant==2 else 205000000
                    if not qualified and cell==0 and block==0 and i==1:cpu=220000000
                    out.append(row('measurement',phase,cell,variant,block,i,cpu))
    out.append(dict(type='study_complete',timingControlQualified=qualified,candidateTimed=qualified,measurements=152 if qualified else 48,stopStateRestored=True))
    return out

class FrameCacheTimingGuards(unittest.TestCase):
    def test_frozen_closure_and_drift(self):
        self.assertEqual(len(timing.sources()),19)
        with patch.object(timing.probe,'sha',return_value='0'*64):
            with self.assertRaises(ValueError):timing.sources()
    def test_exact_reference_digest_and_coverage(self):
        value=dict(type='verification',cases=114,digest=12253948951681505686,referenceStateMatched=True,stopAndFaultPrecedence=True,sameAllocation=True)
        self.assertEqual(timing.verification(value),value)
        for key,new in [('digest',0),('cases',113),('sameAllocation',1)]:
            wrong=dict(value);wrong[key]=new
            with self.subTest(key=key),self.assertRaises(ValueError):timing.verification(wrong)
        with patch.object(timing,'EXPECTED_DIGEST',None):
            with self.assertRaises(ValueError):timing.verification(value)
    def test_positive_artificial_protocol_fixture(self):
        result=timing.analyze(events());self.assertTrue(result['throughputQualified']);self.assertEqual(result['measurementRows'],152);self.assertFalse(result['productionPromotionAllowed'])
    def test_negative_control_prevents_candidate_acceptance(self):
        result=timing.analyze(events(False));self.assertFalse(result['timingControlQualified']);self.assertFalse(result['candidateTimed']);self.assertFalse(result['throughputQualified'])
    def test_missing_or_duplicate_rows_rejected(self):
        original=events();i=next(i for i,x in enumerate(original)if x['type']=='measurement')
        for rows in [original[:i]+original[i+1:],original[:i]+[original[i]]+original[i:]]:
            with self.assertRaises(ValueError):timing.analyze(rows)
    def test_warmup_and_workload_cannot_be_silently_changed(self):
        original=events();i=next(i for i,x in enumerate(original)if x['type']=='warmup'and x['phase']==1)
        with self.assertRaises(ValueError):timing.analyze(original[:i]+original[i+1:])
        wrong=json.loads(json.dumps(original));next(x for x in wrong if x['type']=='measurement')['mode']=9
        with self.assertRaises(ValueError):timing.analyze(wrong)
    def test_deferred_cells_require_their_own_calibration(self):
        original=events();i=next(i for i,x in enumerate(original)if x['type']=='calibration'and x['phase']==2)
        with self.assertRaises(ValueError):timing.analyze(original[:i]+original[i+1:])
    def test_fallback_regression_and_bad_clock_quality_fail(self):
        wrong=events()
        for x in wrong:
            if x.get('type')=='measurement'and x['phase']==1 and x['mode']==2 and x['variant']==2:x.update(cpuNs=220000000,rawNs=220000000,wallNs=220000000,firstRawNs=110000000,firstWallNs=110000000)
        self.assertFalse(timing.analyze(wrong)['throughputQualified'])
        wrong=events();x=next(x for x in wrong if x.get('type')=='measurement'and x['phase']==1);x['firstRawNs']+=10000000;x['qualityPassed']=False
        self.assertFalse(timing.analyze(wrong)['throughputQualified'])
    def test_all_linked_text_kept_but_roots_required(self):
        text=b'/temporary/helper:\n(__TEXT,__text) section\n_nfs_frame_control:\n0000 ret\n_nfs_frame_candidate:\n0040 ret\n_more_project_code:\n0080 ret\n'
        body=timing.linked_text(text);self.assertIn('_more_project_code:',body);self.assertNotIn('/temporary',body)
        with self.assertRaises(ValueError):timing.linked_text(b'_nfs_frame_control:\n0000 ret\n')
if __name__=='__main__':unittest.main()
