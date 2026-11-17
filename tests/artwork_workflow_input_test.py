"""Optional owner fingerprint is validated as data before source preparation."""
from pathlib import Path
import os
import subprocess
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]


class ArtworkWorkflowInput(unittest.TestCase):
    def test_validator_accepts_only_optional_hex(self):
        for value in ("", "a" * 64, "ABCDEF01" * 8, "a" * 63, "a" * 65,
                      "g" * 64, " " + "a" * 64, "$(touch injected)", "a" * 64 + "\n"):
            result = subprocess.run([sys.executable, str(ROOT / "scripts/validate_artwork_sha256.py")],
                                    env={**os.environ, "NFS_HD_ARTWORK_SHA256": value,
                                         "NFS_CLASSIC_UI_REQUESTED": "false"},
                                    capture_output=True, text=True)
            expected = value in ("", "a" * 64, "ABCDEF01" * 8)
            self.assertEqual(result.returncode == 0, expected, repr(value))

    def test_classic_requires_reviewed_fingerprint(self):
        for value in ("", "a" * 63, "a" * 64):
            result = subprocess.run(
                [sys.executable, str(ROOT / "scripts/validate_artwork_sha256.py")],
                env={**os.environ, "NFS_HD_ARTWORK_SHA256": value,
                     "NFS_CLASSIC_UI_REQUESTED": "true"}, capture_output=True, text=True)
            self.assertEqual(result.returncode == 0, len(value) == 64)
            if not value:
                self.assertIn("Classic UI requires the reviewed artwork fingerprint", result.stderr)

    def test_workflow_propagation_and_early_validation(self):
        for name in ("ipados.yml", "apple-device.yml"):
            text = (ROOT / ".github/workflows" / name).read_text()
            self.assertIn("      artwork_sha256:\n", text)
            self.assertIn("      NFS_HD_ARTWORK_SHA256: ${{ inputs.artwork_sha256 }}", text)
            self.assertLess(text.index("python3 scripts/validate_artwork_sha256.py"),
                            text.index("python3 scripts/prepare_ci_sources.py"))
            for line in text.splitlines():
                if "cmake -S . -B " in line:
                    self.assertIn('-DNFS_HD_ARTWORK_SHA256="$NFS_HD_ARTWORK_SHA256"', line)
                self.assertNotIn("-j 3", line)
        parent = (ROOT / ".github/workflows/ipados.yml").read_text()
        self.assertEqual(parent.count("      artwork_sha256: ${{ inputs.artwork_sha256 }}"), 2)
        self.assertLess(parent.index("Validate artwork fingerprint"),
                        parent.index("Publication and independent secret gates"))
        child = (ROOT / ".github/workflows/apple-device.yml").read_text()
        self.assertIn("      NFS_BUILD_JOBS: '2'", child)


    def test_deferred_classic_ui_requires_complete_location_photos(self):
        flag = '-DNFS_CLASSIC_REQUIRE_LOCATION_PHOTO_ARTWORK="$NFS_CLASSIC_UI"'
        child = (ROOT / ".github/workflows/apple-device.yml").read_text()
        configure = [line for line in child.splitlines() if "cmake -S . -B " in line]
        device = [line for line in configure if "-B .build/ios " in line]
        self.assertEqual(len(device), 1)
        self.assertIn(flag, device[0])
        self.assertEqual(child.count(flag), 1)
        for line in configure:
            if line != device[0]:
                self.assertNotIn(flag, line)
        parent = (ROOT / ".github/workflows/ipados.yml").read_text()
        self.assertEqual(parent.count("uses: ./.github/workflows/apple-device.yml"), 2)
        self.assertNotIn(flag, parent)
        cmake = (ROOT / "cmake/BundledOwnedArtwork.cmake").read_text()
        self.assertIn("SOURCE src/hd/ArtworkActivation.cpp app/main.cpp APPEND PROPERTY COMPILE_DEFINITIONS NFS_REQUIRE_LOCATION_PHOTO_ARTWORK=1", cmake)


if __name__ == "__main__":
    unittest.main()
