# SPDX-License-Identifier: MIT
import importlib.util
import json
from pathlib import Path
import struct
import tempfile
import unittest
from unittest.mock import patch
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('frame_cache_probe',ROOT/'scripts/apple_frame_cache_probe.py');probe=importlib.util.module_from_spec(spec);spec.loader.exec_module(probe)
class FrameCacheProbeChecks(unittest.TestCase):
    def test_frozen_closure_drift_is_rejected(self):
        self.assertEqual(len(probe.sources()),18)
        with patch.object(probe,'sha',return_value='0'*64):
            with self.assertRaises(ValueError):probe.sources()
    def test_manifest_cannot_admit_other_inputs(self):
        with patch.object(probe,'FIXTURE')as fixture:
            (fixture/'source-provenance.json').read_text.return_value=json.dumps({'sources':[{'file':'game/original.bin','sha256':'0'*64}]})
            with self.assertRaises(ValueError):probe.sources()
    def test_macho_rejects_wrong_architecture_type_and_bounds(self):
        raw=struct.pack('<IIIIIIII',0xfeedfacf,0x0100000c,0,2,0,0,0,0)
        with tempfile.TemporaryDirectory()as tmp:
            p=Path(tmp)/'helper';p.write_bytes(raw);self.assertEqual(probe.macho(p,2)['architecture'],'arm64')
            for data in [raw[:-1],raw[:4]+struct.pack('<I',7)+raw[8:],raw[:12]+struct.pack('<I',1)+raw[16:]]:
                p.write_bytes(data)
                with self.assertRaises(ValueError):probe.macho(p,2)
    def test_ios_target_and_sdk_are_checked(self):
        raw=struct.pack('<IIIIIIII',0xfeedfacf,0x0100000c,0,2,1,24,0,0)+struct.pack('<IIIIII',0x32,24,2,16<<16,(18<<16)|(5<<8),0)
        with tempfile.TemporaryDirectory()as tmp:
            p=Path(tmp)/'helper';p.write_bytes(raw)
            self.assertEqual(probe.macho(p,2,2,16<<16,(18<<16)|(5<<8))['platform'],2)
            for data in [raw[:-1],raw[:40]+struct.pack('<I',1)+raw[44:],raw[:48]+struct.pack('<I',0)+raw[52:]]:
                p.write_bytes(data)
                with self.assertRaises(ValueError):probe.macho(p,2,2,16<<16,(18<<16)|(5<<8))
    def test_exact_oracle_coverage_and_types_required(self):
        edge=dict(passed=True,checks=90,probeMatchesCanonicalValidity=True,probePreservesErrnoAndFpFlags=True,fullWidthAndPartialWindowsChecked=True,revisionInvalidationAndSmallerReuseChecked=True,unboundAndRebindingChecked=True,noRetiredRevisionPointerDereference=True)
        integer=dict(checks=12109,dataBytes=69037,bssBytes=680644,concurrentOwnedChunkReads=80000,concurrentLiveSpanReads=80000,concurrentStaticSpanReads=160000,oracleDigest=8306997119277202445,bindingTransitionsChecked=True,engineObjectBytes=8,heapObjectBytes=49256)
        native=dict(passed=True,nativeSpanDifferentialCases=15277,concurrentNativeCopyCycles=40000,bindingTransitionsChecked=True,exactErrorsPreserved=True,engineObjectBytes=8)
        rows=dict(edges=edge,integer=integer,native=native);self.assertEqual(probe.verify_results(rows),rows)
        apple=json.loads(json.dumps(rows));apple['integer']['heapObjectBytes']=49280;self.assertEqual(probe.verify_results(apple,49280),apple)
        with self.assertRaises(ValueError):probe.verify_results(apple)
        for category,key,value in [('edges','checks',89),('edges','passed',1),('integer','oracleDigest',0),('native','concurrentNativeCopyCycles',39999),('native','exactErrorsPreserved',False)]:
            copied=json.loads(json.dumps(rows));copied[category][key]=value
            with self.subTest(category=category,key=key),self.assertRaises(ValueError):probe.verify_results(copied)
if __name__=='__main__':unittest.main()
