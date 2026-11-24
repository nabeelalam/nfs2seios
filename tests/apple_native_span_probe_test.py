# SPDX-License-Identifier: MIT
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
import unittest
from unittest.mock import patch
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('native_span_probe',ROOT/'scripts/apple_native_span_probe.py')
probe=importlib.util.module_from_spec(spec);spec.loader.exec_module(probe)
class NativeSpanProbeChecks(unittest.TestCase):
    def test_sources_are_frozen_and_production_matches_one_complete_variant(self):
        manifest=json.loads((probe.FIXTURE/'source-provenance.json').read_text())['sources']
        self.assertEqual(len(manifest),5)
        for e in manifest:
            for variant in ['control','candidate']:
                content=(probe.FIXTURE/variant/e['file']).read_bytes()
                self.assertEqual(hashlib.sha256(content).hexdigest(),e[variant+'_sha256'])
            self.assertNotEqual(e['control_sha256'],e['candidate_sha256'])
        matching=[variant for variant in ['control','candidate']if all((probe.FIXTURE/variant/e['file']).read_bytes().replace(b'\r\n',b'\n')==(ROOT/'src'/e['file']).read_bytes().replace(b'\r\n',b'\n')for e in manifest)]
        self.assertEqual(len(matching),1)
    def test_public_manual_route_is_sequential_and_has_no_payload_upload(self):
        workflow=(ROOT/'.github/workflows/lock-benchmark.yml').read_text()
        review,native=workflow.split('  review-native-span:\n',1)[1].split('  native-span:\n',1)
        for job in [review,native]:
            for required in ["github.event_name == 'workflow_dispatch'","github.repository == 'nabeelalam/nfs2seios'","github.event.repository.visibility == 'public'","github.ref == 'refs/heads/codex/hd-ui'","inputs.experiment == 'native-span'","ref: ${{ github.sha }}","persist-credentials: false"]:self.assertIn(required,job)
            for forbidden in ['upload-artifact','download-artifact','cache@','secrets.','gh release','cmake']:self.assertNotIn(forbidden,job)
        self.assertIn('scripts/publication_gate.py --mode history',review)
        self.assertIn('--local-safety-check',review)
        self.assertIn('needs: review-native-span',native)
        self.assertIn('runs-on: macos-15\n',native)
        self.assertIn('timeout-minutes: 8',native)
        self.assertIn('Xcode_16.4.app',native)
    def test_wrong_native_hosts_stop_before_commands(self):
        for system,arch in [('linux','aarch64'),('darwin','x86_64')]:
            with patch.object(probe.sys,'platform',system),patch.object(probe.platform,'machine',return_value=arch),patch.object(probe,'checked')as command:
                with self.assertRaises(ValueError):probe.execute()
                command.assert_not_called()
    def test_failure_output_is_not_echoed(self):
        with patch.object(probe.time,'monotonic',return_value=1),patch.object(probe.subprocess,'run',return_value=subprocess.CompletedProcess([],1,b'private output',b'private output')):
            with self.assertRaises(ValueError)as error:probe.checked(['compiler'],2)
            self.assertNotIn('private output',str(error.exception))
    def test_expired_deadline_does_not_start_command(self):
        with patch.object(probe.time,'monotonic',return_value=2),patch.object(probe.subprocess,'run')as command:
            with self.assertRaises(ValueError):probe.checked(['compiler'],1)
            command.assert_not_called()
if __name__=='__main__':unittest.main()
