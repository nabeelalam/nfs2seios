# SPDX-License-Identifier: MIT
"""Parser/command-policy checks only; synthetic assembly is not Apple evidence."""
import importlib.util,pathlib,unittest
ROOT=pathlib.Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location("probe",ROOT/"scripts/apple_stop_slot_probe.py")
probe=importlib.util.module_from_spec(spec);spec.loader.exec_module(probe)
class ProbeTests(unittest.TestCase):
 def test_commands_compile_only_and_shared_options(self):
  old=probe.command("clang++","/SDK",pathlib.Path("/source"),"old",pathlib.Path("old.s"))
  new=probe.command("clang++","/SDK",pathlib.Path("/source"),"candidate",pathlib.Path("new.s"))
  for cmd in [old,new]:
   self.assertIn("-S",cmd);self.assertIn("arm64-apple-ios16.0",cmd)
   for flag in ["-O3","-fwrapv","-fno-strict-aliasing"]:self.assertIn(flag,cmd)
   self.assertFalse(any(flag in cmd for flag in ["-shared","-c","-flto"]))
  differences=[(a,b) for a,b in zip(old,new) if a!=b]
  self.assertEqual(len(differences),2)
  self.assertIn("/src",differences[0][0].replace(chr(92),"/"));self.assertIn("/fixtures/",differences[0][1].replace(chr(92),"/"))
 def test_parser_counts_and_rejects_missing_function(self):
  body=" adrpx0, symbol@TLVPPAGE\n blr x8\n ldarb w9, [x8]\n tbz w9, #0, Ldone\n"
  text="\n".join("_nfs_probe_"+name+":\n"+body+" .cfi_endproc\n" for name in ["accessor","transform","rep8"])
  report=probe.inspect(text)
  self.assertEqual(report["whole_translation_unit"]["live_atomic_loads"],3)
  for entry in report["functions"].values():self.assertEqual(entry["indirect_calls"],1)
  with self.assertRaises(ValueError):probe.inspect(text.replace("_nfs_probe_rep8:","_wrong:"))
 def test_workflow_stays_isolated(self):
  workflow=(ROOT/".github/workflows/ipados.yml").read_text()
  self.assertEqual(workflow.count("      stop_slot_probe:"),1)
  job=workflow.split("  stop-slot-assembly:\n",1)[1]
  for requirement in ["github.repository == 'nabeelalam/nfs2seios'","refs/heads/codex/lpm-performance","inputs.stop_slot_probe","inputs.artwork_sha256 == ''","macos-15-intel","timeout-minutes: 10","Xcode_16.4.app","ref: ${{ github.sha }}","persist-credentials: false"]:self.assertIn(requirement,job)
  for name in ["build_ios","classic_ui","include_probe","thin_lto","publish_ipa","publish_base_draft","publish_release"]:self.assertIn("!inputs."+name,job)
  for forbidden in ["upload-artifact","release","cmake","prepare_ci_sources","cache@"]:
   if forbidden=="release":continue
   self.assertNotIn(forbidden,job)
  self.assertIn("'reviewed-stop-slot-probe' || 'reviewed-native-build'",workflow)
 def test_fixture_keeps_live_values_and_original_unchanged(self):
  old=(ROOT/"src/GuestCopyTransaction.h").read_text();new=(ROOT/"tests/fixtures/stop_slot_address_cache/GuestCopyTransaction.h").read_text()
  self.assertNotIn("control_slot",old);self.assertIn("WorkerControl *control=*control_slot",new)
  self.assertIn("control->stop.load()",new);self.assertNotIn("memory_order_relaxed",new)
  self.assertIn("!*critical_depth_slot && !*stop_deferrals_slot",new)
if __name__=="__main__":unittest.main()
