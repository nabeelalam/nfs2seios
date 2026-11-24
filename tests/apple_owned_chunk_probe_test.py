# SPDX-License-Identifier: MIT
import hashlib
import importlib.util
from pathlib import Path
from unittest.mock import patch
import subprocess
import unittest

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location("owned_chunk", ROOT / "scripts/apple_owned_chunk_probe.py")
probe = importlib.util.module_from_spec(spec)
spec.loader.exec_module(probe)

class OwnedChunkProbeChecks(unittest.TestCase):
    def test_frozen_control_is_original_and_production_matches_validated_candidate(self):
        expected = {"EngineMemory.cpp": "f5fb4d20071484eb7c6f467196cc17819899ff37d4f4ef47f183ab817138275a",
                    "HeapArena.h": "6a9de84b902ac227bd5db8bb97f45a3bf0b566e775f9030f6ef81cc26cf38902"}
        for name, digest in expected.items():
            original = (probe.FIXTURE / "control" / name).read_bytes()
            self.assertEqual(hashlib.sha256(original).hexdigest(), digest)
            self.assertNotEqual(original, (ROOT / "src" / name).read_bytes())
            validated={(probe.FIXTURE / name).read_bytes()}
            if name=="EngineMemory.cpp": validated.add((ROOT / "tests/fixtures/native_span/candidate" / name).read_bytes())
            self.assertIn((ROOT / "src" / name).read_bytes().replace(b"\r\n",b"\n"),{v.replace(b"\r\n",b"\n")for v in validated})

    def test_native_mode_refuses_linux_and_intel_mac_before_commands(self):
        for system, arch in (("linux", "aarch64"), ("darwin", "x86_64")):
            with patch.object(probe.sys, "platform", system), patch.object(probe.platform, "machine", return_value=arch), patch.object(probe, "checked") as command:
                with self.assertRaises(ValueError): probe.execute()
                command.assert_not_called()

    def test_failed_or_expired_command_does_not_echo_diagnostics(self):
        with patch.object(probe.time, "monotonic", return_value=1), patch.object(probe.subprocess, "run", return_value=subprocess.CompletedProcess([], 1, b"sensitive", b"sensitive")):
            with self.assertRaises(ValueError) as result: probe.checked(["compiler"], 2)
            self.assertNotIn("sensitive", str(result.exception))
        with patch.object(probe.time, "monotonic", return_value=2), patch.object(probe.subprocess, "run") as command:
            with self.assertRaises(ValueError): probe.checked(["compiler"], 1)
            command.assert_not_called()

    def test_public_manual_route_has_sequential_review_and_no_payload_upload(self):
        workflow = (ROOT / ".github/workflows/lock-benchmark.yml").read_text()
        review, native = workflow.split("  review-owned-chunk:\n", 1)[1].split("  native-owned-chunk:\n", 1)
        for job in (review, native):
            for required in ("github.event_name == 'workflow_dispatch'", "github.repository == 'nabeelalam/nfs2seios'", "github.event.repository.visibility == 'public'", "github.ref == 'refs/heads/codex/hd-ui'", "inputs.experiment == 'owned-chunk'", "ref: ${{ github.sha }}", "persist-credentials: false"):
                self.assertIn(required, job)
            for forbidden in ("upload-artifact", "download-artifact", "cache@", "secrets.", "gh release", "cmake"):
                self.assertNotIn(forbidden, job)
        self.assertIn("scripts/publication_gate.py --mode history", review)
        self.assertIn("--local-safety-check", review)
        self.assertIn("needs: review-owned-chunk", native)
        self.assertIn("runs-on: macos-15\n", native)
        self.assertIn("timeout-minutes: 8", native)
        self.assertIn("Xcode_16.4.app", native)

if __name__ == "__main__": unittest.main()
