# SPDX-License-Identifier: MIT
"""Keep the lightweight native helper route separate from app builds/publication."""
from pathlib import Path
import unittest
import sys,importlib.util,json,struct,io
from unittest.mock import patch
from types import SimpleNamespace
ROOT=Path(__file__).resolve().parents[1]
class WorkflowChecks(unittest.TestCase):
 def test_manual_registered_route_and_read_only_permissions(self):
  text=(ROOT/'.github/workflows/lock-benchmark.yml').read_text()
  self.assertIn('  workflow_dispatch:',text)
  for forbidden in ('  push:', '  pull_request:', 'workflow_call:', 'secrets.', 'contents: write', 'actions: write'):
   self.assertNotIn(forbidden,text)
  self.assertIn('permissions:\n  contents: read',text)
  self.assertIn('group: reviewed-native-build',text)
  self.assertIn('cancel-in-progress: false',text)
 def test_both_tls_jobs_fail_closed_and_run_sequentially(self):
  text=(ROOT/'.github/workflows/lock-benchmark.yml').read_text()
  review=text.split('  review-tls:\n',1)[1].split('  native-tls:\n',1)[0]
  native=text.split('  native-tls:\n',1)[1]
  for job in (review,native):
   for required in ("github.event_name == 'workflow_dispatch'", "github.repository == 'nabeelalam/nfs2seios'", "github.event.repository.visibility == 'public'", "github.ref == 'refs/heads/codex/hd-ui'", "inputs.experiment == 'thread-local'",'ref: ${{ github.sha }}','persist-credentials: false'):
    self.assertIn(required,job)
  self.assertIn('needs: review-tls',native);self.assertIn('if: success()',native)
  self.assertIn('runs-on: ubuntu-24.04',review);self.assertIn('timeout-minutes: 5',review)
  self.assertIn('scripts/publication_gate.py --mode history',review)
  self.assertIn('scripts/publication_audit.py --mode worktree',review)
  self.assertIn('--local-safety-check',review)
  self.assertIn('runs-on: macos-15\n',native);self.assertIn('timeout-minutes: 8',native)
  self.assertIn('test "$(uname -m)" = arm64',native)
  self.assertIn('Xcode_16.4.app',native)
 def test_no_game_assets_build_upload_or_cache_in_tls_jobs(self):
  text=(ROOT/'.github/workflows/lock-benchmark.yml').read_text().split('  review-tls:\n',1)[1]
  for forbidden in ('cmake','prepare_ci_sources','upload-artifact','download-artifact','cache@','gh release','publish_ipa','publish_release','larger','xlarge'):
   self.assertNotIn(forbidden,text)
  self.assertEqual(text.count('scripts/apple_stop_slot_probe.py'),1)
  self.assertEqual(text.count('scripts/apple_stop_slot_runtime_probe.py\n'),1)
 def test_original_lock_experiment_remains_main_only(self):
  job=(ROOT/'.github/workflows/lock-benchmark.yml').read_text().split('  benchmark:\n',1)[1].split('  review-tls:\n',1)[0]
  self.assertIn("github.ref == 'refs/heads/main'",job)
  self.assertIn("inputs.experiment == 'copy-lock'",job)
  self.assertIn('tests/guest_copy_lock_benchmark.cpp',job)

sys.path.insert(0,str(ROOT/'scripts'))
try:
 spec=importlib.util.spec_from_file_location('runtime_probe',ROOT/'scripts/apple_stop_slot_runtime_probe.py')
 runtime=importlib.util.module_from_spec(spec);spec.loader.exec_module(runtime)
finally:sys.path.pop(0)
class RuntimeProbeChecks(unittest.TestCase):
 def test_native_mode_refuses_wrong_platform_without_compiling(self):
  with patch.object(runtime.sys,'platform','linux'),patch.object(runtime,'checked')as run:
   with self.assertRaises(ValueError):runtime.configuration(False)
   run.assert_not_called()
  with patch.object(runtime.sys,'platform','darwin'),patch.object(runtime.platform,'machine',return_value='x86_64'),patch.object(runtime,'checked')as run:
   with self.assertRaises(ValueError):runtime.configuration(False)
   run.assert_not_called()
 def test_native_header_guard_refuses_wrong_arch_fat_or_nonexecutable(self):
  class Blob:
   def __init__(self,cpu=0x0100000C,magic=0xFEEDFACF,mode=0o700):self.data=struct.pack('<IIII',magic,cpu,0,2)+bytes(16);self.mode=mode
   def is_symlink(self):return False
   def is_file(self):return True
   def stat(self):return SimpleNamespace(st_size=len(self.data),st_mode=self.mode)
   def open(self,_):return io.BytesIO(self.data)
  runtime.native_arm64_macho(Blob())
  for blob in (Blob(cpu=0x01000007),Blob(magic=0xcafebabe),Blob(mode=0o600)):
   with self.assertRaises(ValueError):runtime.native_arm64_macho(blob)
 def test_parser_keeps_verification_and_measurements_distinct(self):
  proof=dict(type='verification',separate_probe_tu=True,actual_thread_tls=True,checks=126,resolver_calls=583,memory_checksum=123)
  self.assertEqual(runtime.parse_rows(json.dumps(proof),False),[proof])
  for key in ('checks','actual_thread_tls','memory_checksum'):
   bad=dict(proof);bad[key]=False
   with self.assertRaises(ValueError):runtime.parse_rows(json.dumps(bad),False)
  rows=[dict(type='measurement',kernel=k,iterations=runtime.ITERATIONS[k],cpu_ns=100,wall_ns=120,checksum=123)for k in runtime.KERNELS]
  output='\n'.join(map(json.dumps,rows));self.assertEqual(len(runtime.parse_rows(output,True)),3)
  with self.assertRaises(ValueError):runtime.parse_rows(output,False)
  with self.assertRaises(ValueError):runtime.parse_rows(output.splitlines()[0],True)
 def test_gate_requires_complete_balanced_samples_and_rejects_regression(self):
  def data(unified_transform):
   return [dict(variant=v,kernel=k,cpu_ns=(100 if v=='old'else unified_transform if k=='transform9'else 70),wall_ns=100,iterations=1)for v in runtime.VARIANTS for k in runtime.KERNELS for _ in range(12)]
  _,decisions=runtime.summarize(data(100));self.assertTrue(decisions['unified']['component_gate_passed'])
  _,decisions=runtime.summarize(data(106));self.assertFalse(decisions['unified']['component_gate_passed'])
  with self.assertRaises(ValueError):runtime.summarize(data(100)[:-1])
if __name__=='__main__':unittest.main()
