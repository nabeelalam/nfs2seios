"""Resource-only packaging checks, not an Apple compile or IPA installation test."""
import shutil
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "scripts"))
from bundle_resources import ROOT, HUD_NOTICE_SOURCES, validate_game_resources

class ResourcesTest(unittest.TestCase):
    def test_required_resources_and_rejections(self):
        with tempfile.TemporaryDirectory(prefix="nfs-resource-check-") as folder:
            app = Path(folder)
            support = ROOT / "candidates/link4/Need For Speed II SE"
            for name in ("install.win", "text.eng", "text.ger", "text.fre", "text.ita", "text.spa", "text.swe"):
                shutil.copyfile(support / name, app / name)
            shutil.copyfile(ROOT / "candidates/link4/LICENSE", app / "GameSupportLicense.txt")
            shutil.copyfile(ROOT / ".build/vendor/SDL/LICENSE.txt", app / "SDLLicense.txt")
            for name,source in HUD_NOTICE_SOURCES.items():shutil.copyfile(source,app/name)
            self.assertEqual(len(validate_game_resources(app)), 13)
            (app / "Oxanium-OFL.txt").unlink()
            with self.assertRaisesRegex(RuntimeError,"missing"):
                validate_game_resources(app)
            shutil.copyfile(HUD_NOTICE_SOURCES["Oxanium-OFL.txt"],app/"Oxanium-OFL.txt")
            (app / "text.eng").write_bytes(b"incorrect language resource")
            with self.assertRaisesRegex(RuntimeError, "differs"):
                validate_game_resources(app)
            shutil.copyfile(support / "text.eng", app / "text.eng")
            (app / "install.win").unlink()
            with self.assertRaisesRegex(RuntimeError, "missing"):
                validate_game_resources(app)
            shutil.copyfile(support / "install.win", app / "install.win")
            (app / "GAMEDATA").mkdir()
            with self.assertRaisesRegex(RuntimeError, "supplied separately"):
                validate_game_resources(app)

if __name__ == "__main__":
    unittest.main()
