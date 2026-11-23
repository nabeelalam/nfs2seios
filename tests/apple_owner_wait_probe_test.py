# SPDX-License-Identifier: MIT
import importlib.util
from pathlib import Path
from unittest.mock import patch
import subprocess
import unittest

ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location("owner_wait",ROOT/"scripts/apple_owner_wait_probe.py")
probe=importlib.util.module_from_spec(spec);spec.loader.exec_module(probe)

class OwnerWaitChecks(unittest.TestCase):
    def test_native_mode_rejects_non_apple_host_before_compiling(self):
        for system,arch in (("linux","x86_64"),("darwin","unknown")):
            with patch.object(probe.sys,"platform",system), patch.object(probe.platform,"machine",return_value=arch), patch.object(probe,"checked") as checked:
                with self.assertRaises(ValueError):probe.execute()
                checked.assert_not_called()

    def test_command_failures_never_echo_private_diagnostics(self):
        with patch.object(probe.time,"monotonic",return_value=1), patch.object(probe.subprocess,"run",return_value=subprocess.CompletedProcess([],1,b"private-path",b"private-secret")):
            with self.assertRaises(ValueError) as result:probe.checked(["compiler"],2)
            self.assertNotIn("private",str(result.exception))
        with patch.object(probe.time,"monotonic",return_value=2), patch.object(probe.subprocess,"run") as run:
            with self.assertRaises(ValueError):probe.checked(["compiler"],1)
            run.assert_not_called()

    def test_manual_sequential_source_only_route(self):
        text=(ROOT/".github/workflows/lock-benchmark.yml").read_text()
        review,native=text.split("  review-owner-wait:\n",1)[1].split("  native-owner-wait:\n",1)
        for job in (review,native):
            for required in ("github.event_name == 'workflow_dispatch'","github.repository == 'nabeelalam/nfs2seios'","github.event.repository.visibility == 'public'","github.ref == 'refs/heads/codex/hd-ui'","inputs.experiment == 'owner-wait'","ref: ${{ github.sha }}","persist-credentials: false"):
                self.assertIn(required,job)
            for forbidden in ("upload-artifact","download-artifact","cache@","secrets.","gh release","publish_ipa","cmake"):
                self.assertNotIn(forbidden,job)
        self.assertIn("scripts/publication_gate.py --mode history",review)
        self.assertIn("--local-safety-check",review)
        self.assertIn("needs: review-owner-wait",native)
        self.assertIn("runs-on: macos-15-intel\n",native)
        self.assertIn("timeout-minutes: 6",native)
        self.assertIn("Xcode_16.4.app",native)

if __name__=="__main__":unittest.main()
