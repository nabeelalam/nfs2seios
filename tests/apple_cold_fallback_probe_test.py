# SPDX-License-Identifier: MIT
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
import unittest
from unittest.mock import patch
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('cold_fallback_probe',ROOT/'scripts/apple_cold_fallback_probe.py')
probe=importlib.util.module_from_spec(spec);spec.loader.exec_module(probe)
class ColdFallbackProbeChecks(unittest.TestCase):
    def test_candidate_is_exactly_declared_outline_and_control_matches_production(self):
        probe.verify_cold_source()
        for name in ['EngineMemory.cpp','EngineMemory.h','GuestRuntimeC.h','GuestAccess.h','GuestCopyTransaction.h']:
            self.assertEqual((probe.FIXTURE/'candidate'/name).read_bytes().replace(b'\r\n',b'\n'),(ROOT/'src'/name).read_bytes().replace(b'\r\n',b'\n'))
        self.assertEqual(probe.variant_source('candidate'),probe.COLD)
        self.assertEqual(probe.variant_source('control'),probe.FIXTURE/'candidate/EngineMemory.cpp')
    def test_candidate_drift_is_rejected(self):
        with patch.object(probe,'COLD_DIGEST','0'*64):
            with self.assertRaises(ValueError):probe.verify_cold_source()
    def test_public_manual_route_is_sequential_and_has_no_payload_upload(self):
        workflow=(ROOT/'.github/workflows/lock-benchmark.yml').read_text()
        review,native=workflow.split('  review-cold-fallback:\n',1)[1].split('  cold-fallback:\n',1)
        for job in [review,native]:
            for required in ["github.event_name == 'workflow_dispatch'","github.repository == 'nabeelalam/nfs2seios'","github.event.repository.visibility == 'public'","github.ref == 'refs/heads/codex/hd-ui'","inputs.experiment == 'cold-fallback'","ref: ${{ github.sha }}","persist-credentials: false"]:self.assertIn(required,job)
            for forbidden in ['upload-artifact','download-artifact','cache@','secrets.','gh release','cmake']:self.assertNotIn(forbidden,job)
        self.assertIn('scripts/publication_gate.py --mode history',review)
        self.assertIn('--local-safety-check',review)
        self.assertIn('needs: review-cold-fallback',native)
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
