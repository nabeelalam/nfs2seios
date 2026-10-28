"""Reject unsafe delivery inputs; fixtures do not prove an installable game."""
import hashlib
import json
from pathlib import Path
import plistlib
import struct
import subprocess
import sys
import tempfile
import unittest
import zipfile
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "scripts"))
from prepare_release_download import inspect, PREFIX, SUPPORT, LICENSES, CLASSIC_LICENSES

class DownloadTest(unittest.TestCase):
    def fixture(self, folder, extra=None, binary=None, classic=False):
        source = "a" * 40
        data = {name: b"support fixture" for name in (SUPPORT | (CLASSIC_LICENSES if classic else set()))}
        data["Info.plist"] = plistlib.dumps({"CFBundleIdentifier": "io.github.nabeelalam.nfsiise.game", "CFBundleExecutable": "NFSIISEGame", "UIDeviceFamily": [2]})
        data["PkgInfo"] = b"APPL????"
        data["NFSIISEGame"] = binary if binary is not None else struct.pack("<8I6I", 0xfeedfacf, 0x100000c, 0, 2, 1, 24, 0, 0, 0x32, 24, 2, 0, 0, 0)
        if extra:
            data.update(extra)
        ipa, manifest = Path(folder)/"game.ipa", Path(folder)/"game.json"
        with zipfile.ZipFile(ipa, "w") as archive:
            for name, value in data.items():
                archive.writestr(PREFIX + name, value)
        metadata = {"executable": "NFSIISEGame", "bundle_id": "io.github.nabeelalam.nfsiise.game", "architecture": "arm64", "platform": "iOS device", "support_sha256": {name: hashlib.sha256(data[name]).hexdigest() for name in (SUPPORT | (CLASSIC_LICENSES if classic else set()))}, "source_commit": source, "ipa_sha256": hashlib.sha256(ipa.read_bytes()).hexdigest(), "configuration": "Release", "personal_signing_required": True, "installation_verified": False, "gameplay_verified": False}
        if classic: metadata.update(classic_ui=True,classic_asset_manifest_sha256="a"*64)
        manifest.write_text(json.dumps(metadata), encoding="utf-8")
        return ipa, manifest, source

    def test_classic_base_cannot_become_public_release(self):
        with tempfile.TemporaryDirectory() as folder:
            ipa,manifest,source=self.fixture(folder,classic=True)
            metadata=json.loads(manifest.read_text())
            metadata.update(classic_ui=True,classic_asset_manifest_sha256="a"*64)
            manifest.write_text(json.dumps(metadata))
            with self.assertRaisesRegex(ValueError,"personal artwork"):
                inspect(ipa,manifest,source)
            metadata["classic_ui"]=False
            manifest.write_text(json.dumps(metadata))
            with self.assertRaisesRegex(ValueError,"classic"):
                inspect(ipa,manifest,source)

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

    def test_public_zip_excludes_all_language_and_configuration(self):
        with tempfile.TemporaryDirectory() as folder:
            ipa, manifest, source = self.fixture(folder)
            output = Path(folder) / "download.zip"
            subprocess.run([sys.executable, str(Path(__file__).resolve().parents[1] / "scripts/prepare_release_download.py"), str(ipa), str(manifest), str(output), "--source", source], check=True, capture_output=True)
            with zipfile.ZipFile(output) as download:
                public_ipa = Path(folder) / "public.ipa"
                public_manifest = Path(folder) / "public.json"
                public_ipa.write_bytes(download.read("NFSIISEGame-development-unsigned.ipa"))
                public_manifest.write_bytes(download.read("NFSIISEGame-development-unsigned.json"))
            inspect(public_ipa, public_manifest, source, assets_separate=True)
            with zipfile.ZipFile(public_ipa) as public:
                self.assertEqual(set(public.namelist()),{PREFIX+name for name in LICENSES|{"Info.plist","PkgInfo","NFSIISEGame"}})
                self.assertNotIn(PREFIX + "install.win", public.namelist())
                self.assertNotIn(PREFIX + "text.eng", public.namelist())
            with self.assertRaisesRegex(ValueError, "support"):
                inspect(ipa, manifest, source, assets_separate=True)

if __name__ == "__main__":
    unittest.main()
