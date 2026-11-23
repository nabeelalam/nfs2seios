# SPDX-License-Identifier: MIT
"""Publication/resource isolation policy for the manual cold-stop experiment."""
from pathlib import Path
import re
import unittest
ROOT=Path(__file__).resolve().parents[1]
class ColdWorkflowTests(unittest.TestCase):
 def setUp(self):self.text=(ROOT/'.github/workflows/lock-benchmark.yml').read_text()
 def block(self,name):
  tail=self.text.split('  '+name+':\n',1)[1]
  return re.split(r'^  [a-z][a-z-]*:\n',tail,maxsplit=1,flags=re.M)[0]
 def test_manual_only_read_only_shared_concurrency(self):
  self.assertIn('          - cold-stop\n',self.text)
  self.assertIn('  workflow_dispatch:',self.text)
  self.assertIn('permissions:\n  contents: read',self.text)
  self.assertIn('group: reviewed-native-build\n  cancel-in-progress: false',self.text)
  for forbidden in ('  push:','  pull_request:','workflow_call:','contents: write','actions: write','secrets.'):
   self.assertNotIn(forbidden,self.text)
 def test_both_jobs_fail_closed_to_exact_public_source(self):
  for name in ('review-cold','native-cold'):
   job=self.block(name)
   for guard in ("github.event_name == 'workflow_dispatch'","github.repository == 'nabeelalam/nfs2seios'","github.event.repository.visibility == 'public'","github.ref == 'refs/heads/codex/hd-ui'","inputs.experiment == 'cold-stop'",'ref: ${{ github.sha }}','persist-credentials: false'):
    self.assertIn(guard,job)
   self.assertIn('actions/checkout@11d5960a326750d5838078e36cf38b85af677262',job)
 def test_privacy_and_safety_complete_before_arm_job(self):
  review=self.block('review-cold');native=self.block('native-cold')
  self.assertIn('fetch-depth: 0',review)
  self.assertLess(review.index('scripts/publication_gate.py --mode history'),review.index('scripts/apple_cold_stop_probe.py --local-safety-check'))
  self.assertIn('scripts/publication_audit.py --mode worktree',review)
  self.assertIn('scripts/setup_secret_scanner.py',review)
  for name in ('apple_cold_workflow_test.py','apple_cold_stop_probe_test.py','apple_stop_slot_probe_test.py','apple_cpu_workflow_test.py'):
   self.assertIn('python3 tests/'+name,review)
  self.assertIn('needs: review-cold',native);self.assertIn('if: success()',native)
 def test_standard_machine_compiler_and_time_bounds(self):
  review=self.block('review-cold');native=self.block('native-cold')
  self.assertIn('runs-on: ubuntu-24.04\n',review);self.assertIn('timeout-minutes: 5',review)
  self.assertIn('runs-on: macos-15\n',native);self.assertIn('timeout-minutes: 8',native)
  self.assertIn('/Applications/Xcode_16.4.app/Contents/Developer',native)
  self.assertIn('test "$(uname -m)" = arm64',native)
  self.assertEqual(native.count('run: python3 scripts/apple_cold_stop_probe.py\n'),1)
 def test_no_game_data_app_build_upload_cache_or_private_compute(self):
  for name in ('review-cold','native-cold'):
   job=self.block(name)
   for forbidden in ('cmake','prepare_ci_sources','upload-artifact','download-artifact','cache@','gh release','publish_ipa','publish_release','larger','xlarge','macos-15-intel','secrets.'):
    self.assertNotIn(forbidden,job)
 def test_existing_experiments_remain_separate(self):
  for name,choice in (('benchmark','copy-lock'),('review-tls','thread-local'),('native-tls','thread-local')):
   self.assertIn("inputs.experiment == '"+choice+"'",self.block(name))
   self.assertNotIn("inputs.experiment == 'cold-stop'",self.block(name))
if __name__=='__main__':unittest.main()
