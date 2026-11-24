# SPDX-License-Identifier: MIT
import importlib.util
import json
from pathlib import Path
import struct
import subprocess
import tempfile
import unittest
from unittest.mock import patch
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('current_accessor_probe',ROOT/'scripts/apple_current_accessor_probe.py');probe=importlib.util.module_from_spec(spec);spec.loader.exec_module(probe)
class CurrentAccessorProbeChecks(unittest.TestCase):
    def test_frozen_sources_exact_and_drift_rejected(self):
        self.assertEqual(len(probe.sources()),16)
        with patch.object(probe,'sha',return_value='0'*64):
            with self.assertRaises(ValueError):probe.sources()
    def test_source_manifest_cannot_expand_to_other_inputs(self):
        with patch.object(probe,'MANIFEST')as manifest:
            manifest.read_text.return_value=json.dumps({'sources':[{'file':'game/original.bin','sha256':'0'*64}]})
            with self.assertRaises(ValueError):probe.sources()
    def test_verification_requires_exact_layout_and_correctness(self):
        row=dict(type='verification',transactionBytes=24,cases=300,checksum=9907831787116055075,actualEngineMemory=True,overlapAndStopPrecedenceVerified=True)
        self.assertEqual(probe.verification(json.dumps(row),'candidate'),row)
        for key,value in [('transactionBytes',16),('cases',299),('checksum',0),('actualEngineMemory',False)]:
            with self.subTest(key=key),self.assertRaises(ValueError):probe.verification(json.dumps(dict(row,**{key:value})),'candidate')
    def test_timed_workloads_require_threaded_process_exact_arguments_and_types(self):
        row=dict(type='measurement',threadedProcess=True,mode=0,words=8,iterations=125000,stopMode=0,cpuNs=100,wallNs=110,checksum=0)
        self.assertEqual(probe.measurement(json.dumps(row),0,8,125000,0),row)
        for key,value in [('threadedProcess',False),('mode',False),('words',4),('iterations',1),('stopMode',1),('cpuNs',0),('checksum',-1)]:
            with self.subTest(key=key),self.assertRaises(ValueError):probe.measurement(json.dumps(dict(row,**{key:value})),0,8,125000,0)
    def test_macho_platform_and_command_bounds(self):
        data=struct.pack('<IIIIIIII',0xfeedfacf,0x0100000c,0,2,1,24,0,0)+struct.pack('<IIIIII',0x32,24,2,16<<16,(18<<16)|(5<<8),0)
        with tempfile.TemporaryDirectory()as folder:
            p=Path(folder)/'helper';p.write_bytes(data);self.assertEqual(probe.macho_platform(p,2,16<<16,(18<<16)|(5<<8))['platform'],2)
            for bad in [data[:-1],data[:32],data[:40]+struct.pack('<I',1)+data[44:]]:
                p.write_bytes(bad)
                with self.assertRaises(ValueError):probe.macho_platform(p,2,16<<16,(18<<16)|(5<<8))
    def test_linked_bodies_must_be_complete_and_include_true_stop_helper(self):
        text='\n'.join(name+':\n0000000100001000 ret'for name in probe.PROBES)+'\n__ZN_handle_requested_stopEv:\n0000000100001100 ret\n'
        self.assertEqual(len(probe.selected_bodies(text.encode(),'candidate')),4)
        with self.assertRaises(ValueError):probe.selected_bodies(text.split('__ZN_handle')[0].encode(),'candidate')
        with self.assertRaises(ValueError):probe.selected_bodies(text.replace(probe.PROBES[0],'_unknown').encode(),'candidate')
        with self.assertRaises(ValueError):probe.selected_bodies((probe.PROBES[0]+':\n'+'ret\n'*2049).encode(),'control')
    def test_wrong_native_hosts_do_not_start_commands(self):
        for system,arch in [('linux','aarch64'),('darwin','x86_64')]:
            with patch.object(probe.sys,'platform',system),patch.object(probe.platform,'machine',return_value=arch),patch.object(probe,'checked')as command:
                with self.assertRaises(ValueError):probe.execute()
                command.assert_not_called()
    def test_expired_deadline_stops_before_command(self):
        with patch.object(probe.time,'monotonic',return_value=2),patch.object(probe.subprocess,'run')as command:
            with self.assertRaises(ValueError):probe.checked(['compiler'],1)
            command.assert_not_called()
    def test_failures_do_not_echo_command_output(self):
        with patch.object(probe.time,'monotonic',return_value=1),patch.object(probe.subprocess,'run',return_value=subprocess.CompletedProcess([],1,b'private output',b'private output')):
            with self.assertRaises(ValueError)as error:probe.checked(['compiler'],2)
            self.assertNotIn('private output',str(error.exception))
    def test_manual_route_is_sequential_pinned_and_without_payload_upload(self):
        workflow=(ROOT/'.github/workflows/lock-benchmark.yml').read_text()
        review,native=workflow.split('  review-current-accessor:\n',1)[1].split('  current-accessor:\n',1)
        for job in [review,native]:
            for required in ["github.event_name == 'workflow_dispatch'","github.repository == 'nabeelalam/nfs2seios'","github.event.repository.visibility == 'public'","github.ref == 'refs/heads/codex/hd-ui'","inputs.experiment == 'current-accessor'","ref: ${{ github.sha }}","persist-credentials: false"]:self.assertIn(required,job)
            for forbidden in ['upload-artifact','download-artifact','cache@','secrets.','gh release','cmake']:self.assertNotIn(forbidden,job)
        self.assertIn('scripts/publication_gate.py --mode history',review)
        self.assertIn('--local-safety-check',review)
        self.assertIn('needs: review-current-accessor',native)
        self.assertIn('runs-on: macos-15\n',native)
        self.assertIn('timeout-minutes: 8',native)
        self.assertIn('Xcode_16.4.app',native)
if __name__=='__main__':unittest.main()
