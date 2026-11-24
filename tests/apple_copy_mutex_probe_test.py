# SPDX-License-Identifier: MIT
import importlib.util
from pathlib import Path
import subprocess
import unittest
from unittest.mock import patch
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('copy_mutex_probe',ROOT/'scripts/apple_copy_mutex_probe.py');probe=importlib.util.module_from_spec(spec);spec.loader.exec_module(probe)
def fixture():
    lines=['CHECK backend=std_mutex shared_updates=80000 concurrent_owners=0 exception_cleanup=pass']
    for workload,ops,workers in [('uncontended',8,1),('two_equal_workers',4,2),('eight_to_one_workers',9,2)]:
        lines.append(f'BENCH backend=std_mutex workload={workload} round=0 seconds={ops/1000000:.9f} operations={ops} ns_per_operation=1000.000 cpu_ns={ops*3} cpu_ns_per_operation=3.000')
        for worker in range(workers):lines.append(f'WAIT backend=std_mutex workload={workload} round=0 worker={worker} max_sampled_wait_ns=1')
    lines.append('PASS: packed payload counts and exception cleanup; no game assets used')
    return ('\n'.join(lines)+'\n').encode()
class CopyMutexProbeChecks(unittest.TestCase):
    def test_complete_accounting_is_accepted(self):
        result=probe.parse(fixture(),['std_mutex'],1,8);self.assertEqual(len(result['rows']),3);self.assertEqual(len(result['waits']),5)
    def test_missing_duplicate_wrong_cpu_and_worker_records_rejected(self):
        data=fixture();lines=data.splitlines(keepends=True)
        for bad in [b''.join(lines[1:]),data+lines[1],data.replace(b'cpu_ns=24 ',b'cpu_ns=25 '),data.replace(b'worker=1',b'worker=2'),data.replace(b'concurrent_owners=0',b'concurrent_owners=1'),b''.join(lines[:-1])]:
            with self.subTest(bad=bad[:20]),self.assertRaises(ValueError):probe.parse(bad,['std_mutex'],1,8)
    def test_native_wrong_host_stops_before_commands(self):
        for system,arch in [('linux','aarch64'),('darwin','x86_64')]:
            with patch.object(probe.sys,'platform',system),patch.object(probe.platform,'machine',return_value=arch),patch.object(probe,'checked')as command:
                with self.assertRaises(ValueError):probe.execute()
                command.assert_not_called()
    def test_command_failures_do_not_echo_output(self):
        with patch.object(probe.time,'monotonic',return_value=1),patch.object(probe.subprocess,'run',return_value=subprocess.CompletedProcess([],1,b'private output',b'private output')):
            with self.assertRaises(ValueError)as error:probe.checked(['compiler'],2)
            self.assertNotIn('private output',str(error.exception))
    def test_expired_deadline_stops_before_command(self):
        with patch.object(probe.time,'monotonic',return_value=2),patch.object(probe.subprocess,'run')as command:
            with self.assertRaises(ValueError):probe.checked(['compiler'],1)
            command.assert_not_called()
    def test_proxy_never_authorizes_production_promotion(self):
        self.assertFalse(probe.GATES['productionPromotionAllowed']);self.assertTrue(probe.GATES['proxyOnly'])
        source=probe.SOURCE.read_text();self.assertIn('check_shared_exclusion',source);self.assertIn('inside.fetch_add(1)',source);self.assertNotIn('lock_private.h',source);self.assertNotIn('ADAPTIVE_SPIN',source)
    def test_manual_native_route_is_sequential_pinned_and_payload_free(self):
        workflow=(ROOT/'.github/workflows/lock-benchmark.yml').read_text()
        review,native=workflow.split('  review-copy-mutex:\n',1)[1].split('  copy-mutex:\n',1)
        for job in [review,native]:
            for required in ["github.event_name == 'workflow_dispatch'","github.repository == 'nabeelalam/nfs2seios'","github.event.repository.visibility == 'public'","github.ref == 'refs/heads/codex/hd-ui'","inputs.experiment == 'copy-mutex'","ref: ${{ github.sha }}","persist-credentials: false"]:self.assertIn(required,job)
            for forbidden in ['upload-artifact','download-artifact','cache@','secrets.','gh release','cmake']:self.assertNotIn(forbidden,job)
        self.assertIn('scripts/publication_gate.py --mode history',review)
        self.assertIn('--local-safety-check',review)
        self.assertIn('needs: review-copy-mutex',native)
        self.assertIn('runs-on: macos-15\n',native)
        self.assertIn('timeout-minutes: 8',native)
        self.assertIn('Xcode_16.4.app',native)
if __name__=='__main__':unittest.main()
