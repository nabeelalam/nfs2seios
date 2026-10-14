"""Reject unsafe delivery inputs; fixtures do not prove an installable game."""
import hashlib
import json
from pathlib import Path
import plistlib
import struct
import sys
import tempfile
import unittest
import zipfile
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "scripts"))
from prepare_release_download import inspect, PREFIX, SUPPORT

class DownloadTest(unittest.TestCase):
    def fixture(self, folder, extra=None, binary=None):
        source = "a" * 40
        data = {name: b"support fixture" for name in SUPPORT}
        data["Info.plist"] = plistlib.dumps({"CFBundleIdentifier": "io.github.nabeelalam.nfsiise.game", "CFBundleExecutable": "NFSIISEGame", "UIDeviceFamily": [2]})
        data["PkgInfo"] = b"APPL????"
        data["NFSIISEGame"] = binary if binary is not None else struct.pack("<8I6I", 0xfeedfacf, 0x100000c, 0, 2, 1, 24, 0, 0, 0x32, 24, 2, 0, 0, 0)
        if extra:
            data.update(extra)
        ipa, manifest = Path(folder)/"game.ipa", Path(folder)/"game.json"
        with zipfile.ZipFile(ipa, "w") as archive:
            for name, value in data.items():
                archive.writestr(PREFIX + name, value)
        metadata = {"executable": "NFSIISEGame", "bundle_id": "io.github.nabeelalam.nfsiise.game", "architecture": "arm64", "platform": "iOS device", "support_sha256": {name: hashlib.sha256(data[name]).hexdigest() for name in SUPPORT}, "source_commit": source, "ipa_sha256": hashlib.sha256(ipa.read_bytes()).hexdigest(), "configuration": "Release", "personal_signing_required": True, "installation_verified": False, "gameplay_verified": False}
        manifest.write_text(json.dumps(metadata), encoding="utf-8")
        return ipa, manifest, source

    def test_accept_and_source_or_digest_rejection(self):
        with tempfile.TemporaryDirectory() as folder:
            ipa, manifest, source = self.fixture(folder)
            self.assertEqual(inspect(ipa, manifest, source)["source_commit"], source)
            with self.assertRaisesRegex(ValueError, "identity"):
                inspect(ipa, manifest, "b" * 40)
            ipa.write_bytes(ipa.read_bytes() + b"changed")
            with self.assertRaisesRegex(ValueError, "digest"):
                inspect(ipa, manifest, source)

    def test_assets_and_private_content_rejected(self):
        with tempfile.TemporaryDirectory() as folder:
            with self.assertRaisesRegex(ValueError, "contents"):
                inspect(*self.fixture(folder, {"GAMEDATA/data": b"forbidden"}))
            # Construct test paths at runtime; the source itself stays publishable.
            path = b"C:" + bytes([92]) + b"Users" + bytes([92]) + b"fixture"
            with self.assertRaisesRegex(ValueError, "privacy"):
                inspect(*self.fixture(folder, {"install.win": path}))

    def test_simulator_and_false_acceptance_rejected(self):
        with tempfile.TemporaryDirectory() as folder:
            binary = struct.pack("<8I6I", 0xfeedfacf, 0x100000c, 0, 2, 1, 24, 0, 0, 0x32, 24, 7, 0, 0, 0)
            with self.assertRaisesRegex(ValueError, "platform"):
                inspect(*self.fixture(folder, binary=binary))
            ipa, manifest, source = self.fixture(folder)
            metadata = json.loads(manifest.read_text())
            metadata["gameplay_verified"] = True
            manifest.write_text(json.dumps(metadata))
            with self.assertRaisesRegex(ValueError, "claims"):
                inspect(ipa, manifest, source)

if __name__ == "__main__":
    unittest.main()
